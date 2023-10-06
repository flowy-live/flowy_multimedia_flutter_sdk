//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <flowy_multimedia/flowy_multimedia_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) flowy_multimedia_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "FlowyMultimediaPlugin");
  flowy_multimedia_plugin_register_with_registrar(flowy_multimedia_registrar);
}
