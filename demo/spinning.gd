extends CSGBox3D


func _physics_process(delta):
	rotate(Vector3(0, 1, 0), 5.0 * delta)
