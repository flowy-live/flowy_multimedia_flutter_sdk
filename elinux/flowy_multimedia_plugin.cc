#include "include/flowy_multimedia/flowy_multimedia_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>

#include <gst/gst.h>

namespace {

class FlowyMultimediaPlugin : public flutter::Plugin {
public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  FlowyMultimediaPlugin();

  virtual ~FlowyMultimediaPlugin();

private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void FlowyMultimediaPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flowy_multimedia",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlowyMultimediaPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlowyMultimediaPlugin::FlowyMultimediaPlugin() { gst_init(NULL, NULL); }

FlowyMultimediaPlugin::~FlowyMultimediaPlugin() { gst_deinit(); }

void FlowyMultimediaPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::cout << "Method called: " << method_call.method_name() << std::endl;
  std::cout << "Method arguments: " << method_call.arguments() << std::endl;

  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "eLinux";
    result->Success(flutter::EncodableValue(version_stream.str()));
  } else if (method_call.method_name().compare("sendAudio") == 0) {
    result->Success(flutter::EncodableValue("Not implemented"));
  } else if (method_call.method_name().compare("receiveAudio")) {

  } else if (method_call.method_name().compare("receiveVideo")) {

  } else if (method_call.method_name().compare("sendVideo")) {

  } else if (method_call.method_name().compare("startRecord")) {
  } else if (method_call.method_name().compare("stopRecord")) {
  } else {
    result->NotImplemented();
  }
}

} // namespace

void FlowyMultimediaPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlowyMultimediaPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
