#pragma once
#include <godot_cpp/core/object.hpp>

namespace godot {

class AssetLoader : public Object {
  GDCLASS(AssetLoader, Object)

  static AssetLoader* singleton;

protected:
  static void _bind_methods();

public:
  AssetLoader();
  ~AssetLoader();
  static AssetLoader* get_singleton();

  void say_hi();
};

} // namespace godot
