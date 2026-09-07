#include "processor.h"
#include "bridge.h"

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/vsttypes.h"

#include <vector>
#include <algorithm>

namespace MoPlugVst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

Processor::Processor(const MoPlugDescriptor* descriptor, Steinberg::FUID controllerFuid)
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

Steinberg::Vst::SpeakerArrangement Processor::speakerArrangementForChannels(int32_t channels)
{
    switch (channels)
    {
        case 1:
            return SpeakerArr::kMono;
        case 2:
            return SpeakerArr::kStereo;
        default:
            return SpeakerArr::kEmpty;
    }
}

tresult Processor::addPluginBus(
    const char* cBusName,
    int32_t busChannels,
    MoPlugBusType busType,
    MoPlugBusDirection busDirection
)
{
    Steinberg::Vst::String128 busName {};
    Steinberg::UString(busName, 128).fromAscii(cBusName);

    if (busType == MOPLUG_BUS_AUDIO)
    {
        auto arrangement = speakerArrangementForChannels(busChannels);
        if (busDirection == MOPLUG_BUS_INPUT)
        {
            addAudioInput(busName, arrangement);
        }
        else
        {
            addAudioOutput(busName, arrangement);
        }
    }
    else if (busType == MOPLUG_BUS_EVENT)
    {
        if (busDirection == MOPLUG_BUS_INPUT)
        {
            addEventInput(busName, busChannels);
        }
        else
        {
            addEventOutput(busName, busChannels);
        }
    }

    return kResultOk;
}

tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
    const auto result = AudioEffect::initialize(context);

    if (result != kResultOk)
        return result;

    for (uint32_t i = 0; i < descriptor_->bus_count; ++i)
    {
        const auto& bus = descriptor_->buses[i];
        addPluginBus(bus.name, bus.channels, bus.type, bus.direction);
    }

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

