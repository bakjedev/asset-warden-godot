#include "godot_cpp/classes/array_mesh.hpp"
#include "godot_cpp/classes/audio_stream_wav.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/texture2d.hpp"

namespace godot {

namespace SizeCalculator {
constexpr size_t ARRAY_SIZE = 32;
constexpr size_t REFCOUNTED_SIZE = 16;

size_t packed_int32_array(size_t p_size);
size_t packed_float32_array(size_t p_size);
size_t packed_vector2_array(size_t p_size);
size_t packed_vector3_array(size_t p_size);
size_t packed_color_array(size_t p_size);

size_t image(const Ref<Image> &p_image);
size_t image_data(int p_width, int p_height, Image::Format p_format, bool p_mipmaps = false);
float pixel(Image::Format p_format);

size_t texture2d(const Ref<Texture2D> &p_texture);

size_t array_mesh(const Ref<ArrayMesh> &p_mesh);

size_t audio_stream_wav(const Ref<AudioStreamWAV> &p_audio);

size_t calculate_resource(const Ref<Resource> &p_resource);

size_t estimate_resource(const String &p_path);

} //namespace SizeCalculator
} //namespace godot