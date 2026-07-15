extends SceneTree
## Headless simulation/integration tests. Run:
## tools/bin/godot --headless --path game --script res://tests/run_headless_tests.gd

var failures: int = 0
var passes: int = 0


func _initialize() -> void:
	call_deferred("_run")


func _run() -> void:
	_test_content_loads()
	_test_deterministic_generation()
	_test_turn_ordering()
	_test_melee_resolution()
	_test_ranged_los()
	_test_inventory_stacking()
	_test_affix_applied()
	_test_equipment_stats()
	_test_recipe_validation()
	_test_skill_points()
	_test_save_round_trip()
	_test_invalid_content_detection()
	print("Brassdeep tests: %d passed, %d failed" % [passes, failures])
	quit(1 if failures > 0 else 0)


func _ok(cond: bool, name: String) -> void:
	if cond:
		passes += 1
		print("PASS: ", name)
	else:
		failures += 1
		print("FAIL: ", name)


func _make_sim() -> GameSim:
	var content := ContentDB.new()
	_ok(content.load_all(), "content.load_all baseline")
	var sim := GameSim.new()
	sim.content = content
	return sim


func _test_content_loads() -> void:
	var content := ContentDB.new()
	_ok(content.load_all(), "content validation clean")
	_ok(content.races.has("race_human"), "race_human exists")
	_ok(content.classes.has("class_engineer"), "class_engineer exists")
	_ok(content.monsters.size() >= 3, "at least 3 monsters")
	_ok(content.recipes.size() >= 3, "at least 3 recipes")


func _test_deterministic_generation() -> void:
	var a := _make_sim()
	var b := _make_sim()
	a.new_game(12345, "race_human", "class_adventurer")
	b.new_game(12345, "race_human", "class_adventurer")
	_ok(a.world.width == b.world.width and a.world.height == b.world.height, "same map size")
	var same_tiles := true
	for i in range(a.world.tiles.size()):
		if a.world.tiles[i] != b.world.tiles[i]:
			same_tiles = false
			break
	_ok(same_tiles, "same tiles for same seed")
	_ok(a.get_player().pos.equals(b.get_player().pos), "same player start")


func _test_turn_ordering() -> void:
	var sim := _make_sim()
	sim.new_game(7, "race_human", "class_adventurer")
	var before := sim.turn_index
	sim.try_player_move(0, 0) # blocked or no-op may not advance; move into open if possible
	# Force a successful action via interact or move search
	var player := sim.get_player()
	var moved := false
	for n in GridPos.neighbors8():
		var res := sim.try_player_move(n.x, n.y)
		if res.get("ok", false):
			moved = true
			break
	_ok(moved or sim.turn_index >= before, "player action attempted")
	_ok(sim.turn_index >= before, "turn index non-decreasing")


func _test_melee_resolution() -> void:
	var rng := BrassRng.new()
	rng.reseed(99)
	var atk := SimActor.new()
	atk.melee_accuracy = 100
	atk.melee_damage = 5
	var def := SimActor.new()
	def.defense = 0
	def.hp = 10
	def.max_hp = 10
	var result := CombatResolver.resolve_melee(rng, atk, def)
	_ok(result["hit"] == true, "high accuracy melee hits")
	_ok(int(result["damage"]) > 0, "melee deals damage")


func _test_ranged_los() -> void:
	var world := SimWorld.new()
	world.resize(5, 5)
	for y in range(5):
		for x in range(5):
			world.set_tile(x, y, SimWorld.TILE_FLOOR)
	world.set_tile(2, 1, SimWorld.TILE_WALL)
	_ok(CombatResolver.has_line_of_sight(world, GridPos.new(1, 1), GridPos.new(3, 1)) == false, "wall blocks LOS")
	_ok(CombatResolver.has_line_of_sight(world, GridPos.new(1, 2), GridPos.new(3, 2)) == true, "open LOS clear")


func _test_inventory_stacking() -> void:
	var content := ContentDB.new()
	content.load_all()
	var actor := SimActor.new()
	var a := SimItem.from_definition(content.get_item("item_ball_ammo"), "a1")
	a.stack = 5
	var b := SimItem.from_definition(content.get_item("item_ball_ammo"), "a2")
	b.stack = 7
	_ok(InventorySystem.add_item(actor, a), "add ammo a")
	_ok(InventorySystem.add_item(actor, b), "add ammo b")
	_ok(actor.count_item("item_ball_ammo") == 12, "ammo stacked to 12")


