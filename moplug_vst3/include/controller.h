#pragma once

#include "public.sdk/source/vst/vsteditcontroller.h"

namespace MoPlugVst3 {

class Controller final
    : public Steinberg::Vst::EditController
{
public:
    static Steinberg::FUnknown* createInstance(void*)
    {
        return static_cast<Steinberg::Vst::IEditController*>(new Controller());
    }
    
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
};
}
