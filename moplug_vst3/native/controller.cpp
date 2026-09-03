#include "controller.h"

#include "bridge.h"
#include "ids.h"

#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"

namespace MoPlugVst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

Controller::Controller(const PluginDescriptor* descriptor) 
    : descriptor_(descriptor)
{
}

Steinberg::FUnknown* Controller::createInstance(void* context) 
{
    auto* descriptor = static_cast<const PluginDescriptor*>(context);

    return static_cast<Steinberg::Vst::IEditController*>(new Controller(descriptor));
}

tresult PLUGIN_API Controller::initialize(FUnknown* context)
{
    const auto result = EditController::initialize(context);

    if (result != kResultOk)
        return result;

    for (uint32_t i = 0; i < descriptor_-> parameter_count; i++)
    {
        const auto& param = descriptor_->parameters[i];

        Steinberg::Vst::ParameterInfo info {};

        info.id = param.id;
        info.stepCount = param.step_count;
        info.defaultNormalizedValue = param.default_value;
        info.flags = Steinberg::Vst::ParameterInfo::kCanAutomate;

        parameters.addParameter(info);
    }

    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    // TODO: Implement mojo to controller state sharing

    return kResultOk;
}
}
