#include "controller.h"
#include "ids.h"
#include "processor.h"

#include "public.sdk/source/main/pluginfactory.h"

#define PLUGIN_NAME "Moplug Plugin"
#define PLUGIN_VENDOR "John Marinelli"
#define PLUGIN_URL "https://github.com/john-marinelli"
#define PLUGIN_EMAIL "marinelli.john@proton.me"
#define PLUGIN_VERSION "0.1.0"

using namespace Steinberg;
using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF(
    PLUGIN_VENDOR,
    PLUGIN_URL,
    PLUGIN_EMAIL
)

DEF_CLASS2(
    INLINE_UID_FROM_FUID(MoPlugVst3::ProcessorUID),
    PClassInfo::kManyInstances,
    kVstAudioEffectClass,
    PLUGIN_NAME,
    Vst::kDistributable,
    "Fx",
    PLUGIN_VERSION,
    kVstVersionString,
    MoPlugVst3::Processor::createInstance
)

DEF_CLASS2(
    INLINE_UID_FROM_FUID(MoPlugVst3::ControllerUID),
    PClassInfo::kManyInstances,
    kVstComponentControllerClass,
    PLUGIN_NAME " Controller",
    0,
    "",
    PLUGIN_VERSION,
    kVstVersionString,
    MoPlugVst3::Controller::createInstance
)

END_FACTORY
