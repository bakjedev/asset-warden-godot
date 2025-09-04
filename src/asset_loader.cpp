#include "asset_loader.h"
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

AssetLoader *AssetLoader::singleton = nullptr;

void AssetLoader::_bind_methods() {
    ClassDB::bind_method(D_METHOD("say_hi"), &AssetLoader::say_hi);

    ClassDB::bind_static_method("AssetLoader", D_METHOD("get_singleton"), &AssetLoader::get_singleton);
}

AssetLoader::AssetLoader() {
    if (singleton == nullptr) {
        singleton = this;
    }
}

AssetLoader::~AssetLoader() {}

AssetLoader* AssetLoader::get_singleton() {
    return singleton;
}

void AssetLoader::say_hi() {
    print_line("HIIII");
    auto start_time = Time::get_singleton()->get_ticks_msec();
    for (int64_t i = 0; i < 20; ++i) {
        auto texture = ResourceLoader::get_singleton()->load(String("res://textures/texture_%d.png") % i);
        if (texture.is_valid()) print_line("loaded valid shit");
    }
    auto end_time = Time::get_singleton()->get_ticks_msec();
    print_line(String("%d ms") % static_cast<int64_t>(end_time - start_time));
}

} // namespace godot