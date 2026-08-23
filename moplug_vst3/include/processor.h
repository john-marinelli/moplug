#pragma once

#include "bridge.h"

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

namespace MoPlugVst3 {

class Processor final
    : public Steinberg::Vst::AudioEffect
{
public:
    Processor();
    ~Processor();

    static Steinberg::FUnknown* createInstance(void*)
    {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(
            new Processor()
        );
    }

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
    void applyParameterChanges(
        Steinberg::Vst::IParameterChanges* changes
    );

    MojoDSPHandle mojo_ = nullptr;

    double sampleRate_ = 44100.0;
    Steinberg::int32 maxBlockSize_ = 0;
};

}

