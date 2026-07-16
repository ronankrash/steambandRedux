extends SceneTree
## Headless simulation/integration tests for Brassdeep Phase 2.

var failures: int = 0
var passes: int = 0


func _initialize() -> void:
	call_deferred("_run")


func _run() -> void:
	_test_content_loads()
	_test_deterministic_generation()
	_test_character_modifiers()
	_test_skill_system()
	_test_turn_ordering()
	_test_melee_resolution()
	_test_ranged_los()
	_test_inventory_ops()
	_test_equipment_and_encumbrance()
	_test_affix_generation()
	_test_ammo_consumption()
	_test_status_effects()
	_test_ai_behaviors()
	_test_crafting_stations()
	_test_merchant_economy()
	_test_expedition_connectivity()
	_test_difficulty_scaling()
	_test_save_round_trip_and_migration()
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
	_ok(content.races.size() >= 6, "at least 6 races")
	_ok(content.classes.size() >= 6, "at least 6 classes")
	_ok(content.skills.size() >= 8, "at least 8 skills")
	_ok(content.monsters.size() >= 12, "at least 12 monsters")
	_ok(content.recipes.size() >= 20, "at least 20 recipes")
	_ok(content.merchants.size() >= 3, "at least 3 merchants")
	_ok(content.environments.size() >= 2, "at least 2 environments")
	var abilities := 0
	for sid in content.skills.keys():
		abilities += (content.skills[sid].get("abilities", []) as Array).size()
	_ok(abilities >= 12, "at least 12 skill abilities")


func _test_deterministic_generation() -> void:
	var a := _make_sim()
	var b := _make_sim()
	a.new_game(12345, "race_human", "class_adventurer")
	b.new_game(12345, "race_human", "class_adventurer")
	a.start_expedition()
	b.start_expedition()
	_ok(a.world.width == b.world.width and a.world.height == b.world.height, "same map size")
	var same_tiles := true
	for i in range(a.world.tiles.size()):
		if a.world.tiles[i] != b.world.tiles[i]:
			same_tiles = false
			break
	_ok(same_tiles, "same tiles for same seed")
	_ok(a.get_player().pos.equals(b.get_player().pos), "same player start")
	# Affix generation deterministic
	var rng1 := BrassRng.new()
	rng1.reseed(99)
	var rng2 := BrassRng.new()
	rng2.reseed(99)
	var i1 := ItemGenerator.create_item(rng1, a.content, "item_brass_plate", "x1", 100)
	var i2 := ItemGenerator.create_item(rng2, b.content, "item_brass_plate", "x2", 100)
	_ok(i1.affix_id == i2.affix_id and i1.defense == i2.defense, "deterministic affix roll")


func _test_character_modifiers() -> void:
	var sim := _make_sim()
	sim.new_game(1, "race_automaton", "class_engineer")
	var p := sim.get_player()
	var race := sim.content.get_race("race_automaton")
	var cls := sim.content.get_class_def("class_engineer")
	_ok(p.max_hp == int(race.get("base_hp", 0)) + int(cls.get("hp_bonus", 0)), "hp from race+class")
	_ok(p.engineering == int(race.get("engineering", 0)) + int(cls.get("engineering", 0)), "engineering applied")
	_ok(p.race_id == "race_automaton" and p.class_id == "class_engineer", "ids stored")
	# 36 combinations sanity: all load
	var combo_ok := true
	var combo_count := 0
	for rid in sim.content.races.keys():
		for cid in sim.content.classes.keys():
			var s2 := GameSim.new()
			s2.content = sim.content
			if not s2.new_game(2, str(rid), str(cid)):
				combo_ok = false
			combo_count += 1
	_ok(combo_ok and combo_count >= 36, "all 36 race/class combos start")


