#include "register_types.h"

#include "core/asset_loader.h"
#include "debugger/debug_receiver.h"
#include "debugger/debug_sender.h"
#include "editor/editor_plugin.h"
#include "editor/panel.h"

#include <godot_cpp/classes/engine.hpp>

using namespace godot;

static AssetLoader *asset_loader_singleton;

void initialize_bakje_extension_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE:
			break;
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			ClassDB::register_class<AssetLoader>();
			ClassDB::register_class<DebugSender>();

			asset_loader_singleton = memnew(AssetLoader);
			Engine::get_singleton()->register_singleton("AssetLoader", AssetLoader::get_singleton());
			break;
		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			ClassDB::register_internal_class<AssetWardenGraph>();
			ClassDB::register_internal_class<AssetWardenPanel>();
			ClassDB::register_internal_class<AssetWardenEditorPlugin>();
			ClassDB::register_class<DebugReceiver>();
			ClassDB::register_class<DebugReceiverPlugin>();
			EditorPlugins::add_by_type<AssetWardenEditorPlugin>();
			break;
		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

void uninitialize_bakje_extension_module(ModuleInitializationLevel p_level) {
	switch (p_level) {
		case MODULE_INITIALIZATION_LEVEL_CORE:
			break;
		case MODULE_INITIALIZATION_LEVEL_SERVERS:
			break;
		case MODULE_INITIALIZATION_LEVEL_SCENE:
			Engine::get_singleton()->unregister_singleton("AssetLoader");
			memdelete(asset_loader_singleton);
			break;
		case MODULE_INITIALIZATION_LEVEL_EDITOR:
			break;
		case MODULE_INITIALIZATION_LEVEL_MAX:
			break;
	}
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT bakje_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_bakje_extension_module);
	init_obj.register_terminator(uninitialize_bakje_extension_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}