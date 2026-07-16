extends SceneTree
## Boots the real main scene and runs AutomationDriver via --e2e-test path.


func _initialize() -> void:
	Automation.enabled = true
	Automation.e2e_test = true
	Automation.instant_animations = true
	Automation.capture_screenshots = OS.get_environment("BRASSDEEP_CAPTURE") == "1"
	call_deferred("_boot")


func _boot() -> void:
	var err := change_scene_to_file("res://presentation/scenes/main.tscn")
	if err != OK:
		push_error("Failed to load main scene: %d" % err)
		quit(1)
