extends Node3D
var lol = 0

func _ready():
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "mesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "texture", "count": 2, "priority": Thread.PRIORITY_NORMAL}
		]
	})

func done_loading(resources: Array ):
	print("GASDA", resources.size())

func _process(_delta):
	if Input.is_action_just_pressed("ui_accept"):
		var paths = []
		for n in 10:
			paths.append("res://textures/noise/texture_%d.png" % n)
		lol = AssetLoader.load_batch(paths, done_loading, Thread.PRIORITY_NORMAL, "texture")
