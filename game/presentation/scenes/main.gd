extends Control
## Phase 2 playable shell: hub, expedition, skills, merchants, crafting, HUD.

enum Mode {
	TITLE, CREATE, PLAY, INVENTORY, CRAFT, SKILLS, MERCHANT, STORAGE, HEALER,
	PAUSE, DEAD, RANGED, CHARACTER
}

@onready var world_view: Node2D = $WorldHost/WorldView
@onready var log_label: RichTextLabel = $UI/LogPanel/Log
@onready var status_label: Label = $UI/StatusBar/Status
@onready var prompt_label: Label = $UI/PromptBar/Prompt
@onready var menu_panel: PanelContainer = $UI/MenuPanel
@onready var menu_list: ItemList = $UI/MenuPanel/Margin/VBox/MenuList
@onready var menu_title: Label = $UI/MenuPanel/Margin/VBox/MenuTitle
@onready var hud_label: Label = $UI/HudBar/Hud

var mode: Mode = Mode.TITLE
var selected_race: String = "race_human"
var selected_class: String = "class_adventurer"
var seed_value: int = 42
var ranged_targets: Array[String] = []
var ranged_index: int = 0
var menu_actions: Array[Dictionary] = []
var craft_station: String = "workbench"
var active_merchant: String = ""
var inv_filter: String = "all"
var inv_sort: String = "category"
var pending_skill: String = ""
var sequencer: AnimSequencer
var race_ids: Array[String] = []
var class_ids: Array[String] = []


func _ready() -> void:
	Automation.parse_cmdline()
	if not has_node("AnimSequencer"):
		sequencer = AnimSequencer.new()
		sequencer.name = "AnimSequencer"
		add_child(sequencer)
	else:
		sequencer = $AnimSequencer
	if world_view and world_view.has_method("bind_sequencer"):
		world_view.bind_sequencer(sequencer)
	$InputRouter.command.connect(_on_command)
	_cache_content_ids()
	_apply_ui_settings()
	_show_title()
	_refresh_prompts()
	if Automation.smoke_test or Automation.e2e_test:
		call_deferred("_run_automation")


func _run_automation() -> void:
	Automation.instant_animations = true
	if sequencer:
		sequencer.speed = AnimSequencer.Speed.FAST
	var driver := AutomationDriver.new(self)
	driver.finished.connect(_on_automation_finished)
	if Automation.smoke_test and not Automation.e2e_test:
		await driver.run_smoke_flow()
	else:
		await driver.run_full_flow()


func _on_automation_finished(ok: bool, message: String) -> void:
	print("AUTOMATION_RESULT: ", "PASS" if ok else "FAIL", " — ", message)
	get_tree().quit(0 if ok else 1)


func _cache_content_ids() -> void:
	race_ids.clear()
	class_ids.clear()
	for id in GameServices.content.races.keys():
		race_ids.append(str(id))
	for id in GameServices.content.classes.keys():
		class_ids.append(str(id))
	race_ids.sort()
	class_ids.sort()
	if not race_ids.is_empty():
		selected_race = race_ids[0]
	if not class_ids.is_empty():
		selected_class = class_ids[0]


func _apply_ui_settings() -> void:
	var settings: Dictionary = GameServices.sim.settings if GameServices.sim else {}
	var text_size := int(settings.get("text_size", 16))
	status_label.add_theme_font_size_override("font_size", text_size)
	prompt_label.add_theme_font_size_override("font_size", maxi(12, text_size - 2))
	if settings.get("anim_speed", "normal") == "fast":
		sequencer.speed = AnimSequencer.Speed.FAST
	else:
		sequencer.speed = AnimSequencer.Speed.NORMAL


func _show_title() -> void:
	mode = Mode.TITLE
	menu_panel.visible = true
	menu_title.text = "Brassdeep"
	menu_actions = [
		{"id": "new", "label": "New Operative"},
		{"id": "load", "label": "Load Save"},
		{"id": "quit", "label": "Quit"},
	]
	_rebuild_menu()


