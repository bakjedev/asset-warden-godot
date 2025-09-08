#include "register_types.h"

#include "asset_loader.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/godot.hpp>

#include <gdextension_interface.h>


using namespace godot;

static AssetLoader* asset_loader_singleton = nullptr;

void initialize_bakje_extension_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    ClassDB::register_class<AssetLoader>();

    asset_loader_singleton = memnew(AssetLoader);

    Engine::get_singleton()->register_singleton("AssetLoader", asset_loader_singleton);
}

void uninitialize_bakje_extension_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    Engine::get_singleton()->unregister_singleton("AssetLoader");

    if (asset_loader_singleton) {
        memdelete(asset_loader_singleton);
        asset_loader_singleton = nullptr;
    }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT bakje_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                              const GDExtensionClassLibraryPtr p_library,
                                              GDExtensionInitialization* r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_bakje_extension_module);
    init_obj.register_terminator(uninitialize_bakje_extension_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}