#pragma once

#include "bridge.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

namespace MoPlugVst3 {

class Controller final
    : public Steinberg::Vst::EditController
{
public:
    explicit Controller(const PluginDescriptor* descriptor);
    ~Controller() override = default;

    static Steinberg::FUnknown* createInstance(void* context);
    
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;

private:
    const PluginDescriptor* descriptor_;
};
}