func _test_skill_system() -> void:
	var sim := _make_sim()
	sim.new_game(3, "race_human", "class_adventurer")
	var p := sim.get_player()
	p.skill_points = 10
	var def := sim.content.get_skill("skill_melee")
	var cost1 := SkillSystem.next_cost(p, def, {})
	_ok(cost1 > 0, "skill cost positive")
	var before_rank := SkillSystem.rank(p, "skill_melee")
	var r1 := SkillSystem.raise_skill(p, def, sim.content, {})
	_ok(r1.get("ok", false), "raise skill ok")
	_ok(SkillSystem.rank(p, "skill_melee") == before_rank + 1, "rank increased by 1")
	var cost2 := SkillSystem.next_cost(p, def, {})
	_ok(cost2 > cost1, "cost increases with rank")
	# Affinity cheaper
	var cost_aff := SkillSystem.next_cost(p, def, {"skill_melee": 0.5})
	_ok(cost_aff <= cost2, "affinity reduces cost")
	# Persist via save
	p.skill_points = 5
	SkillSystem.raise_skill(p, def, sim.content, {})
	var path := "user://brassdeep_skill_save.json"
	SaveSystem.save_to_file(path, sim)
	var loaded := SaveSystem.load_from_file(path, sim.content)
	_ok(loaded.get("ok", false), "skill save load ok")
	var p2: SimActor = loaded["sim"].get_player()
	_ok(SkillSystem.rank(p2, "skill_melee") == SkillSystem.rank(p, "skill_melee"), "skill ranks persist")


func _test_turn_ordering() -> void:
	var sim := _make_sim()
	sim.new_game(7, "race_human", "class_adventurer")
	sim.start_expedition()
	var before := sim.turn_index
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


func _test_inventory_ops() -> void:
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
	var split := InventorySystem.split_stack(actor, actor.inventory[0].instance_id, 2, "split1")
	_ok(split.get("ok", false), "split stack")
	InventorySystem.sort_inventory(actor, "name")
	_ok(actor.inventory.size() >= 2, "inventory after sort")


func _test_equipment_and_encumbrance() -> void:
	var content := ContentDB.new()
	content.load_all()
	var actor := SimActor.new()
	actor.melee_damage = 2
	actor.defense = 1
	actor.encumbrance_cap = 40.0
	var weapon := SimItem.from_definition(content.get_item("item_wrench"), "w1")
	var armor := SimItem.from_definition(content.get_item("item_brass_plate"), "a1")
	InventorySystem.add_item(actor, weapon)
	InventorySystem.add_item(actor, armor)
	InventorySystem.equip(actor, weapon)
	InventorySystem.equip(actor, armor)
	_ok(actor.equipment.get("mainhand") != null, "mainhand equipped")
	_ok(actor.equipment.get("body") != null, "body equipped")
	_ok(actor.total_melee_damage() == 2 + weapon.damage, "weapon damage applied")
	_ok(actor.total_defense() == 1 + armor.defense, "armor defense applied")
	_ok(actor.carry_weight() > 0.0, "encumbrance weight tracked")


func _test_affix_generation() -> void:
	var content := ContentDB.new()
	content.load_all()
	var affix := content.get_affix("affix_reinforced")
	var plate := SimItem.from_definition(content.get_item("item_brass_plate"), "p1", affix)
	_ok(plate.affix_id == "affix_reinforced", "affix id set")
	_ok(plate.defense > int(content.get_item("item_brass_plate").get("defense", 0)), "affix boosts defense")


