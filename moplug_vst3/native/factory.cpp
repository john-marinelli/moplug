#include "bridge.h"
#include "controller.h"
#include "ids.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "processor.h"

#include "public.sdk/source/main/pluginfactory.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"


#define PLUGIN_NAME "Moplug Plugin"
#define PLUGIN_VENDOR "John Marinelli"
#define PLUGIN_URL "https://github.com/john-marinelli"
#define PLUGIN_EMAIL "marinelli.john@proton.me"
#define PLUGIN_VERSION "0.1.0"


static bool registerClasses(Steinberg::CPluginFactory& factory, const PluginDescriptor* descriptor)
{
    Steinberg::FUID controllerFUID= MoPlugVst3::makeFUID(
        std::string(descriptor->stable_id) + ".controller"
    );
    Steinberg::PClassInfo controllerInfo(
        controllerFUID,
        Steinberg::PClassInfo::kManyInstances,
        kVstComponentControllerClass,
        ""
    );
    factory.registerClass(
        &controllerInfo,
        MoPlugVst3::Controller::createInstance,
        const_cast<PluginDescriptor*>(descriptor)
    );

    MoPlugVst3::ProcessorCreateContext processorContext(
        descriptor,
        controllerFUID
    );

    Steinberg::PClassInfo processorInfo(
        MoPlugVst3::makeFUID(std::string(descriptor->stable_id) + ".processor"),
        Steinberg::PClassInfo::kManyInstances,
        kVstAudioEffectClass,
        descriptor->display_name
    );
    factory.registerClass(
        &processorInfo,
        MoPlugVst3::Processor::createInstance,
        &processorContext
    );

    return true;
}

SMTG_EXPORT_SYMBOL
Steinberg::IPluginFactory* PLUGIN_API GetPluginFactory()
{
    static const PluginDescriptor* desc = mojo_get_plugin_descriptor();
    static Steinberg::PFactoryInfo factoryInfo(
        "Company name",
        "website",
        "email",
        Steinberg::PFactoryInfo::kNoFlags
    );
    static Steinberg::CPluginFactory factory(factoryInfo);

    static const bool registered = registerClasses(
        factory,
        desc
    );

    // TODO: deal with failure to register
    (void)registered;

    return &factory;
}