func _show_create() -> void:
	mode = Mode.CREATE
	menu_panel.visible = true
	menu_title.text = "Character Creation"
	menu_actions.clear()
	for rid in race_ids:
		var race: Dictionary = GameServices.content.get_race(rid)
		var mark := ">" if rid == selected_race else " "
		menu_actions.append({
			"id": "race:%s" % rid,
			"label": "%s Race: %s  HP%d Melee%d Eng%d" % [
				mark, race.get("name", rid), int(race.get("base_hp", 0)),
				int(race.get("melee_accuracy", 0)), int(race.get("engineering", 0))
			],
		})
	for cid in class_ids:
		var cls: Dictionary = GameServices.content.get_class_def(cid)
		var mark := ">" if cid == selected_class else " "
		menu_actions.append({
			"id": "class:%s" % cid,
			"label": "%s Class: %s  +HP%d Ranged%d Stealth%d" % [
				mark, cls.get("name", cid), int(cls.get("hp_bonus", 0)),
				int(cls.get("ranged_accuracy", 0)), int(cls.get("stealth", 0))
			],
		})
	menu_actions.append({"id": "compare", "label": "Compare selected bonuses"})
	menu_actions.append({"id": "seed", "label": "Seed: %d (confirm to roll)" % seed_value})
	menu_actions.append({"id": "start", "label": "Begin in Brassharbor"})
	menu_actions.append({"id": "back", "label": "Back"})
	_rebuild_menu()


func _start_game() -> void:
	if not GameServices.start_new_game(seed_value, selected_race, selected_class):
		status_label.text = "Failed to start: content errors"
		return
	_bind_sim_signals()
	mode = Mode.PLAY
	menu_panel.visible = false
	_refresh_world()
	_refresh_status()
	_on_log("Brassharbor awaits. Visit merchants, craft, then take the stairs.")


func _bind_sim_signals() -> void:
	var sim := GameServices.sim
	if sim.log_message.is_connected(_on_log):
		sim.log_message.disconnect(_on_log)
	sim.log_message.connect(_on_log)
	sequencer.clear()
	sequencer.bind_sim(sim)
	_apply_ui_settings()


func _rebuild_menu() -> void:
	menu_list.clear()
	for action in menu_actions:
		menu_list.add_item(str(action["label"]))
	if menu_list.item_count > 0:
		menu_list.select(0)
		menu_list.ensure_current_is_visible()
	menu_list.grab_focus()


func _selected_action() -> Dictionary:
	if menu_list.get_selected_items().is_empty() or menu_actions.is_empty():
		return {}
	var idx: int = menu_list.get_selected_items()[0]
	if idx < 0 or idx >= menu_actions.size():
		return {}
	return menu_actions[idx]


func _on_command(action: String, payload: Dictionary) -> void:
	match mode:
		Mode.TITLE, Mode.CREATE, Mode.PAUSE, Mode.INVENTORY, Mode.CRAFT, Mode.SKILLS, Mode.MERCHANT, Mode.STORAGE, Mode.HEALER, Mode.DEAD, Mode.CHARACTER:
			_handle_menu_command(action, payload)
		Mode.PLAY:
			_handle_play_command(action, payload)
		Mode.RANGED:
			_handle_ranged_command(action, payload)


