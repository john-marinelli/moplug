#pragma once


#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace MoPlugVst3 {
    
    // TODO: THESE ARE TEMPORARY FIXED IDs!! change once you aren't testing
    // If this isn't changed to, for example a hash or something of a unique name,
    // all moplug vsts will be recognized as the same (I think?)
    static const Steinberg::FUID ProcessorUID(
        0x42817B12,
        0x345A4020,
        0x917B5813,
        0x33547701
    );

    static const Steinberg::FUID ControllerUID(
        0x912AF105,
        0x502040CB,
        0xA1C96210,
        0x70445820
    );

    enum ParameterIDs : Steinberg::Vst::ParamID
    {
        kGainParameter = 0
    };
}
