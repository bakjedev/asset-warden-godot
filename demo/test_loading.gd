extends Node3D

var test : int = 1

func _ready():
	pass

func done_loading(resource: Resource, path: String, error: int ):
	print("yeahh ", path, " ", test)
	test = test + 1

func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		for n in 10:
			AssetLoader.load(String("res://textures/noise/texture_%d.png") % n, done_loading, Thread.PRIORITY_NORMAL, "texture")
	if Input.is_action_just_pressed("ui_cancel"):
		AssetLoader.initialize(2)
		test = 1 
