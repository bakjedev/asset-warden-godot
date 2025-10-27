#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/audio_stream_wav.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/texture2d.hpp"

namespace godot {

namespace SizeCalculator {
constexpr size_t BOOL_SIZE = 1;
constexpr size_t INT_SIZE = 8;
constexpr size_t FLOAT_SIZE = 8;
constexpr size_t VARIANT_SIZE = 32;
constexpr size_t STRING_SIZE = 24;
constexpr size_t ARRAY_SIZE = 32;
constexpr size_t DICTIONARY_SIZE = 48;
constexpr size_t REFCOUNTED_SIZE = 16;
constexpr size_t NODE_SIZE = 256;

size_t packed_int32_array(size_t size);
size_t packed_float32_array(size_t size);
size_t packed_vector2_array(size_t size);
size_t packed_vector3_array(size_t size);
size_t packed_color_array(size_t size);

size_t image(const Ref<Image> &image);
size_t image_data(int width, int height, Image::Format format, bool mipmaps = false);
float pixel(Image::Format format);

size_t texture2d(const Ref<Texture2D> &texture);

size_t array_mesh(const Ref<ArrayMesh> &mesh);

size_t audio_stream_wav(const Ref<AudioStreamWAV> &audio);

size_t calculate_resource(const Ref<Resource> &resource);

size_t estimate_resource(const String &p_path);

} //namespace SizeCalculator
} //namespace godot