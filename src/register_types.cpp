#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

#include "asset_loader.h"

using namespace godot;

void initialize_bakje_extension_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  ClassDB::register_class<AssetLoader>();
  auto* asset_loader = memnew(AssetLoader);
  Engine::get_singleton()->register_singleton("AssetLoader", asset_loader);
}

void uninitialize_bakje_extension_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  auto* asset_loader = AssetLoader::get_singleton();
  if (asset_loader) {
    Engine::get_singleton()->unregister_singleton("AssetLoader");
    memdelete(asset_loader);
  }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT
bakje_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                   const GDExtensionClassLibraryPtr p_library,
                   GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_bakje_extension_module);
  init_obj.register_terminator(uninitialize_bakje_extension_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}