#include "include/flowy_multimedia/flowy_multimedia_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include "include/flowy_media.h"

namespace
{

class FlowyMultimediaPlugin : public flutter::Plugin
{
public:
    static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar);

    FlowyMultimediaPlugin();

    virtual ~FlowyMultimediaPlugin();

private:
    // Called when a method is called on this plugin's channel from Dart.
    void HandleMethodCall(const flutter::MethodCall<flutter::EncodableValue>& method_call,
                          std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

    std::unique_ptr<FlowyMedia> m_media;
};

// static
void FlowyMultimediaPlugin::RegisterWithRegistrar(flutter::PluginRegistrar* registrar)
{
    auto channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
        registrar->messenger(), "flowy_multimedia", &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlowyMultimediaPlugin>();

    channel->SetMethodCallHandler([plugin_pointer = plugin.get()](const auto& call, auto result)
                                  { plugin_pointer->HandleMethodCall(call, std::move(result)); });

    registrar->AddPlugin(std::move(plugin));
}

FlowyMultimediaPlugin::FlowyMultimediaPlugin()
{
    std::cout << "Creating FlowyMultimediaPlugin" << std::endl;
    m_media = std::make_unique<FlowyMedia>();
}

FlowyMultimediaPlugin::~FlowyMultimediaPlugin()
{
}

void FlowyMultimediaPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>&             method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result)
{
    std::cout << "Method called: " << method_call.method_name() << std::endl;
    std::cout << "Method arguments: " << method_call.arguments() << std::endl;

    if (method_call.method_name().compare("getPlatformVersion") == 0)
    {
        std::ostringstream version_stream;
        version_stream << "eLinux";
        result->Success(flutter::EncodableValue(version_stream.str()));
    }
    else if (method_call.method_name().compare("init") == 0)
    {
        m_media->Init();
        result->Success(flutter::EncodableValue("Initialized"));
    }
    else if (method_call.method_name().compare("startSendLiveAudio") == 0)
    {
        m_media->StartSendLiveAudio();
        result->Success(flutter::EncodableValue("Started"));
    }
    else if (method_call.method_name().compare("stopSendLiveAudio") == 0)
    {
        m_media->StopSendLiveAudio();
        result->Success(flutter::EncodableValue("Stopped"));
    }
    else if (method_call.method_name().compare("startRecord") == 0)
    {
        m_media->StartRecord();
        result->Success(flutter::EncodableValue("Started recording"));
    }
    else if (method_call.method_name().compare("stopRecord") == 0)
    {
        std::string file_path = m_media->StopRecord();
        result->Success(flutter::EncodableValue("Stopped recording"));
    }
    else
    {
        result->NotImplemented();
    }

    std::cout << "Method call handled" << std::endl;
}

}  // namespace

void FlowyMultimediaPluginRegisterWithRegistrar(FlutterDesktopPluginRegistrarRef registrar)
{
    FlowyMultimediaPlugin::RegisterWithRegistrar(
        flutter::PluginRegistrarManager::GetInstance()->GetRegistrar<flutter::PluginRegistrar>(
            registrar));
}