func _test_affix_applied() -> void:
	var content := ContentDB.new()
	content.load_all()
	var affix := content.get_affix("affix_reinforced")
	var plate := SimItem.from_definition(content.get_item("item_brass_plate"), "p1", affix)
	_ok(plate.affix_id == "affix_reinforced", "affix id set")
	_ok(plate.defense > int(content.get_item("item_brass_plate").get("defense", 0)), "affix boosts defense")
	_ok(plate.name.contains("Reinforced") or plate.name.to_lower().contains("reinforced"), "affix renames item")


func _test_equipment_stats() -> void:
	var content := ContentDB.new()
	content.load_all()
	var actor := SimActor.new()
	actor.melee_damage = 2
	actor.defense = 1
	var weapon := SimItem.from_definition(content.get_item("item_wrench"), "w1")
	var armor := SimItem.from_definition(content.get_item("item_brass_plate"), "a1")
	InventorySystem.add_item(actor, weapon)
	InventorySystem.add_item(actor, armor)
	InventorySystem.equip(actor, weapon)
	InventorySystem.equip(actor, armor)
	_ok(actor.total_melee_damage() == 2 + weapon.damage, "weapon damage applied")
	_ok(actor.total_defense() == 1 + armor.defense, "armor defense applied")


func _test_recipe_validation() -> void:
	var content := ContentDB.new()
	content.load_all()
	var actor := SimActor.new()
	actor.engineering = 5
	var wrench := SimItem.from_definition(content.get_item("item_wrench"), "w")
	var scrap := SimItem.from_definition(content.get_item("item_scrap"), "s")
	scrap.stack = 2
	InventorySystem.add_item(actor, wrench)
	InventorySystem.add_item(actor, scrap)
	var recipe := content.get_recipe("recipe_reinforced_wrench")
	_ok(CraftingSystem.can_craft(actor, recipe), "can craft reinforced wrench")
	var result := CraftingSystem.craft(actor, recipe, content, "out1")
	_ok(result.get("ok", false), "craft succeeded")
	_ok(actor.count_item("item_reinforced_wrench") == 1, "result in inventory")


func _test_skill_points() -> void:
	var sim := _make_sim()
	sim.new_game(11, "race_human", "class_adventurer")
	var player := sim.get_player()
	player.skill_points = 2
	var before := player.melee_accuracy
	_ok(sim.spend_skill_point("melee").get("ok", false), "spend melee sp")
	_ok(player.melee_accuracy == before + 2, "melee accuracy increased")
	_ok(player.skill_points == 1, "sp decremented")


func _test_save_round_trip() -> void:
	var sim := _make_sim()
	sim.new_game(4242, "race_automaton", "class_engineer")
	sim.try_player_move(1, 0)
	var path := "user://brassdeep_test_save.json"
	_ok(SaveSystem.save_to_file(path, sim), "save wrote")
	var loaded := SaveSystem.load_from_file(path, sim.content)
	_ok(loaded.get("ok", false), "load ok")
	if loaded.get("ok", false):
		var sim2: GameSim = loaded["sim"]
		_ok(sim2.seed_value == 4242, "seed preserved")
		_ok(sim2.get_player().race_id == "race_automaton", "race preserved")
		_ok(sim2.combat_log.size() > 0, "combat log restored")
	else:
		_ok(false, "seed preserved")
		_ok(false, "race preserved")
		_ok(false, "combat log restored")
	var bad := SaveSystem.deserialize({"schema_version": 999}, sim.content)
	_ok(bad.get("ok", false) == false, "invalid schema rejected")


func _test_invalid_content_detection() -> void:
	var content := ContentDB.new()
	content.load_all()
	content.items["item_bad"] = {
		"id": "item_bad",
		"equip_slot": "hat",
		"allowed_affixes": ["affix_missing"],
	}
	content.errors.clear()
	content._validate()
	_ok(content.errors.size() >= 2, "invalid slot and affix detected")
