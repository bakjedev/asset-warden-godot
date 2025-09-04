extends CSGBox3D

func _physics_process(delta: float) -> void:
	if self.position.x < 5.0:
		self.position += Vector3(delta, 0, 0)
