extends Node
## Narrow application service locator. Avoid dumping game state here.

var content: ContentDB = ContentDB.new()
var sim: GameSim = GameSim.new()
var last_input_device: String = "keyboard" # keyboard | controller
var save_path: String = "user://brassdeep_save.json"


func _ready() -> void:
	if not content.load_all():
		push_error("Content validation failed: %s" % ", ".join(content.errors))
	sim.content = content


func start_new_game(seed_value: int, race_id: String, class_id: String) -> bool:
	return sim.new_game(seed_value, race_id, class_id)


func save_game() -> bool:
	return SaveSystem.save_to_file(save_path, sim)


func load_game() -> Dictionary:
	var result := SaveSystem.load_from_file(save_path, content)
	if result.get("ok", false):
		sim = result["sim"]
	return result
