extends Control
## Main playable vertical slice: character creation through death/restart.

enum Mode { TITLE, CREATE, PLAY, INVENTORY, CRAFT, PAUSE, DEAD, RANGED }

@onready var world_view: Node2D = $WorldHost/WorldView
@onready var log_label: RichTextLabel = $UI/LogPanel/Log
@onready var status_label: Label = $UI/StatusBar/Status
@onready var prompt_label: Label = $UI/PromptBar/Prompt
@onready var menu_panel: PanelContainer = $UI/MenuPanel
@onready var menu_list: ItemList = $UI/MenuPanel/Margin/VBox/MenuList
@onready var menu_title: Label = $UI/MenuPanel/Margin/VBox/MenuTitle

var mode: Mode = Mode.TITLE
var selected_race: String = "race_human"
var selected_class: String = "class_adventurer"
var seed_value: int = 42
var ranged_targets: Array[String] = []
var ranged_index: int = 0
var menu_actions: Array[Dictionary] = []


func _ready() -> void:
	$InputRouter.command.connect(_on_command)
	_show_title()
	_refresh_prompts()


func _show_title() -> void:
	mode = Mode.TITLE
	menu_panel.visible = true
	menu_title.text = "Brassdeep"
	menu_actions = [
		{"id": "new", "label": "New Expedition"},
		{"id": "load", "label": "Load Save"},
		{"id": "quit", "label": "Quit"},
	]
	_rebuild_menu()


func _show_create() -> void:
	mode = Mode.CREATE
	menu_panel.visible = true
	menu_title.text = "Character Creation"
	menu_actions = [
		{"id": "race_human", "label": "Race: Human"},
		{"id": "race_automaton", "label": "Race: Automaton"},
		{"id": "class_adventurer", "label": "Class: Adventurer"},
		{"id": "class_engineer", "label": "Class: Engineer"},
		{"id": "seed", "label": "Seed: %d (A to roll)" % seed_value},
		{"id": "start", "label": "Begin Descent"},
		{"id": "back", "label": "Back"},
	]
	_rebuild_menu()


func _start_game() -> void:
	if not GameServices.start_new_game(seed_value, selected_race, selected_class):
		status_label.text = "Failed to start: content errors"
		return
	mode = Mode.PLAY
	menu_panel.visible = false
	GameServices.sim.log_message.connect(_on_log)
	_refresh_world()
	_refresh_status()


func _rebuild_menu() -> void:
	menu_list.clear()
	for action in menu_actions:
		menu_list.add_item(str(action["label"]))
	if menu_list.item_count > 0:
		menu_list.select(0)


func _on_command(action: String, payload: Dictionary) -> void:
	match mode:
		Mode.TITLE, Mode.CREATE, Mode.PAUSE, Mode.INVENTORY, Mode.CRAFT, Mode.DEAD:
			_handle_menu_command(action, payload)
		Mode.PLAY:
			_handle_play_command(action, payload)
		Mode.RANGED:
			_handle_ranged_command(action, payload)


func _handle_menu_command(action: String, _payload: Dictionary) -> void:
	if action == "move":
		var dx: int = int(_payload.get("dx", 0))
		var dy: int = int(_payload.get("dy", 0))
		if menu_list.item_count == 0:
			return
		var idx := menu_list.get_selected_items()[0] if menu_list.get_selected_items().size() > 0 else 0
		if dy < 0 or dx < 0:
			idx = maxi(0, idx - 1)
		elif dy > 0 or dx > 0:
			idx = mini(menu_list.item_count - 1, idx + 1)
		menu_list.select(idx)
		return
	if action == "cancel":
		if mode == Mode.CREATE or mode == Mode.PAUSE or mode == Mode.INVENTORY or mode == Mode.CRAFT:
			if mode == Mode.PAUSE or mode == Mode.INVENTORY or mode == Mode.CRAFT:
				mode = Mode.PLAY
				menu_panel.visible = false
			else:
				_show_title()
		return
	if action != "confirm":
		return
	if menu_list.get_selected_items().is_empty():
		return
	var chosen: Dictionary = menu_actions[menu_list.get_selected_items()[0]]
	_activate_menu_id(str(chosen["id"]))


