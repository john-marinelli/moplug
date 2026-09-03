#include "processor.h"
#include "bridge.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <vector>

namespace MoPlugVst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

Processor::Processor(const PluginDescriptor* descriptor, Steinberg::FUID controllerFuid)
    : descriptor_(descriptor)
{
    setControllerClass(controllerFuid);
}

Processor::~Processor()
{
    if (mojo_)
    {
        mojo_dsp_destroy(mojo_);
        mojo_ = nullptr;
    }
}
Steinberg::FUnknown* Processor::createInstance(void* context)
{
    auto* processorContext = static_cast<ProcessorCreateContext*>(context);
    return static_cast<Steinberg::Vst::IAudioProcessor*>(
        new Processor(processorContext->descriptor, processorContext->controllerFUID)
    );
}

tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
    const auto result = AudioEffect::initialize(context);

    if (result != kResultOk)
        return result;

    addAudioInput(
        STR16("Stereo In"),
        SpeakerArr::kStereo
    );

    addAudioOutput(
        STR16("Stereo Out"),
        SpeakerArr::kStereo
    );

    addEventInput(
        STR16("Event In"),
        1
    );

    mojo_ = mojo_dsp_create();

    if (!mojo_)
        return kResultFalse;

    return kResultOk;
}

tresult PLUGIN_API Processor::terminate()
{
    if (mojo_)
    {
        mojo_dsp_destroy(mojo_);
        mojo_ = nullptr;
    }

    return AudioEffect::terminate();
}

tresult PLUGIN_API Processor::setupProcessing(
    ProcessSetup& setup
)
{
    sampleRate_ = setup.sampleRate;
    maxBlockSize_ = setup.maxSamplesPerBlock;

    const auto result = AudioEffect::setupProcessing(setup);

    if (result != kResultOk)
        return result;

    const int32 inputBusCount = getBusCount(MediaTypes::kAudio, BusDirections::kInput);
    const int32 outputBusCount = getBusCount(MediaTypes::kAudio, BusDirections::kOutput);

    input_buses_.resize(static_cast<size_t>(inputBusCount));
    output_buses_.resize(static_cast<size_t>(outputBusCount));

    input_events_.resize(MAX_EVENTS);
    output_events_.resize(MAX_EVENTS);

    input_params_.resize(MAX_PARAM_CHANGES);
    output_params_.resize(MAX_PARAM_CHANGES);

    if (mojo_)
    {
        mojo_dsp_prepare(
            mojo_,
            sampleRate_,
            maxBlockSize_,
            2,
            2
        );
    }

    return kResultOk;
}

tresult PLUGIN_API Processor::setActive(TBool state)
{
    if (state && mojo_)
    {
        mojo_dsp_reset(mojo_);
    }

    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize)
{
    if (symbolicSampleSize == kSample32)
        return kResultTrue;

    return kResultFalse;
}

void Processor::applyParameterChanges(IParameterChanges* changes)
{
    if (!changes || !mojo_)
        return;

    const int32 parameterCount = changes->getParameterCount();

    for (int32 i = 0; i < parameterCount; ++i)
    {
        IParamValueQueue* queue = changes->getParameterData(i);
        
        if (!queue)
            continue;

        const int32 pointCount = queue->getPointCount();

        if (pointCount <= 0)
            continue;

        // TODO: Simple implementation for testing
        // this currently overwrites all but the last param change
        // Implement sample offset in mojo_dsp_set_parameter
        int32 sampleOffset = 0;
        ParamValue value = 0.0;

        if (
            queue->getPoint(
                pointCount - 1,
                sampleOffset,
                value
            ) == kResultTrue
        )
        {
            mojo_dsp_set_parameter(
                mojo_,
                queue->getParameterId(),
                value
            );
        }
    }
}