func _test_ammo_consumption() -> void:
	var sim := _make_sim()
	sim.new_game(5, "race_human", "class_gunslinger")
	sim.start_expedition()
	var p := sim.get_player()
	# Ensure ranged + ammo
	for entry in p.inventory:
		var item: SimItem = entry
		if item.equip_slot == "ranged":
			InventorySystem.equip(p, item)
		if item.is_ammo:
			InventorySystem.equip(p, item)
	var ammo_before := 0
	var ammo_item := p.find_ammo()
	if ammo_item:
		ammo_before = ammo_item.stack
	# Spawn a dummy foe adjacent with LOS
	var foe := SimActor.new()
	foe.id = "foe1"
	foe.kind = "monster"
	foe.name = "Dummy"
	foe.pos = p.pos.add(2, 0)
	foe.hp = 20
	foe.max_hp = 20
	foe.alive = true
	sim.actors.append(foe)
	# Clear walls between if needed
	sim.world.set_tile(p.pos.x + 1, p.pos.y, SimWorld.TILE_FLOOR)
	sim.world.set_tile(foe.pos.x, foe.pos.y, SimWorld.TILE_FLOOR)
	if p.weapon_range() > 0 and ammo_before > 0:
		var res := sim.try_player_ranged("foe1")
		_ok(res.get("ok", false), "ranged attack ok")
		var ammo_after_item := p.find_ammo()
		var ammo_after := ammo_after_item.stack if ammo_after_item else 0
		_ok(ammo_after == ammo_before - 1 or (ammo_after_item == null and ammo_before == 1), "ammo consumed")
	else:
		_ok(false, "ranged attack ok")
		_ok(false, "ammo consumed")


func _test_status_effects() -> void:
	var actor := SimActor.new()
	actor.hp = 10
	actor.max_hp = 10
	actor.add_status("poison", 2)
	_ok(actor.status_effects.has("poison"), "poison applied")
	actor.tick_statuses()
	_ok(int(actor.status_effects.get("poison", 0)) == 1, "poison ticks down")
	actor.resistances["fear"] = 100
	actor.add_status("fear", 3)
	_ok(not actor.status_effects.has("fear"), "full fear resist blocks")


func _test_ai_behaviors() -> void:
	var sim := _make_sim()
	sim.new_game(8, "race_human", "class_adventurer")
	sim.start_expedition()
	var player := sim.get_player()
	var flee := SimActor.new()
	flee.id = "flee1"
	flee.behavior = "flee"
	flee.pos = player.pos.add(1, 0)
	flee.vision_range = 8
	flee.hear_radius = 8
	flee.alive = true
	var decision := AiController.decide(sim, flee, player)
	_ok(str(decision.get("type", "")) in ["flee", "wait"], "flee AI decides flee/wait")
	var sentry := SimActor.new()
	sentry.id = "s1"
	sentry.behavior = "ranged_sentry"
	sentry.pos = player.pos.add(3, 0)
	sentry.vision_range = 8
	sentry.alive = true
	sim.world.set_tile(player.pos.x + 1, player.pos.y, SimWorld.TILE_FLOOR)
	sim.world.set_tile(player.pos.x + 2, player.pos.y, SimWorld.TILE_FLOOR)
	sim.world.set_tile(sentry.pos.x, sentry.pos.y, SimWorld.TILE_FLOOR)
	var d2 := AiController.decide(sim, sentry, player)
	_ok(str(d2.get("type", "")) in ["ranged", "approach", "kite"], "sentry AI actionable")


func _test_crafting_stations() -> void:
	var sim := _make_sim()
	sim.new_game(9, "race_gnome", "class_engineer")
	var p := sim.get_player()
	# Give ingredients for a workbench recipe
	for id in ["item_wrench", "item_scrap"]:
		if sim.content.items.has(id):
			var it := SimItem.from_definition(sim.content.get_item(id), "c_" + id)
			if id == "item_scrap":
				it.stack = 5
			InventorySystem.add_item(p, it)
	p.engineering = 10
	p.skill_ranks["skill_engineering"] = 3
	p.skill_ranks["skill_smithing"] = 3
	# Move to workbench in town
	for y in range(sim.world.height):
		for x in range(sim.world.width):
			if sim.world.get_tile(x, y) == SimWorld.TILE_WORKBENCH:
				p.pos = GridPos.new(x, y)
	var recipes := CraftingSystem.recipes_for_station(sim.content, "workbench")
	_ok(recipes.size() > 0, "workbench recipes exist")
	var forged := CraftingSystem.recipes_for_station(sim.content, "forge")
	var alchemy := CraftingSystem.recipes_for_station(sim.content, "alchemy")
	_ok(forged.size() > 0 and alchemy.size() > 0, "forge and alchemy recipes exist")
	var crafted_any := false
	for recipe in recipes:
		if CraftingSystem.can_craft(p, recipe):
			var result := sim.craft_recipe(str(recipe.get("id", "")), "workbench")
			crafted_any = result.get("ok", false)
			break
	_ok(crafted_any, "crafted at workbench")