func _activate_menu_id(id: String) -> void:
	match mode:
		Mode.TITLE:
			match id:
				"new":
					_show_create()
				"load":
					var result := GameServices.load_game()
					if result.get("ok", false):
						mode = Mode.PLAY
						menu_panel.visible = false
						GameServices.sim.log_message.connect(_on_log)
						_refresh_world()
						_refresh_status()
						_on_log("Save loaded.")
					else:
						status_label.text = "Load failed: %s" % result.get("reason", "?")
				"quit":
					get_tree().quit()
		Mode.CREATE:
			match id:
				"race_human", "race_automaton":
					selected_race = id
					status_label.text = "Selected %s" % id
				"class_adventurer", "class_engineer":
					selected_class = id
					status_label.text = "Selected %s" % id
				"seed":
					seed_value = randi_range(1, 999999)
					_show_create()
				"start":
					_start_game()
				"back":
					_show_title()
		Mode.PAUSE:
			match id:
				"resume":
					mode = Mode.PLAY
					menu_panel.visible = false
				"save":
					status_label.text = "Saved" if GameServices.save_game() else "Save failed"
				"title":
					_show_title()
		Mode.INVENTORY:
			if id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
			elif id.begins_with("spend:"):
				GameServices.sim.spend_skill_point(id.substr(6))
				_open_character()
				_refresh_status()
			elif id.begins_with("equip:"):
				var iid := id.substr(6)
				var player := GameServices.sim.get_player()
				for entry in player.inventory:
					var item: SimItem = entry
					if item.instance_id == iid:
						InventorySystem.equip(player, item)
						_open_inventory()
						_refresh_status()
						break
		Mode.CRAFT:
			if id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
			elif id.begins_with("recipe:"):
				GameServices.sim.craft_recipe(id.substr(7))
				_refresh_world()
				_refresh_status()
				_open_craft()
		Mode.DEAD:
			if id == "restart":
				_show_create()
			elif id == "title":
				_show_title()


func _handle_play_command(action: String, payload: Dictionary) -> void:
	if GameServices.sim.player_dead:
		_show_dead()
		return
	match action:
		"move":
			GameServices.sim.try_player_move(int(payload.get("dx", 0)), int(payload.get("dy", 0)))
			_after_sim()
		"confirm", "interact":
			var result := GameServices.sim.interact()
			if result.get("workbench", false):
				_open_craft()
			_after_sim()
		"inventory":
			_open_inventory()
		"character":
			_open_character()
		"pause":
			_open_pause()
		"quick_item":
			GameServices.sim.use_quick_item()
			_after_sim()
		"ranged_mode":
			_enter_ranged()
		"inspect":
			status_label.text = "Inspect: tiles and actors use fog-of-war visibility."
		"camera_peek":
			if world_view and world_view.has_method("peek_camera"):
				world_view.peek_camera(float(payload.get("dx", 0.0)), float(payload.get("dy", 0.0)))
		"craft_slot":
			var recipes := ["recipe_reinforced_wrench", "recipe_filter_mask", "recipe_calibrated_pepperbox"]
			var slot: int = int(payload.get("slot", 0))
			if slot >= 0 and slot < recipes.size():
				GameServices.sim.craft_recipe(recipes[slot])
				_after_sim()


func _handle_ranged_command(action: String, payload: Dictionary) -> void:
	if action == "cancel":
		mode = Mode.PLAY
		_refresh_prompts()
		return
	if action == "cycle_next" or (action == "move" and int(payload.get("dx", 0)) > 0):
		if ranged_targets.size() > 0:
			ranged_index = (ranged_index + 1) % ranged_targets.size()
			_refresh_prompts()
		return
	if action == "cycle_prev" or (action == "move" and int(payload.get("dx", 0)) < 0):
		if ranged_targets.size() > 0:
			ranged_index = (ranged_index - 1 + ranged_targets.size()) % ranged_targets.size()
			_refresh_prompts()
		return
	if action == "confirm" or action == "ranged_mode":
		if ranged_targets.is_empty():
			mode = Mode.PLAY
			return
		# Equip pepperbox if needed
		var player := GameServices.sim.get_player()
		if player.weapon_range() <= 0:
			for entry in player.inventory:
				var item: SimItem = entry
				if item.def_id == "item_pepperbox" or item.range_tiles > 0:
					InventorySystem.equip(player, item)
					break
		GameServices.sim.try_player_ranged(ranged_targets[ranged_index])
		mode = Mode.PLAY
		_after_sim()


func _enter_ranged() -> void:
	ranged_targets.clear()
	var player := GameServices.sim.get_player()
	for entry in GameServices.sim.actors:
		var actor: SimActor = entry
		if actor.is_player() or not actor.alive:
			continue
		if GameServices.sim.world.is_visible(actor.pos.x, actor.pos.y):
			ranged_targets.append(actor.id)
	if ranged_targets.is_empty():
		status_label.text = "No visible targets."
		return
	ranged_index = 0
	mode = Mode.RANGED
	_refresh_prompts()