tresult PLUGIN_API Processor::process(
    ProcessData& data
)
{
    if (!mojo_)
        return kResultFalse;

    applyParameterChanges(data.inputParameterChanges);

    if (data.numSamples <= 0)
        return kResultOk;

    if (data.numInputs <= 0 || data.numOutputs <= 0)
        return kResultOk;

    MoPlugProcessData block {};

    block.frames = data.numSamples;

    if (data.symbolicSampleSize == kSample32)
        block.sample_format = MOPLUG_SAMPLE_F32;
    else
        block.sample_format = MOPLUG_SAMPLE_F64;

    
    const int32 inputBusCount = std::min(data.numInputs, static_cast<int32>(input_buses_.size()));

    for (int32 i = 0; i < inputBusCount; ++i)
    {
        AudioBusBuffers& source = data.inputs[i];

        MoPlugAudioBus& destination = input_buses_[i];

        destination.channel_count = source.numChannels;

        if (data.symbolicSampleSize == kSample32)
        {
            destination.channels = reinterpret_cast<void**>(source.channelBuffers32);
        }
        else
        {
            destination.channels = reinterpret_cast<void**>(source.channelBuffers64);
        }
    }

    block.inputs = input_buses_.data();
    block.input_bus_count = inputBusCount;

    const int32 outputBusCount = std::min(
        data.numOutputs, static_cast<int32>(output_buses_.size())
    );

    for (int32 i = 0; i < outputBusCount; ++i)
    {
        AudioBusBuffers& source = data.outputs[i];

        MoPlugAudioBus& destination = output_buses_[i];

        destination.channel_count = source.numChannels;

        if (data.symbolicSampleSize == kSample32)
        {
            destination.channels = reinterpret_cast<void**>(source.channelBuffers32);
        }
        else
        {
            destination.channels = reinterpret_cast<void**>(source.channelBuffers64);
        }
    }

    block.outputs = output_buses_.data();
    block.output_bus_count = outputBusCount;

    int32 inParamCount = 0;

    if (data.inputParameterChanges)
    {
        const int32 qCount = data.inputParameterChanges->getParameterCount();

        for (int32 qi = 0; qi < qCount; ++qi)
        {
            IParamValueQueue* queue = data.inputParameterChanges->getParameterData(qi);

            if (!queue)
                continue;

            const ParamID id = queue->getParameterId();
            const int32 pCount = queue->getPointCount();

            for (int32 pi = 0; pi < pCount; ++pi)
            {
                if (inParamCount >= static_cast<int32>(input_params_.size()))
                    break;

                int32 sampleOffset = 0;
                ParamValue value = 0.0;

                if (queue->getPoint(pi, sampleOffset, value) != kResultOk)
                    continue;

                MoPlugParamChange& destination = input_params_[inParamCount++];

                destination.id = static_cast<uint32_t>(id);
                destination.value = value;
                destination.sample_offset = sampleOffset;
            }
        }
    }

    block.



    AudioBusBuffers& input = data.inputs[0];
    AudioBusBuffers& output = data.outputs[0];

    if (!input.channelBuffers32 || !output.channelBuffers32)
        return kResultFalse;

    VstAudioBlock block{
        .inputs = input.channelBuffers32,
        .outputs = output.channelBuffers32,
        .input_channels = input.numChannels,
        .output_channels = output.numChannels,
        .frames = data.numSamples,
    };

    mojo_dsp_process_f32(
        mojo_,
        &block
    );

    return kResultOk;
}

tresult PLUGIN_API Processor::setState(IBStream* state)
{
    if (!state || !mojo_)
        return kResultFalse;

    int64 oldPosition = 0;

    if (state->tell(&oldPosition) != kResultOk)
        return kResultFalse;

    int64 endPosition = 0;

    if (state->seek(0, IBStream::kIBSeekEnd, &endPosition) != kResultOk)
        return kResultFalse;

    if (state->seek(oldPosition, IBStream::kIBSeekSet, nullptr) != kResultOk)
        return kResultFalse;

    const auto size = static_cast<size_t>(endPosition - oldPosition);

    if (size == 0)
        return kResultOk;

    std::vector<uint8_t> data(size);

    int32 bytesRead = 0;

    if (state->read(data.data(), static_cast<int32>(size), &bytesRead) != kResultOk)
        return kResultFalse;

    if (static_cast<size_t>(bytesRead) != size)
        return kResultFalse;

    if (mojo_dsp_deserialize_state(mojo_, data.data(), data.size()))
        return kResultOk;

    return kResultFalse;
}

tresult PLUGIN_API Processor::getState(IBStream* state)
{
    if (!state || !mojo_)
        return kResultFalse;

    const size_t size = mojo_dsp_state_size(mojo_);

    if (size == 0)
        return kResultOk;

    std::vector<uint8_t> data(size);

    if (!mojo_dsp_serialize_state(mojo_, data.data(), data.size()))
        return kResultFalse;

    int32 bytesWritten = 0;

    if (state->write(data.data(), static_cast<int32>(data.size()), &bytesWritten) != kResultOk)
        return kResultFalse;

    if (bytesWritten == static_cast<int32>(data.size()))
        return kResultOk;

    return kResultFalse;
}
}