func _handle_menu_command(action: String, _payload: Dictionary) -> void:
	if action == "move":
		if menu_list.item_count == 0:
			return
		var idx := menu_list.get_selected_items()[0] if menu_list.get_selected_items().size() > 0 else 0
		var dx: int = int(_payload.get("dx", 0))
		var dy: int = int(_payload.get("dy", 0))
		if dy < 0 or dx < 0:
			idx = maxi(0, idx - 1)
		elif dy > 0 or dx > 0:
			idx = mini(menu_list.item_count - 1, idx + 1)
		menu_list.select(idx)
		menu_list.ensure_current_is_visible()
		return
	if action == "cancel":
		if mode in [Mode.PAUSE, Mode.INVENTORY, Mode.CRAFT, Mode.SKILLS, Mode.MERCHANT, Mode.STORAGE, Mode.HEALER, Mode.CHARACTER]:
			if pending_skill != "":
				pending_skill = ""
				_open_skills()
				return
			mode = Mode.PLAY
			menu_panel.visible = false
			_refresh_prompts()
		elif mode == Mode.CREATE:
			_show_title()
		return
	if action != "confirm":
		return
	var chosen := _selected_action()
	if chosen.is_empty():
		return
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
						_bind_sim_signals()
						mode = Mode.PLAY
						menu_panel.visible = false
						if result.get("migrated", false):
							_on_log(str(result.get("message", "Save migrated.")))
						_refresh_world()
						_refresh_status()
						_on_log("Save loaded.")
					else:
						status_label.text = str(result.get("message", result.get("reason", "Load failed")))
				"quit":
					get_tree().quit()
		Mode.CREATE:
			if id.begins_with("race:"):
				selected_race = id.substr(5)
				_show_create()
			elif id.begins_with("class:"):
				selected_class = id.substr(6)
				_show_create()
			elif id == "compare":
				_show_compare()
			elif id == "seed":
				seed_value = randi_range(1, 999999)
				_show_create()
			elif id == "start":
				_start_game()
			elif id == "back":
				_show_title()
		Mode.PAUSE:
			match id:
				"resume":
					mode = Mode.PLAY
					menu_panel.visible = false
				"save":
					status_label.text = "Saved" if GameServices.save_game() else "Save failed"
				"anim":
					var cur := str(GameServices.sim.settings.get("anim_speed", "normal"))
					GameServices.sim.settings["anim_speed"] = "fast" if cur == "normal" else "normal"
					_apply_ui_settings()
					_open_pause()
				"text":
					var ts := int(GameServices.sim.settings.get("text_size", 16))
					GameServices.sim.settings["text_size"] = 14 if ts >= 18 else ts + 2
					_apply_ui_settings()
					_open_pause()
				"title":
					_show_title()
		Mode.INVENTORY:
			_activate_inventory(id)
		Mode.CRAFT:
			if id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
			elif id.begins_with("recipe:"):
				GameServices.sim.craft_recipe(id.substr(7), craft_station)
				_refresh_status()
				_open_craft(craft_station)
		Mode.SKILLS:
			if id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
			elif id.begins_with("confirm:"):
				GameServices.sim.raise_skill(id.substr(8))
				pending_skill = ""
				_open_skills()
				_refresh_status()
			elif id.begins_with("skill:"):
				pending_skill = id.substr(6)
				_open_skills()
		Mode.MERCHANT:
			_activate_merchant(id)
		Mode.STORAGE:
			if id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
			elif id.begins_with("dep:"):
				GameServices.sim.storage_deposit(id.substr(4))
				_open_storage()
			elif id.begins_with("wd:"):
				GameServices.sim.storage_withdraw(int(id.substr(3)))
				_open_storage()
		Mode.HEALER:
			if id == "heal":
				GameServices.sim.heal_at_service()
				_refresh_status()
			mode = Mode.PLAY
			menu_panel.visible = false
		Mode.CHARACTER:
			if id == "skills":
				_open_skills()
			elif id == "close":
				mode = Mode.PLAY
				menu_panel.visible = false
		Mode.DEAD:
			if id == "restart":
				_show_create()
			elif id == "title":
				_show_title()


func _show_compare() -> void:
	var race := GameServices.content.get_race(selected_race)
	var cls := GameServices.content.get_class_def(selected_class)
	menu_title.text = "Compare"
	menu_actions = [
		{"id": "noop", "label": "%s / %s" % [race.get("name"), cls.get("name")]},
		{"id": "noop", "label": "HP %d  Def %d  Eng %d" % [
			int(race.get("base_hp", 0)) + int(cls.get("hp_bonus", 0)),
			int(race.get("defense", 0)) + int(cls.get("defense", 0)),
			int(race.get("engineering", 0)) + int(cls.get("engineering", 0)),
		]},
		{"id": "noop", "label": str(race.get("description", ""))},
		{"id": "noop", "label": str(cls.get("description", ""))},
		{"id": "back_create", "label": "Back"},
	]
	# reuse create mode activation via temporary
	mode = Mode.CREATE
	_rebuild_menu()
	# Map back
	menu_actions[menu_actions.size() - 1]["id"] = "noop"
	menu_actions.append({"id": "start", "label": "Begin Descent Prep"})
	menu_actions.append({"id": "back", "label": "Back to list"})
	_rebuild_menu()


