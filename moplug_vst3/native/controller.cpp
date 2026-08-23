#include "controller.h"

#include "ids.h"

#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"

namespace MoPlugVst3 {

using namespace Steinberg;
using namespace Steinberg::Vst;

tresult PLUGIN_API Controller::initialize(FUnknown* context)
{
    const auto result = EditController::initialize(context);

    if (result != kResultOk)
        return result;

    parameters.addParameter(
        STR16("Gain"),
        STR16(""),
        0,
        1.0,
        ParameterInfo::kCanAutomate,
        kGainParameter
    );

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
