extends Node3D

@export var boxes: Array[Node] = []

func _ready():
	print("GDScript AssetLoader instance: ", AssetLoader.get_instance_id())
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "mesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "texture", "count": 2, "priority": Thread.PRIORITY_NORMAL}
		]
	})

func done_loading(resource: Resource, path: String, _error: int ):
	boxes[int(path[-5])].material.albedo_texture = resource as Texture
	print("Set texture for ", path[-5])

func _process(_delta):
	if Input.is_action_just_pressed("ui_accept"):
		var paths = []
		for n in 10:
			paths.append("res://textures/noise/texture_%d.png" % n)
		AssetLoader.load_batch(paths, done_loading, Thread.PRIORITY_NORMAL, "texture")