func _activate_inventory(id: String) -> void:
	var player := GameServices.sim.get_player()
	if id == "close":
		mode = Mode.PLAY
		menu_panel.visible = false
	elif id.begins_with("equip:"):
		var iid := id.substr(6)
		for entry in player.inventory:
			var item: SimItem = entry
			if item.instance_id == iid:
				InventorySystem.equip(player, item)
				break
		_open_inventory()
		_refresh_status()
	elif id.begins_with("use:"):
		var iid := id.substr(4)
		for entry in player.inventory:
			var item: SimItem = entry
			if item.instance_id == iid:
				InventorySystem.use_item(player, item)
				break
		_open_inventory()
		_refresh_status()
	elif id.begins_with("drop:"):
		InventorySystem.drop_item(player, GameServices.sim.world, id.substr(5))
		_open_inventory()
	elif id.begins_with("split:"):
		InventorySystem.split_stack(player, id.substr(6), 1, "split_%d" % Time.get_ticks_msec())
		_open_inventory()
	elif id.begins_with("quick:"):
		var iid := id.substr(6)
		for entry in player.inventory:
			var item: SimItem = entry
			if item.instance_id == iid:
				InventorySystem.set_quick_slot(player, 0, item.def_id)
				status_label.text = "Quick slot: %s" % item.name
				break
		_open_inventory()
	elif id.begins_with("filter:"):
		inv_filter = id.substr(7)
		_open_inventory()
	elif id.begins_with("sort:"):
		inv_sort = id.substr(5)
		InventorySystem.sort_inventory(player, inv_sort)
		_open_inventory()
	elif id.begins_with("unequip:"):
		InventorySystem.unequip(player, id.substr(8))
		_open_inventory()
		_refresh_status()


func _activate_merchant(id: String) -> void:
	if id == "close":
		mode = Mode.PLAY
		menu_panel.visible = false
	elif id.begins_with("buy:"):
		GameServices.sim.buy_from_merchant(active_merchant, id.substr(4))
		_open_merchant(active_merchant)
		_refresh_status()
	elif id.begins_with("sell:"):
		GameServices.sim.sell_to_merchant(active_merchant, id.substr(5))
		_open_merchant(active_merchant)
		_refresh_status()


func _handle_play_command(action: String, payload: Dictionary) -> void:
	if GameServices.sim.player_dead:
		_show_dead()
		return
	if GameServices.sim.awaiting_visual and action in ["move", "confirm", "interact", "quick_item", "ranged_mode"]:
		return
	match action:
		"move":
			GameServices.sim.try_player_move(int(payload.get("dx", 0)), int(payload.get("dy", 0)))
			_after_sim()
		"confirm", "interact":
			var result := GameServices.sim.interact()
			if result.get("station", "") != "":
				_open_craft(str(result["station"]))
			elif result.has("merchant"):
				_open_merchant(str(result["merchant"]))
			elif result.get("healer", false):
				_open_healer()
			elif result.get("storage", false):
				_open_storage()
			_after_sim()
		"inventory":
			_open_inventory()
		"character":
			_open_character()
		"pause":
			_open_pause()
		"quick_item":
			GameServices.sim.use_quick_item(0)
			_after_sim()
		"ranged_mode":
			_enter_ranged()
		"inspect":
			_show_context_prompt()
		"camera_peek":
			if world_view and world_view.has_method("peek_camera"):
				world_view.peek_camera(float(payload.get("dx", 0.0)), float(payload.get("dy", 0.0)))
		"cycle_next":
			inv_filter = "all"
		"craft_slot":
			pass


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
		var player := GameServices.sim.get_player()
		if player.weapon_range() <= 0:
			for entry in player.inventory:
				var item: SimItem = entry
				if item.equip_slot == "ranged":
					InventorySystem.equip(player, item)
					break
		GameServices.sim.try_player_ranged(ranged_targets[ranged_index])
		mode = Mode.PLAY
		_after_sim()


