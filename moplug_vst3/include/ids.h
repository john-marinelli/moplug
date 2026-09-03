#pragma once


#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"
#include <string_view>

namespace MoPlugVst3 {

    constexpr uint32_t fnv1a32(
        std::string_view text,
        uint32_t seed = 2166136261u
    )
    {
        uint32_t hash = seed;
        
        for (char c : text)
        {
            hash ^= static_cast<uint8_t>(c);
            hash *= 16777619u;
        }

        return hash;
    }

    inline Steinberg::FUID makeFUID(std::string_view name)
    {
        const uint32_t a = fnv1a32(name, 2166136261u);
        const uint32_t b = fnv1a32(name, 0x9E3779B9u);
        const uint32_t c = fnv1a32(name, 0x85EBCA6Bu);
        const uint32_t d = fnv1a32(name, 0xC2B2AE35u);

        return Steinberg::FUID(a, b, c, d);
    }

    enum ParameterIDs : Steinberg::Vst::ParamID
    {
        kGainParameter = 0
    };
}
