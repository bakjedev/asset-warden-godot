extends CSGBox3D

var paths = []
func _ready():
	for n in 10:
		paths.append("res://textures/noise/texture_%d.png" % n)
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "mesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "texture", "count": 2, "priority": Thread.PRIORITY_NORMAL},
		]
	})

func done_loading(resource: Resource, path: String, error: int ):
	print("yeahh ", path, " ", error, " ", resource.get_rid())
	material.albedo_texture = resource as Texture
	

func _physics_process(delta: float) -> void:
	if self.position.x < 5.0:
		self.position += Vector3(delta, 0, 0)

func _process(_delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		AssetLoader.load_batch(paths, done_loading, Thread.PRIORITY_NORMAL, "texture")
