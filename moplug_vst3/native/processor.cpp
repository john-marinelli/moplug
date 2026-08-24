#include "processor.h"
#include "ids.h"

#include "pluginterfaces/vst/ivstparameterchanges.h"

#include <vector>

namespace MoPlugVst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

Processor::Processor()
{
    setControllerClass(ControllerUID);
}

Processor::~Processor()
{
    if (mojo_)
    {
        mojo_dsp_destroy(mojo_);
        mojo_ = nullptr;
    }
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

    if (data.symbolicSampleSize != kSample32)
        return kResultFalse;

    if (data.numInputs <= 0 || data.numOutputs <= 0)
        return kResultOk;

    AudioBusBuffers& input = data.inputs[0];
    AudioBusBuffers& output = data.outputs[0];

    if (!input.channelBuffers32 || !output.channelBuffers32)
        return kResultFalse;

    mojo_dsp_process_f32(
        mojo_,
        input.channelBuffers32,
        output.channelBuffers32,
        input.numChannels,
        output.numChannels,
        data.numSamples
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