func _enter_ranged() -> void:
	ranged_targets.clear()
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
	menu_title.text = "Inventory / Equipment"
	menu_actions.clear()
	var player := GameServices.sim.get_player()
	menu_actions.append({"id": "noop", "label": "Weight %.1f / %.1f | Gold %d" % [player.carry_weight(), player.encumbrance_cap, player.gold]})
	for slot in SimItem.EQUIP_SLOTS:
		var eq: SimItem = player.equipment.get(slot)
		if eq != null:
			menu_actions.append({"id": "unequip:%s" % slot, "label": "[%s] %s (unequip)" % [slot, eq.name]})
		else:
			menu_actions.append({"id": "noop", "label": "[%s] —" % slot})
	menu_actions.append({"id": "filter:all", "label": "Filter: all"})
	menu_actions.append({"id": "filter:weapon", "label": "Filter: weapons"})
	menu_actions.append({"id": "filter:consumable", "label": "Filter: consumables"})
	menu_actions.append({"id": "sort:name", "label": "Sort: name"})
	menu_actions.append({"id": "sort:weight", "label": "Sort: weight"})
	menu_actions.append({"id": "sort:value", "label": "Sort: value"})
	var items := InventorySystem.filter_inventory(player, inv_filter if inv_filter != "weapon" else "weapon")
	if inv_filter == "weapon":
		items = []
		for entry in player.inventory:
			var it: SimItem = entry
			if it.category in ["weapon", "firearm"]:
				items.append(it)
	elif inv_filter != "all":
		items = InventorySystem.filter_inventory(player, inv_filter)
	else:
		items = player.inventory.duplicate()
	for entry in items:
		var item: SimItem = entry
		var cmp := ""
		var equipped: SimItem = player.equipment.get(item.equip_slot)
		if item.equip_slot != "none" and equipped != null:
			cmp = " | vs eq: " + item.compare_summary(equipped)
		var label := "%s x%d (%.1f) %s" % [item.name, item.stack, item.total_weight(), item.description]
		menu_actions.append({"id": "noop", "label": label + cmp})
		if item.equip_slot != "none":
			menu_actions.append({"id": "equip:%s" % item.instance_id, "label": "  Equip"})
		if item.use_effect != "":
			menu_actions.append({"id": "use:%s" % item.instance_id, "label": "  Use"})
		menu_actions.append({"id": "quick:%s" % item.instance_id, "label": "  Set quick"})
		if item.stack > 1:
			menu_actions.append({"id": "split:%s" % item.instance_id, "label": "  Split 1"})
		menu_actions.append({"id": "drop:%s" % item.instance_id, "label": "  Drop"})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_craft(station: String) -> void:
	craft_station = station
	mode = Mode.CRAFT
	menu_panel.visible = true
	menu_title.text = "Crafting: %s" % station
	menu_actions.clear()
	var player := GameServices.sim.get_player()
	for recipe in CraftingSystem.recipes_for_station(GameServices.content, station):
		var rid := str(recipe.get("id", ""))
		var blocked := CraftingSystem.explain_blocked(player, recipe)
		var label := str(recipe.get("name", rid))
		if blocked != "":
			menu_actions.append({"id": "noop", "label": "%s — %s" % [label, blocked]})
		else:
			menu_actions.append({"id": "recipe:%s" % rid, "label": "Craft %s" % label})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_skills() -> void:
	mode = Mode.SKILLS
	menu_panel.visible = true
	menu_title.text = "Skills (SP %d)" % GameServices.sim.get_player().skill_points
	menu_actions.clear()
	var player := GameServices.sim.get_player()
	var affinities := GameServices.sim.affinities_for_player()
	if pending_skill != "":
		var def := GameServices.content.get_skill(pending_skill)
		var check := SkillSystem.can_raise(player, def, GameServices.content, affinities)
		menu_actions.append({"id": "noop", "label": "Confirm spend on %s?" % def.get("name", pending_skill)})
		if check.get("ok", false):
			menu_actions.append({"id": "confirm:%s" % pending_skill, "label": "Confirm (%d SP)" % int(check["cost"])})
		else:
			menu_actions.append({"id": "noop", "label": SkillSystem.explain_unavailable(check)})
		menu_actions.append({"id": "skill:", "label": "Cancel"})
		pending_skill = pending_skill if check.get("ok", false) else pending_skill
		_rebuild_menu()
		return
	for sid in GameServices.content.skills.keys():
		var def: Dictionary = GameServices.content.skills[sid]
		var rank := SkillSystem.rank(player, sid)
		var check := SkillSystem.can_raise(player, def, GameServices.content, affinities)
		var cost := SkillSystem.next_cost(player, def, affinities)
		var label := "%s rank %d/%d" % [def.get("name", sid), rank, int(def.get("max_rank", 5))]
		if check.get("ok", false):
			menu_actions.append({"id": "skill:%s" % sid, "label": "%s — raise (%d SP)" % [label, cost]})
		else:
			menu_actions.append({"id": "noop", "label": "%s — %s" % [label, SkillSystem.explain_unavailable(check)]})
		for ability in def.get("abilities", []):
			var aid := str(ability.get("id", ""))
			var unlocked := aid in player.unlocked_abilities
			menu_actions.append({"id": "noop", "label": "  %s %s" % ["[ON]" if unlocked else "[ ]", ability.get("name", aid)]})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_merchant(merchant_id: String) -> void:
	active_merchant = merchant_id
	mode = Mode.MERCHANT
	menu_panel.visible = true
	var state: Dictionary = GameServices.sim.merchants.get(merchant_id, {})
	var def: Dictionary = state.get("def", GameServices.content.get_merchant(merchant_id))
	menu_title.text = "%s (Gold %d)" % [def.get("name", merchant_id), GameServices.sim.get_player().gold]
	menu_actions.clear()
	var stock: Dictionary = state.get("stock", {})
	for item_id in stock.keys():
		var count := int(stock[item_id])
		if count <= 0:
			continue
		var idef := GameServices.content.get_item(str(item_id))
		var price := EconomySystem.buy_price(idef, def, GameServices.sim.get_player())
		menu_actions.append({"id": "buy:%s" % item_id, "label": "Buy %s x%d (%d brass)" % [idef.get("name", item_id), count, price]})
	for entry in GameServices.sim.get_player().inventory:
		var item: SimItem = entry
		var price := EconomySystem.sell_price(item, def, GameServices.sim.get_player())
		menu_actions.append({"id": "sell:%s" % item.instance_id, "label": "Sell %s (%d)" % [item.name, price]})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_storage() -> void:
	mode = Mode.STORAGE
	menu_panel.visible = true
	menu_title.text = "Storage Locker"
	menu_actions.clear()
	for i in range(GameServices.sim.storage.size()):
		var item: SimItem = GameServices.sim.storage[i]
		menu_actions.append({"id": "wd:%d" % i, "label": "Withdraw %s" % item.name})
	for entry in GameServices.sim.get_player().inventory:
		var item: SimItem = entry
		menu_actions.append({"id": "dep:%s" % item.instance_id, "label": "Deposit %s" % item.name})
	menu_actions.append({"id": "close", "label": "Close"})
	_rebuild_menu()


