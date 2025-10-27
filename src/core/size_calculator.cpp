#include "size_calculator.h"
#include "godot_cpp/classes/audio_stream_wav.hpp"
#include "godot_cpp/classes/file_access.hpp"

using namespace godot;

size_t SizeCalculator::packed_int32_array(size_t p_size) {
	return ARRAY_SIZE + (p_size * sizeof(int32_t));
}

size_t SizeCalculator::packed_float32_array(size_t p_size) {
	return ARRAY_SIZE + (p_size * sizeof(float));
}

size_t SizeCalculator::packed_vector2_array(size_t p_size) {
	return ARRAY_SIZE + (p_size * sizeof(float) * 2);
}

size_t SizeCalculator::packed_vector3_array(size_t p_size) {
	return ARRAY_SIZE + (p_size * sizeof(float) * 3);
}

size_t SizeCalculator::packed_color_array(size_t p_size) {
	return ARRAY_SIZE + (p_size * sizeof(float) * 4);
}

size_t SizeCalculator::image(const Ref<Image> &p_image) {
	if (p_image.is_null()) {
		return 0;
	}

	return image_data(p_image->get_width(), p_image->get_height(), p_image->get_format(), p_image->has_mipmaps());
}

size_t SizeCalculator::image_data(int p_width, int p_height, Image::Format p_format, bool p_mipmaps) {
	if (p_width <= 0 || p_height <= 0) {
		return 0;
	}

	auto pixel_size = pixel(p_format);
	size_t size = static_cast<size_t>(p_width * p_height * pixel_size);

	size_t total = REFCOUNTED_SIZE + sizeof(Image) + size;

	if (p_mipmaps) {
		total += (size / 3);
	}

	return total;
}

float SizeCalculator::pixel(Image::Format p_format) { // https://github.com/godotengine/godot/blob/master/core/io/image.cpp#L132
	switch (p_format) {
		case Image::FORMAT_L8:
			return 1;
		case Image::FORMAT_LA8:
			return 2;
		case Image::FORMAT_R8:
			return 1;
		case Image::FORMAT_RG8:
			return 2;
		case Image::FORMAT_RGB8:
			return 3;
		case Image::FORMAT_RGBA8:
			return 4;
		case Image::FORMAT_RGBA4444:
			return 2;
		case Image::FORMAT_RGB565:
			return 2;
		case Image::FORMAT_RF:
			return 4;
		case Image::FORMAT_RGF:
			return 8;
		case Image::FORMAT_RGBF:
			return 12;
		case Image::FORMAT_RGBAF:
			return 16;
		case Image::FORMAT_RH:
			return 2;
		case Image::FORMAT_RGH:
			return 4;
		case Image::FORMAT_RGBH:
			return 6;
		case Image::FORMAT_RGBAH:
			return 8;
		case Image::FORMAT_RGBE9995:
			return 4;
		case Image::FORMAT_DXT1:
			return 1;
		case Image::FORMAT_DXT3:
			return 1;
		case Image::FORMAT_DXT5:
			return 1;
		case Image::FORMAT_RGTC_R:
			return 1;
		case Image::FORMAT_RGTC_RG:
			return 1;
		case Image::FORMAT_BPTC_RGBA:
			return 1;
		case Image::FORMAT_BPTC_RGBF:
			return 1;
		case Image::FORMAT_BPTC_RGBFU:
			return 1;
		case Image::FORMAT_ETC:
			return 1;
		case Image::FORMAT_ETC2_R11:
			return 1;
		case Image::FORMAT_ETC2_R11S:
			return 1;
		case Image::FORMAT_ETC2_RG11:
			return 1;
		case Image::FORMAT_ETC2_RG11S:
			return 1;
		case Image::FORMAT_ETC2_RGB8:
			return 1;
		case Image::FORMAT_ETC2_RGBA8:
			return 1;
		case Image::FORMAT_ETC2_RGB8A1:
			return 1;
		case Image::FORMAT_ETC2_RA_AS_RG:
			return 1;
		case Image::FORMAT_DXT5_RA_AS_RG:
			return 1;
		case Image::FORMAT_ASTC_4x4:
			return 1;
		case Image::FORMAT_ASTC_4x4_HDR:
			return 1;
		case Image::FORMAT_ASTC_8x8:
			return 1;
		case Image::FORMAT_ASTC_8x8_HDR:
			return 1;
		default:
			return 0;
	}
}

size_t SizeCalculator::texture2d(const Ref<Texture2D> &p_texture) {
	if (p_texture.is_null()) {
		return 0;
	}

	size_t size = REFCOUNTED_SIZE + sizeof(Texture2D);

	auto width = p_texture->get_width();
	auto height = p_texture->get_height();

	auto format = Image::FORMAT_RGBA8; // assume rgba8 by default
	auto has_mipmaps = false;
	auto image = p_texture->get_image();
	if (image.is_valid()) {
		format = image->get_format();
		has_mipmaps = image->has_mipmaps();
	}

	size_t data = image_data(width, height, format, has_mipmaps);

	return size + data;
}

