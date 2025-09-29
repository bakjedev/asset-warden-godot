extends Node3D

@export var load_button: Button

func _ready():
	AssetLoader.initialize({
		"distribution": AssetLoader.DIST_CUSTOM,
		"pools": [
			{"type": "mesh", "count": 2, "priority": Thread.PRIORITY_HIGH},
			{"type": "texture", "count": 2, "priority": Thread.PRIORITY_NORMAL}
		]
	})
	
	if load_button:
		load_button.pressed.connect(_on_load_button_pressed)

func _on_load_button_pressed():
	print("YEAH!")