func _open_inventory() -> void:
	mode = Mode.INVENTORY
	menu_panel.visible = true
	menu_title.text = "Inventory"
	menu_actions.clear()
	var player := GameServices.sim.get_player()
	for entry in player.inventory:
		var item: SimItem = entry
		var label := "%s x%d" % [item.name, item.stack]
		if item.description != "":
			label = "%s — %s" % [label, item.description]
		if item.equip_slot != "none":
			menu_actions.append({"id": "equip:%s" % item.instance_id, "label": "Equip %s" % label})
		else:
			menu_actions.append({"id": "noop", "label": label})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_craft() -> void:
	mode = Mode.CRAFT
	menu_panel.visible = true
	menu_title.text = "Workbench"
	menu_actions = [
		{"id": "recipe:recipe_reinforced_wrench", "label": "Reinforce Wrench"},
		{"id": "recipe:recipe_filter_mask", "label": "Assemble Filter Mask"},
		{"id": "recipe:recipe_calibrated_pepperbox", "label": "Calibrate Pepperbox"},
		{"id": "close", "label": "Close"},
	]
	_rebuild_menu()


func _open_character() -> void:
	mode = Mode.INVENTORY
	menu_panel.visible = true
	menu_title.text = "Character"
	var p := GameServices.sim.get_player()
	menu_actions = [
		{"id": "noop", "label": "HP %d/%d  SP %d" % [p.hp, p.max_hp, p.skill_points]},
		{"id": "noop", "label": "Melee %d/%d  Ranged %d/%d" % [p.total_melee_accuracy(), p.total_melee_damage(), p.total_ranged_accuracy(), p.total_ranged_damage()]},
		{"id": "noop", "label": "Defense %d  Engineering %d" % [p.total_defense(), p.engineering]},
	]
	if p.skill_points > 0:
		menu_actions.append({"id": "spend:melee", "label": "Spend SP: Melee"})
		menu_actions.append({"id": "spend:ranged", "label": "Spend SP: Ranged"})
		menu_actions.append({"id": "spend:defense", "label": "Spend SP: Defense"})
		menu_actions.append({"id": "spend:engineering", "label": "Spend SP: Engineering"})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_pause() -> void:
	mode = Mode.PAUSE
	menu_panel.visible = true
	menu_title.text = "Paused"
	menu_actions = [
		{"id": "resume", "label": "Resume"},
		{"id": "save", "label": "Save"},
		{"id": "title", "label": "Abort to Title"},
	]
	_rebuild_menu()


func _show_dead() -> void:
	mode = Mode.DEAD
	menu_panel.visible = true
	menu_title.text = "Fallen"
	menu_actions = [
		{"id": "noop", "label": GameServices.sim.death_summary},
		{"id": "restart", "label": "New Seed / Restart"},
		{"id": "title", "label": "Title"},
	]
	_rebuild_menu()


func _after_sim() -> void:
	# Brief presentation lock so input does not stack while the view updates.
	GameServices.sim.awaiting_visual = true
	_refresh_world()
	_refresh_status()
	if GameServices.sim.player_dead:
		GameServices.sim.awaiting_visual = false
		_show_dead()
		return
	get_tree().create_timer(0.1).timeout.connect(_clear_visual_lock)


func _clear_visual_lock() -> void:
	if GameServices.sim != null:
		GameServices.sim.awaiting_visual = false
		_refresh_prompts()


func _refresh_world() -> void:
	if world_view:
		world_view.queue_redraw()
		world_view.center_on_player()


func _refresh_status() -> void:
	var p := GameServices.sim.get_player()
	if p == null:
		return
	var device := GameServices.last_input_device
	status_label.text = "HP %d/%d | Turn %d | Eng %d | SP %d | Seed %d | Input %s" % [
		p.hp, p.max_hp, GameServices.sim.turn_index, p.engineering, p.skill_points, GameServices.sim.seed_value, device
	]
	_refresh_prompts()


func _refresh_prompts() -> void:
	var c := GameServices.last_input_device == "controller"
	if mode == Mode.RANGED and ranged_targets.size() > 0:
		var t := GameServices.sim.get_actor(ranged_targets[ranged_index])
		prompt_label.text = "Target: %s | %s fire | %s cancel | bumpers cycle" % [
			t.name if t else "?", ("A" if c else "Enter"), ("B" if c else "Esc")
		]
		return
	if c:
		prompt_label.text = "Move: LStick/DPad | A interact/attack | X oil | Y inventory | RT ranged | Start pause | View sheet"
	else:
		prompt_label.text = "Move: WASD/Arrows | Space interact | F ranged | Q oil | I inventory | Esc pause | C sheet"


func _on_log(text: String) -> void:
	log_label.append_text(text + "\n")
