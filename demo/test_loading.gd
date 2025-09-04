extends Node3D

func _process(delta: float) -> void:
	if Input.is_action_just_pressed("ui_accept"):
		AssetLoader.say_hi()
		#var start_time = Time.get_ticks_msec()
		#for i in range(20):
		#	var texture = ResourceLoader.load_threaded_request("res://textures/texture_%d.png" % i)
		#var end_time = Time.get_ticks_msec()
		#print("%d ms" % (end_time - start_time))