func _open_healer() -> void:
	mode = Mode.HEALER
	menu_panel.visible = true
	menu_title.text = "Field Medic"
	menu_actions = [
		{"id": "heal", "label": "Full heal & cleanse (15 brass)"},
		{"id": "close", "label": "Leave"},
	]
	_rebuild_menu()


func _open_character() -> void:
	mode = Mode.CHARACTER
	menu_panel.visible = true
	var p := GameServices.sim.get_player()
	menu_title.text = "Operative Sheet"
	var statuses := ", ".join(p.status_effects.keys()) if not p.status_effects.is_empty() else "none"
	menu_actions = [
		{"id": "noop", "label": "Lv%d XP %d | HP %d/%d | SP %d | Gold %d" % [p.level, p.xp, p.hp, p.max_hp, p.skill_points, p.gold]},
		{"id": "noop", "label": "%s / %s" % [p.race_id, p.class_id]},
		{"id": "noop", "label": "Melee %d/%d  Ranged %d/%d  Def %d" % [p.total_melee_accuracy(), p.total_melee_damage(), p.total_ranged_accuracy(), p.total_ranged_damage(), p.total_defense()]},
		{"id": "noop", "label": "Statuses: %s" % statuses},
		{"id": "skills", "label": "Open Skills"},
		{"id": "close", "label": "Close"},
	]
	_rebuild_menu()


