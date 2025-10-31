@tool
extends MeshInstance3D

# IMPORTANT, THIS WAS AI GENERATED. (most of it)

@export var plane_size: Vector2 = Vector2(10, 10)
@export var subdivisions: Vector2i = Vector2i(50, 50)

@export var noise_height: float = 2.0
@export var noise_frequency: float = 0.05
@export var noise_octaves: int = 4
@export var noise_lacunarity: float = 2.0
@export var noise_gain: float = 0.5
@export var noise_seed: int = 0

@export var use_vertex_colors: bool = true
@export var color_gradient: Gradient

@export var regenerate_mesh: bool = false:
	set(value):
		if value:
			generate_mesh()

@export var export_obj_file: bool = false:
	set(value):
		if value:
			export_to_obj()

@export_file("*.obj") var obj_save_path: String = "res://terrain.obj"

var noise: FastNoiseLite
var current_vertices: PackedVector3Array
var current_uvs: PackedVector2Array
var current_normals: PackedVector3Array
var current_indices: PackedInt32Array

func _ready():
	generate_mesh()
	export_to_obj()
	
func generate_mesh():
	noise = FastNoiseLite.new()
	noise.noise_type = FastNoiseLite.TYPE_SIMPLEX
	noise.frequency = noise_frequency
	noise.fractal_octaves = noise_octaves
	noise.fractal_lacunarity = noise_lacunarity
	noise.fractal_gain = noise_gain
	noise.seed = noise_seed
	
	var arrays = []
	arrays.resize(Mesh.ARRAY_MAX)
	
	var vertices = PackedVector3Array()
	var uvs = PackedVector2Array()
	var normals = PackedVector3Array()
	var colors = PackedColorArray()
	
	var vertex_count_x = subdivisions.x + 1
	var vertex_count_z = subdivisions.y + 1
	
	for z in vertex_count_z:
		for x in vertex_count_x:
			var u = float(x) / float(subdivisions.x)
			var v = float(z) / float(subdivisions.y)
			
			var pos_x = (u - 0.5) * plane_size.x
			var pos_z = (v - 0.5) * plane_size.y
			
			var height = noise.get_noise_2d(pos_x * 10, pos_z * 10) * noise_height
			
			vertices.append(Vector3(pos_x, height, pos_z))
			uvs.append(Vector2(u, v))
			
			if use_vertex_colors and color_gradient:
				var normalized_height = (height + noise_height) / (2.0 * noise_height)
				colors.append(color_gradient.sample(normalized_height))
	
	var indices = PackedInt32Array()
	for z in subdivisions.y:
		for x in subdivisions.x:
			var idx = z * vertex_count_x + x
			
			indices.append(idx)
			indices.append(idx + 1)
			indices.append(idx + vertex_count_x)
			
			indices.append(idx + 1)
			indices.append(idx + vertex_count_x + 1)
			indices.append(idx + vertex_count_x)
	
	normals.resize(vertices.size())
	for i in range(normals.size()):
		normals[i] = Vector3.ZERO
	
	for i in range(0, indices.size(), 3):
		var i0 = indices[i]
		var i1 = indices[i + 1]
		var i2 = indices[i + 2]
		
		var v0 = vertices[i0]
		var v1 = vertices[i1]
		var v2 = vertices[i2]
		
		var face_normal = (v1 - v0).cross(v2 - v0).normalized()
		
		normals[i0] += face_normal
		normals[i1] += face_normal
		normals[i2] += face_normal
	
	for i in range(normals.size()):
		normals[i] = normals[i].normalized()
	
	current_vertices = vertices
	current_uvs = uvs
	current_normals = normals
	current_indices = indices
	
	arrays[Mesh.ARRAY_VERTEX] = vertices
	arrays[Mesh.ARRAY_TEX_UV] = uvs
	arrays[Mesh.ARRAY_NORMAL] = normals
	if use_vertex_colors and colors.size() > 0:
		arrays[Mesh.ARRAY_COLOR] = colors
	arrays[Mesh.ARRAY_INDEX] = indices
	
	var array_mesh = ArrayMesh.new()
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)

	mesh = array_mesh
	
	if not get_surface_override_material(0):
		var material = StandardMaterial3D.new()
		material.vertex_color_use_as_albedo = use_vertex_colors
		material.albedo_color = Color.WHITE if use_vertex_colors else Color(0.3, 0.5, 0.3)
		material.roughness = 0.8
		set_surface_override_material(0, material)

func export_to_obj():
	var file = FileAccess.open(obj_save_path, FileAccess.WRITE)
	if file == null:
		push_error("Failed to open file for writing: " + obj_save_path)
		return
	
	file.store_line("# bakje terrain exporter")
	
	file.store_line("o TerrainMesh")
	file.store_line("")
	
	file.store_line("# Vertices")
	for vertex in current_vertices:
		file.store_line("v " + str(vertex.x) + " " + str(vertex.y) + " " + str(vertex.z))
	file.store_line("")
	
	file.store_line("# Texture Coordinates")
	for uv in current_uvs:
		file.store_line("vt " + str(uv.x) + " " + str(uv.y))
	file.store_line("")
	
	file.store_line("# Normals")
	for normal in current_normals:
		file.store_line("vn " + str(normal.x) + " " + str(normal.y) + " " + str(normal.z))
	file.store_line("")
	
	file.store_line("# Faces")
	for i in range(0, current_indices.size(), 3):
		var i0 = current_indices[i] + 1
		var i1 = current_indices[i + 1] + 1
		var i2 = current_indices[i + 2] + 1
		
		# Format: f vertex/texture/normal for each vertex of the triangle
		file.store_line("f " + 
			str(i0) + "/" + str(i0) + "/" + str(i0) + " " +
			str(i1) + "/" + str(i1) + "/" + str(i1) + " " +
			str(i2) + "/" + str(i2) + "/" + str(i2))
	
	file.close()