tresult PLUGIN_API Processor::process(
    ProcessData& data
)
{
    if (!mojo_)
        return kResultFalse;

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

    block.input_params = input_params_.data();
    block.input_param_count = inParamCount;

    int32 inEventCount = 0;

    if (data.inputEvents)
    {
        const int32 eventCount = data.inputEvents->getEventCount();

        for (int32 i = 0; i < eventCount; ++i)
        {
            if (inEventCount >= static_cast<int32>(input_events_.size()))
                break;

            Event source {};

            if (data.inputEvents->getEvent(i, source) != kResultOk)
                continue;

            MoPlugEvent converted {};

            converted.sample_offset = source.sampleOffset;
            converted.bus_index = source.busIndex;

            bool supported = true;

            switch (source.type)
            {
                case Event::kNoteOnEvent:
                {
                    converted.type = MOPLUG_EVENT_NOTE_ON;
                    converted.channel = source.noteOn.channel;
                    converted.pitch = source.noteOn.pitch;
                    converted.note_id = source.noteOn.noteId;
                    converted.value = source.noteOn.velocity;
                    break;
                }
                case Event::kNoteOffEvent:
                {
                    converted.type = MOPLUG_EVENT_NOTE_OFF;
                    converted.channel = source.noteOff.channel;
                    converted.pitch = source.noteOff.pitch;
                    converted.note_id = source.noteOff.noteId;
                    converted.value = source.noteOff.velocity;
                    break;
                }
                case Event::kPolyPressureEvent:
                {
                    converted.type = MOPLUG_EVENT_POLY_PRESSURE;
                    converted.channel = source.polyPressure.channel;
                    converted.pitch = source.polyPressure.pitch;
                    converted.note_id = source.polyPressure.noteId;
                    converted.value = source.polyPressure.pressure;
                    break;
                }
                default:
                {
                    supported = false;
                    break;
                }
            }

            if (supported)
            {
                input_events_[inEventCount++] = converted;
            }
        }
    }

    block.input_events = input_events_.data();
    block.input_event_count = inEventCount;

    if (data.processContext)
    {
        const ProcessContext& source = *data.processContext;

        transport_ = {};

        transport_.state = source.state;
        transport_.sample_rate = source.sampleRate;
        transport_.project_time_samples = source.projectTimeSamples;

        if (source.state & ProcessContext::kContTimeValid)
        {
            transport_.continuous_time_samples = source.continousTimeSamples;
        }

        if (source.state & ProcessContext::kProjectTimeMusicValid)
        {
            transport_.project_time_beats = source.projectTimeMusic;
        }

        if (source.state & ProcessContext::kBarPositionValid)
        {
            transport_.bar_position_beats = source.barPositionMusic;
        }

        if (source.state & ProcessContext::kTempoValid)
        {
            transport_.tempo = source.tempo;
        }

        if (source.state & ProcessContext::kTimeSigValid)
        {
            transport_.time_sig_numerator = source.timeSigNumerator;
            transport_.time_sig_denominator = source.timeSigDenominator;
        }

        if (source.state & ProcessContext::kCycleValid)
        {
            transport_.cycle_start_beats = source.cycleStartMusic;
            transport_.cycle_end_beats = source.cycleEndMusic;
        }

        block.transport = &transport_;
    }
    else
    {
        block.transport = nullptr;

    }

    block.output_events = output_events_.data();
    block.output_event_capacity = static_cast<int32>(output_events_.size());
    block.output_event_count = 0;

    block.output_params = output_params_.data();
    block.output_param_capacity = static_cast<int32>(output_params_.size());
    block.output_param_count = 0;

    mojo_dsp_process(mojo_, &block);
    
    const int32 genEventCount = std::min(block.output_event_count, block.output_event_capacity);
    const int32 genParamCount = std::min(block.output_param_count, block.output_param_capacity);

    if (data.outputEvents)
    {
        for (int32 i = 0; i < genEventCount; ++i)
        {
            const MoPlugEvent& source = output_events_[i];

            Event destination {};

            destination.busIndex = source.bus_index;
            destination.sampleOffset = source.sample_offset;

            switch (source.type)
            {
                case MOPLUG_EVENT_NOTE_ON:
                {
                    destination.type = Event::kNoteOnEvent;
                    destination.noteOn.channel = source.channel;
                    destination.noteOn.pitch = source.pitch;
                    destination.noteOn.noteId = source.note_id;
                    destination.noteOn.velocity = source.value;
                    destination.noteOn.tuning = 0.f;
                    destination.noteOn.length = 0;
                    data.outputEvents->addEvent(destination);
                    break;
                }
                case MOPLUG_EVENT_NOTE_OFF:
                {
                    destination.type = Event::kNoteOffEvent;
                    destination.noteOff.channel = source.channel;
                    destination.noteOff.pitch = source.pitch;
                    destination.noteOff.noteId = source.note_id;
                    destination.noteOff.velocity = source.value;
                    destination.noteOff.tuning = 0.f;
                    data.outputEvents->addEvent(destination);
                    break;
                }
                case MOPLUG_EVENT_POLY_PRESSURE:
                {
                    destination.type = Event::kPolyPressureEvent;
                    destination.polyPressure.channel = source.channel;
                    destination.polyPressure.pitch = source.pitch;
                    destination.polyPressure.noteId = source.note_id;
                    destination.polyPressure.pressure = source.value;
                    data.outputEvents->addEvent(destination);
                    break;
                }

            }
        }
    }

    if (data.outputParameterChanges)
    {
        for (int32 i = 0; i < genParamCount; ++i)
        {
            const MoPlugParamChange& source = output_params_[i];

            const ParamID id = static_cast<ParamID>(source.id);

            int32 qIndex = 0;

            IParamValueQueue* queue = data.outputParameterChanges->addParameterData(id, qIndex);

            if (!queue)
                continue;

            int32 pIndex = 0;

            queue->addPoint(source.sample_offset, source.value, pIndex);
        }
    }

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
