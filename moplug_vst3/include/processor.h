#pragma once

#include "bridge.h"

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include <vector>

namespace MoPlugVst3 {

typedef struct ProcessorCreateContext {
    const MoPlugDescriptor* descriptor;
    Steinberg::FUID controllerFUID;
} ProcessorCreateContext;

class Processor final
    : public Steinberg::Vst::AudioEffect
{
public:
    explicit Processor(const MoPlugDescriptor* descriptor, Steinberg::FUID controllerFuid);
    ~Processor();

    static Steinberg::FUnknown* createInstance(void* context);


    Steinberg::tresult PLUGIN_API initialize(
        Steinberg::FUnknown* context
    ) override;

    Steinberg::tresult PLUGIN_API terminate() override;

    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& setup
    ) override;

    Steinberg::tresult PLUGIN_API setActive(
        Steinberg::TBool state
    ) override;

    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolicSampleSize
    ) override;

    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data
    ) override;

    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* state
    ) override;

    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* state
    ) override;

private:
    static Steinberg::Vst::SpeakerArrangement speakerArrangementForChannels(int32_t channels);
    Steinberg::tresult addPluginBus(
        const char* cBusName,
        int32_t busChannels,
        MoPlugBusType busType,
        MoPlugBusDirection busDirection
    );

    MojoDSPHandle mojo_ = nullptr;

    double sampleRate_ = 44100.0;
    Steinberg::int32 maxBlockSize_ = 0;

    const MoPlugDescriptor* descriptor_;

    std::vector<MoPlugAudioBus> input_buses_;
    std::vector<MoPlugAudioBus> output_buses_;

    std::vector<MoPlugEvent> input_events_;
    std::vector<MoPlugEvent> output_events_;

    std::vector<MoPlugParamChange> input_params_;
    std::vector<MoPlugParamChange> output_params_;

    MoPlugTransport transport_ {};

    static constexpr size_t MAX_EVENTS = 4096;
    static constexpr size_t MAX_PARAM_CHANGES = 8192;
};

}