size_t SizeCalculator::array_mesh(const Ref<ArrayMesh> &p_mesh) {
	if (p_mesh.is_null()) {
		return 0;
	}

	size_t total = REFCOUNTED_SIZE + sizeof(ArrayMesh);

	for (int i = 0; i < p_mesh->get_surface_count(); i++) {
		Array arrays = p_mesh->surface_get_arrays(i);

		if (arrays.size() > 0) {
			// vertices
			if (arrays[ArrayMesh::ARRAY_VERTEX].get_type() == Variant::PACKED_VECTOR3_ARRAY) {
				PackedVector3Array vertices = arrays[ArrayMesh::ARRAY_VERTEX];
				total += packed_vector3_array(vertices.size());
			}

			// normals
			if (arrays.size() > ArrayMesh::ARRAY_NORMAL && arrays[ArrayMesh::ARRAY_NORMAL].get_type() == Variant::PACKED_VECTOR3_ARRAY) {
				PackedVector3Array normals = arrays[ArrayMesh::ARRAY_NORMAL];
				total += packed_vector3_array(normals.size());
			}

			// tangents
			if (arrays.size() > ArrayMesh::ARRAY_TANGENT && arrays[ArrayMesh::ARRAY_TANGENT].get_type() == Variant::PACKED_FLOAT32_ARRAY) {
				PackedFloat32Array tangents = arrays[ArrayMesh::ARRAY_TANGENT];
				total += packed_float32_array(tangents.size());
			}

			// colors
			if (arrays.size() > ArrayMesh::ARRAY_COLOR && arrays[ArrayMesh::ARRAY_COLOR].get_type() == Variant::PACKED_COLOR_ARRAY) {
				PackedColorArray colors = arrays[ArrayMesh::ARRAY_COLOR];
				total += packed_color_array(colors.size());
			}

			// UV
			if (arrays.size() > ArrayMesh::ARRAY_TEX_UV && arrays[ArrayMesh::ARRAY_TEX_UV].get_type() == Variant::PACKED_VECTOR2_ARRAY) {
				PackedVector2Array uvs = arrays[ArrayMesh::ARRAY_TEX_UV];
				total += packed_vector2_array(uvs.size());
			}

			// UV2
			if (arrays.size() > ArrayMesh::ARRAY_TEX_UV2 && arrays[ArrayMesh::ARRAY_TEX_UV2].get_type() == Variant::PACKED_VECTOR2_ARRAY) {
				PackedVector2Array uv2s = arrays[ArrayMesh::ARRAY_TEX_UV2];
				total += packed_vector2_array(uv2s.size());
			}

			// indices
			if (arrays.size() > ArrayMesh::ARRAY_INDEX && arrays[ArrayMesh::ARRAY_INDEX].get_type() == Variant::PACKED_INT32_ARRAY) {
				PackedInt32Array indices = arrays[ArrayMesh::ARRAY_INDEX];
				total += packed_int32_array(indices.size());
			}

			// bones
			if (arrays.size() > ArrayMesh::ARRAY_BONES && arrays[ArrayMesh::ARRAY_BONES].get_type() == Variant::PACKED_INT32_ARRAY) {
				PackedInt32Array bones = arrays[ArrayMesh::ARRAY_BONES];
				total += packed_int32_array(bones.size());
			}

			// weights
			if (arrays.size() > ArrayMesh::ARRAY_WEIGHTS && arrays[ArrayMesh::ARRAY_WEIGHTS].get_type() == Variant::PACKED_FLOAT32_ARRAY) {
				PackedFloat32Array weights = arrays[ArrayMesh::ARRAY_WEIGHTS];
				total += packed_float32_array(weights.size());
			}
		}
	}

	return total;
}

size_t SizeCalculator::audio_stream_wav(const Ref<AudioStreamWAV> &p_audio) {
	if (p_audio.is_null()) {
		return 0;
	}

	auto data = p_audio->get_data();
	return REFCOUNTED_SIZE + sizeof(AudioStreamWAV) + data.size();
}

size_t SizeCalculator::calculate_resource(const Ref<Resource> &p_resource) {
	if (p_resource.is_null()) {
		return 0;
	}

	if (p_resource->is_class("Image")) {
		return image(Object::cast_to<Image>(p_resource.ptr()));
	}

	if (p_resource->is_class("Texture") || p_resource->is_class("Texture2D")) {
		return texture2d(Object::cast_to<Texture2D>(p_resource.ptr()));
	}

	if (p_resource->is_class("ArrayMesh")) {
		return array_mesh(Object::cast_to<ArrayMesh>(p_resource.ptr()));
	}

	if (p_resource->is_class("AudioStreamWAV")) {
		return audio_stream_wav(Object::cast_to<AudioStreamWAV>(p_resource.ptr()));
	}

	return REFCOUNTED_SIZE + 256;
}

size_t SizeCalculator::estimate_resource(const String &p_path) {
	if (p_path.is_empty() || !FileAccess::file_exists(p_path)) {
		return 0;
	}

	auto file = FileAccess::open(p_path, FileAccess::READ);
	if (!file.is_valid()) {
		return 0;
	}

	size_t file_size = file->get_length();
	file->close();

	auto ext = p_path.get_extension().to_lower();

	auto ratio = 2.0f;

	if (ext == "png") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "jpg" || ext == "jpeg") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "obj") {
		ratio = 3.0f;
	} else if (ext == "gltf") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "glb") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "fbx") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "wav") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "ogg") {
		ratio = 2.0f; // dont know yet
	} else if (ext == "mp3") {
		ratio = 2.0f; // dont know yet
	}

	return static_cast<size_t>(file_size / ratio);
}