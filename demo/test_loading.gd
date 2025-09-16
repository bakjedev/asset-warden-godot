extends Node3D

var test : int = 1

func _ready():
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "texture", "count": 5, "priority": Thread.PRIORITY_HIGH},
			{"type": "mesh", "count": 5, "priority": Thread.PRIORITY_NORMAL}
		]
	})

func done_loading(resource: Resource, path: String, error: int ):
	print("yeahh ", path, " ", test)
	test = test + 1

func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		for n in 10:
			AssetLoader.load(String("res://textures/noise/texture_%d.png") % n, done_loading, Thread.PRIORITY_NORMAL, "texture")
