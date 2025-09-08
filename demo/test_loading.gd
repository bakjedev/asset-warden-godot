extends Node3D

func _ready():
	pass
	AssetLoader.initialize_worker_threads(2)
func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		for i in range(10):
			var path = "res://textures/noise/texture_%d.png" % i
			AssetLoader.load(path, _on_texture_loaded)

func _on_texture_loaded(resource: Resource, path: String, error: int):
	print("Loaded: ", path, " - Valid: ", resource != null, " - Error: ", error)