func _test_merchant_economy() -> void:
	var sim := _make_sim()
	sim.new_game(10, "race_human", "class_adventurer")
	var p := sim.get_player()
	p.gold = 200
	var mid := "merchant_general"
	_ok(sim.merchants.has(mid), "general merchant present")
	var stock: Dictionary = sim.merchants[mid]["stock"]
	var bought := false
	for item_id in stock.keys():
		if int(stock[item_id]) > 0:
			var res := sim.buy_from_merchant(mid, str(item_id))
			bought = res.get("ok", false)
			break
	_ok(bought, "bought from merchant")
	# Sell something
	if p.inventory.size() > 0:
		var item: SimItem = p.inventory[0]
		var sold := sim.sell_to_merchant(mid, item.instance_id)
		_ok(sold.get("ok", false), "sold to merchant")
	else:
		_ok(false, "sold to merchant")


func _test_expedition_connectivity() -> void:
	var sim := _make_sim()
	sim.new_game(11, "race_dwarf", "class_officer")
	_ok(sim.world.is_town, "starts in town")
	sim.start_expedition()
	_ok(sim.depth == 1 and not sim.world.is_town, "expedition depth 1")
	var reachable := sim.world.flood_walkable(sim.get_player().pos)
	_ok(reachable > 20, "level connectivity walkable")
	sim.descend_to(2)
	_ok(sim.depth == 2, "descend to 2")
	sim.descend_to(5)
	_ok(sim.depth == 5, "descend to 5")
	var has_boss := false
	for entry in sim.actors:
		var a: SimActor = entry
		if a.is_boss:
			has_boss = true
	_ok(has_boss, "boss present on depth 5")
	sim.return_to_town(true)
	_ok(sim.world.is_town and sim.depth == 0, "extract to town")


func _test_difficulty_scaling() -> void:
	var sim := _make_sim()
	sim.new_game(12, "race_human", "class_adventurer")
	var m1 := sim._create_monster("mon_gear_rat", GridPos.new(1, 1), 1)
	var m4 := sim._create_monster("mon_gear_rat", GridPos.new(1, 1), 4)
	_ok(m4.max_hp > m1.max_hp, "deeper monsters scale tougher")
	_ok(m4.melee_damage >= m1.melee_damage, "deeper monsters scale damage")


func _test_save_round_trip_and_migration() -> void:
	var sim := _make_sim()
	sim.new_game(4242, "race_automaton", "class_engineer")
	sim.start_expedition()
	sim.try_player_move(1, 0)
	var path := "user://brassdeep_test_save.json"
	_ok(SaveSystem.save_to_file(path, sim), "save wrote")
	var loaded := SaveSystem.load_from_file(path, sim.content)
	_ok(loaded.get("ok", false), "load ok")
	if loaded.get("ok", false):
		var sim2: GameSim = loaded["sim"]
		_ok(sim2.seed_value == 4242, "seed preserved")
		_ok(sim2.get_player().race_id == "race_automaton", "race preserved")
		_ok(sim2.depth == sim.depth, "depth preserved")
		_ok(SaveSystem.SCHEMA_VERSION == 2, "schema v2")
	var bad := SaveSystem.deserialize({"schema_version": 999}, sim.content)
	_ok(bad.get("ok", false) == false, "invalid schema rejected")
	# v1 migration
	var v1 := {
		"schema_version": 1,
		"seed": 7,
		"turn": 3,
		"log": ["hi"],
		"rng_state": 7,
		"world": sim.world.to_dict(),
		"actors": [sim.get_player().to_dict()],
		"next_instance": 3,
		"player_id": "player",
		"dead": false,
		"death_summary": "",
	}
	var migrated := SaveSystem.deserialize(v1, sim.content)
	_ok(migrated.get("ok", false) and migrated.get("migrated", false), "v1 migrates to v2")


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
