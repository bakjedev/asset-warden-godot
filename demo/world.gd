extends Node3D

@export var load_button: Button
@export var delete_button: Button
@export var progress_slider: HSlider
@export var label: Label

var batch_id: int = -1
var spacing = 10.0
var index = 0
var cancel = false

func _ready():
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "ArrayMesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "Texture", "count": 2, "priority": Thread.PRIORITY_NORMAL}
		]
	})
	
	AssetLoader.budgets.set_mb("ArrayMesh", 100)
	
	
	if load_button:
		load_button.pressed.connect(_on_load_button_pressed)
		
	if delete_button:
		delete_button.pressed.connect(_on_delete_button_pressed)

func test(resource: Resource):
	if (!is_instance_valid(resource)):
		return
	var mesh = resource as Mesh
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.mesh = mesh
	var x_pos = (index % 16) * spacing
	var z_pos = (index / 16) * spacing
	mesh_instance.position = Vector3(x_pos, 0, z_pos)
	mesh_instance.rotation_degrees = Vector3(180, 0, 0)
	add_child(mesh_instance)
	index += 1

func done_loading(_resources: Array):
	cancel = false
	load_button.text = "Load"

func _on_delete_button_pressed():
	for child in get_children():
		child.queue_free()
	index = 0

func _on_load_button_pressed():
	if cancel:
		load_button.text = "Load"
		AssetLoader.cancel_batch(batch_id)
		cancel = false
	else:
		load_button.text = "Cancel"
		var paths = []
		for n in 256:
			paths.append("res://sub100world/terrain_sub100_%d.obj" % n)
		batch_id = AssetLoader.load_batch(paths, "ArrayMesh", Thread.PRIORITY_NORMAL, test, done_loading)
		cancel = true

func _process(_delta):
	var progress = AssetLoader.progress_batch(batch_id)
	progress_slider.value = progress