func _open_pause() -> void:
	mode = Mode.PAUSE
	menu_panel.visible = true
	menu_title.text = "Paused"
	var anim := str(GameServices.sim.settings.get("anim_speed", "normal"))
	var ts := int(GameServices.sim.settings.get("text_size", 16))
	menu_actions = [
		{"id": "resume", "label": "Resume"},
		{"id": "save", "label": "Save"},
		{"id": "anim", "label": "Anim speed: %s" % anim},
		{"id": "text", "label": "Text size: %d" % ts},
		{"id": "title", "label": "Abort to Title"},
	]
	_rebuild_menu()


func _show_dead() -> void:
	mode = Mode.DEAD
	menu_panel.visible = true
	menu_title.text = "Fallen"
	menu_actions = [
		{"id": "noop", "label": GameServices.sim.death_summary},
		{"id": "restart", "label": "New Operative"},
		{"id": "title", "label": "Title"},
	]
	_rebuild_menu()
	_refresh_prompts()


func _show_context_prompt() -> void:
	var p := GameServices.sim.get_player()
	var tile := GameServices.sim.world.get_tile(p.pos.x, p.pos.y)
	status_label.text = "Tile %d at %d,%d | Depth %d" % [tile, p.pos.x, p.pos.y, GameServices.sim.depth]


func _after_sim() -> void:
	_refresh_world()
	_refresh_status()
	if GameServices.sim.player_dead:
		_show_dead()


func _refresh_world() -> void:
	if world_view:
		world_view.queue_redraw()
		world_view.center_on_player()


func _refresh_status() -> void:
	var p := GameServices.sim.get_player()
	if p == null:
		return
	var device := GameServices.last_input_device
	var place := "Town" if GameServices.sim.world.is_town else "Depth %d" % GameServices.sim.depth
	status_label.text = "HP %d/%d | %s | Turn %d | SP %d | Lv%d | %s | %s" % [
		p.hp, p.max_hp, place, GameServices.sim.turn_index, p.skill_points, p.level, device,
		"ENC" if p.is_encumbered() else "OK",
	]
	var ranged: SimItem = p.get_ranged_weapon()
	var ammo := p.find_ammo()
	var ammo_n := ammo.stack if ammo else 0
	var status_labels: Array[String] = []
	for key in p.status_effects.keys():
		var sid := str(key)
		if sid.begins_with("abilstat_"):
			continue
		status_labels.append(sid.replace("_", " "))
	var statuses := ", ".join(status_labels) if not status_labels.is_empty() else "-"
	var quick_name := "-"
	if p.quick_item_id != "":
		var qdef: Dictionary = GameServices.content.get_item(p.quick_item_id)
		quick_name = str(qdef.get("name", p.quick_item_id)) if not qdef.is_empty() else p.quick_item_id
	hud_label.text = "Main:%s | Range:%s r%d ammo:%d | Quick:%s | Gold:%d | %s" % [
		p.get_equipped("mainhand").name if p.get_equipped("mainhand") else "-",
		ranged.name if ranged else "-",
		p.weapon_range(),
		ammo_n,
		quick_name,
		p.gold,
		statuses,
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
	if mode != Mode.PLAY:
		prompt_label.text = "Move: navigate | A/Enter confirm | B/Esc back"
		return
	if c:
		prompt_label.text = "Move | A interact | X quick | Y inv | RT ranged | View sheet | Start pause"
	else:
		prompt_label.text = "WASD move | Space interact | Q quick | I inv | F ranged | C sheet | Esc pause"


func _on_log(text: String) -> void:
	log_label.append_text(text + "\n")
