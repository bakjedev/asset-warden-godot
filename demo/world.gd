extends Node3D

@export var load_button: Button
@export var progress_label: Label

var batch_id: int = -1
var done: bool = false
var loaded_count: int = 0
var are_meshes: bool = false

func _ready():
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "mesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "texture", "count": 2, "priority": Thread.PRIORITY_NORMAL}
		]
	})
	
	if load_button:
		load_button.pressed.connect(_on_load_button_pressed)

func done_loading(_res: Array):
	print("DONE!")
	done = true
	loaded_count = _res.size()
	if _res[0] is Mesh and not are_meshes:
		are_meshes = true
	
	var spacing = 10.0
	var index = 0
	
	for resource in _res:
		var mesh = resource as Mesh
		var mesh_instance = MeshInstance3D.new()
		mesh_instance.mesh = mesh
		var x_pos = (index % 16) * spacing
		var z_pos = (index / 16.0) * spacing
		mesh_instance.position = Vector3(x_pos, 0, z_pos)
		mesh_instance.rotation_degrees = Vector3(180, 0, 0)
		add_child(mesh_instance)
		index += 1


func _on_load_button_pressed():
	var paths = []
	for n in 256:
		paths.append("res://sub100world/terrain_sub100_%d.obj" % n)
	batch_id = AssetLoader.load_batch(paths, done_loading,Thread.PRIORITY_NORMAL, "mesh")

func _process(_delta):
	if done:
		progress_label.text = "YES DONE " + String.num(loaded_count) + " Are Meshes? " + String.num(are_meshes)
	else:
		var status = AssetLoader.status_batch(batch_id)
		var progress = AssetLoader.progress_batch(batch_id)
		progress_label.text = "Status: " + String.num(status) + " Progress: " + String.num(progress)
