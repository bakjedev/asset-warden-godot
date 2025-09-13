extends Node3D

func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		AssetLoader.wake_one()
	if Input.is_action_just_pressed("ui_cancel"):
		AssetLoader.shutdown()
