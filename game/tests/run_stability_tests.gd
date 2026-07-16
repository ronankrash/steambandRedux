extends SceneTree
## Stress / state-transition invariant tests.

var failures: int = 0
var passes: int = 0


func _initialize() -> void:
	call_deferred("_run")


func _ok(cond: bool, name: String) -> void:
	if cond:
		passes += 1
		print("PASS: ", name)
	else:
		failures += 1
		print("FAIL: ", name)


func _make() -> GameSim:
	var c := ContentDB.new()
	c.load_all()
	var s := GameSim.new()
	s.content = c
	s.new_game(777, "race_automaton", "class_engineer")
	return s


func _run() -> void:
	_test_panel_cycle_sim_state()
	_test_save_load_cycles()
	_test_hub_expedition_cycles()
	_test_economy_stress()
	_test_craft_full_inventory()
	_test_drop_occupied()
	_test_death_and_adjacent_boss()
	_test_malformed_saves()
	_test_empty_merchant_and_ammo()
	_test_extreme_builds()
	_test_consumable_effects()
	print("Stability tests: %d passed, %d failed" % [passes, failures])
	quit(1 if failures > 0 else 0)


func _test_panel_cycle_sim_state() -> void:
	# Repeated hub/expedition transitions must preserve character economy.
	var sim := _make()
	var gold := sim.get_player().gold
	var ok_cycles := true
	for _i in range(100):
		sim.return_to_town(false)
		if not sim.world.is_town:
			ok_cycles = false
		sim.start_expedition()
		if sim.depth != 1:
			ok_cycles = false
	_ok(ok_cycles, "100 hub/expedition panel-equivalent cycles")
	_ok(sim.get_player().gold == gold, "gold unchanged by travel cycles")


func _test_save_load_cycles() -> void:
	var sim := _make()
	sim.start_expedition()
	sim.get_player().gold = 123
	sim.get_player().skill_ranks["skill_melee"] = 2
	for i in range(10):
		var path := "user://stab_save_%d.json" % i
		_ok(SaveSystem.save_to_file(path, sim), "save %d" % i)
		var loaded := SaveSystem.load_from_file(path, sim.content)
		_ok(loaded.get("ok", false), "load %d" % i)
		sim = loaded["sim"]
		_ok(sim.get_player().gold == 123, "gold persist %d" % i)
		_ok(int(sim.get_player().skill_ranks.get("skill_melee", 0)) == 2, "skill persist %d" % i)


func _test_hub_expedition_cycles() -> void:
	var sim := _make()
	for i in range(20):
		sim.start_expedition()
		_ok(sim.depth == 1 and not sim.world.is_town, "hub->expedition %d" % i)
		sim.return_to_town(i % 2 == 0)
		_ok(sim.world.is_town and sim.depth == 0, "expedition->hub %d" % i)


func _test_economy_stress() -> void:
	var sim := _make()
	var p := sim.get_player()
	p.gold = 10000
	var mid := "merchant_general"
	var stock: Dictionary = sim.merchants[mid]["stock"]
	var buys := 0
	for item_id in stock.keys():
		while int(sim.merchants[mid]["stock"].get(item_id, 0)) > 0 and p.gold > 0:
			var res := sim.buy_from_merchant(mid, str(item_id))
			if not res.get("ok", false):
				break
			buys += 1
			if buys > 50:
				break
		if buys > 50:
			break
	_ok(buys > 0, "stress bought items")
	var sells := 0
	while p.inventory.size() > 0 and sells < 30:
		var item: SimItem = p.inventory[0]
		var res := sim.sell_to_merchant(mid, item.instance_id)
		if not res.get("ok", false):
			break
		sells += 1
	_ok(sells > 0, "stress sold items")
	_ok(p.gold >= 0, "gold non-negative")


func _test_craft_full_inventory() -> void:
	var sim := _make()
	var p := sim.get_player()
	# Fill inventory
	while p.inventory.size() < InventorySystem.MAX_SLOTS:
		var scrap := SimItem.from_definition(sim.content.get_item("item_scrap"), "f_%d" % p.inventory.size())
		if not InventorySystem.add_item(p, scrap):
			break
	p.engineering = 10
	p.skill_ranks["skill_engineering"] = 5
	# Stand on workbench
	for y in range(sim.world.height):
		for x in range(sim.world.width):
			if sim.world.get_tile(x, y) == SimWorld.TILE_WORKBENCH:
				p.pos = GridPos.new(x, y)
	# Ensure ingredients somehow - may need to free a slot
	InventorySystem.remove_item_count(p, "item_scrap", 1)
	var wrench := SimItem.from_definition(sim.content.get_item("item_wrench"), "fw")
	InventorySystem.add_item(p, wrench)
	var scrap2 := SimItem.from_definition(sim.content.get_item("item_scrap"), "fs")
	scrap2.stack = 5
	InventorySystem.add_item(p, scrap2)
	var before_ground := sim.world.ground_items.size()
	var crafted := false
	for rid in sim.content.recipes.keys():
		var recipe: Dictionary = sim.content.recipes[rid]
		if str(recipe.get("station", "")) != "workbench":
			continue
		if CraftingSystem.can_craft(p, recipe):
			var res := sim.craft_recipe(rid)
			crafted = res.get("ok", false)
			if res.get("dropped", false):
				_ok(sim.world.ground_items.size() >= before_ground, "craft overflow dropped safely")
			break
	_ok(crafted, "craft with near-full inventory")


func _test_drop_occupied() -> void:
	var sim := _make()
	var p := sim.get_player()
	# Use non-stacking unique items so instance IDs remain addressable after add.
	var item := SimItem.from_definition(sim.content.get_item("item_wrench"), "drop1")
	item.instance_id = "drop1"
	InventorySystem.add_item(p, item)
	var res := InventorySystem.drop_item(p, sim.world, "drop1")
	_ok(res.get("ok", false), "drop ok")
	var item2 := SimItem.from_definition(sim.content.get_item("item_cutlass"), "drop2")
	item2.instance_id = "drop2"
	InventorySystem.add_item(p, item2)
	var res2 := InventorySystem.drop_item(p, sim.world, "drop2")
	_ok(res2.get("ok", false), "drop stack on occupied tile")
	var k := sim.world.key(p.pos.x, p.pos.y)
	_ok(sim.world.ground_items.get(k, []).size() >= 2, "multiple ground items same tile")


func _test_death_and_adjacent_boss() -> void:
	var sim := _make()
	sim.descend_to(5)
	var player := sim.get_player()
	var boss: SimActor = null
	for entry in sim.actors:
		if (entry as SimActor).is_boss:
			boss = entry
			break
	_ok(boss != null, "boss exists")
	if boss:
		boss.hp = 1
		player.pos = boss.pos.add(1, 0)
		sim.world.set_tile(player.pos.x, player.pos.y, SimWorld.TILE_FLOOR)
		var res := sim.try_player_move(boss.pos.x - player.pos.x, boss.pos.y - player.pos.y)
		_ok(res.get("ok", false), "melee boss")
	player.hp = 0
	player.alive = false
	sim._handle_player_death(player)
	_ok(sim.player_dead and sim.death_summary != "", "death summary set")


func _test_malformed_saves() -> void:
	var sim := _make()
	var bad1 := SaveSystem.deserialize({}, sim.content)
	_ok(not bad1.get("ok", false), "empty save rejected")
	var bad2 := SaveSystem.deserialize({"schema_version": 2}, sim.content)
	_ok(not bad2.get("ok", false), "incomplete v2 rejected")
	var path := "user://missing_should_not_exist_123.json"
	if FileAccess.file_exists(path):
		DirAccess.remove_absolute(path)
	var missing := SaveSystem.load_from_file(path, sim.content)
	_ok(not missing.get("ok", false), "missing save rejected")
	# Save while "panel open" is irrelevant to sim — save mid expedition
	sim.start_expedition()
	_ok(SaveSystem.save_to_file("user://mid_panel.json", sim), "save mid-run")
	# schema migration
	var v1 := SaveSystem.serialize(sim)
	v1["schema_version"] = 1
	v1.erase("depth")
	var mig := SaveSystem.deserialize(v1, sim.content)
	_ok(mig.get("ok", false) and mig.get("migrated", false), "migrate after mid-run serialize")


func _test_empty_merchant_and_ammo() -> void:
	var sim := _make()
	var mid := "merchant_general"
	sim.merchants[mid]["stock"] = {}
	var res := sim.buy_from_merchant(mid, "item_scrap")
	_ok(not res.get("ok", false), "empty merchant buy fails")
	sim.start_expedition()
	var p := sim.get_player()
	# Exhaust ammo
	for entry in p.inventory.duplicate():
		var it: SimItem = entry
		if it.is_ammo:
			p.inventory.erase(it)
	if p.equipment.get("ammo") != null:
		p.equipment["ammo"] = null
	var foe := SimActor.new()
	foe.id = "f"
	foe.pos = p.pos.add(2, 0)
	foe.alive = true
	foe.hp = 5
	sim.actors.append(foe)
	sim.world.set_tile(p.pos.x + 1, p.pos.y, SimWorld.TILE_FLOOR)
	sim.world.set_tile(foe.pos.x, foe.pos.y, SimWorld.TILE_FLOOR)
	# Equip ranged if needed
	for entry in p.inventory:
		if (entry as SimItem).equip_slot == "ranged":
			InventorySystem.equip(p, entry)
			break
	var shot := sim.try_player_ranged("f")
	_ok(not shot.get("ok", false), "ranged fails without ammo")


func _test_extreme_builds() -> void:
	var sim := _make()
	for rid in sim.content.races.keys():
		for cid in ["class_occultist", "class_gunslinger"]:
			var s := GameSim.new()
			s.content = sim.content
			_ok(s.new_game(1, str(rid), cid), "extreme %s/%s" % [rid, cid])
			var p := s.get_player()
			_ok(p.max_hp > 0 and p.hp == p.max_hp, "hp ok %s/%s" % [rid, cid])


func _test_consumable_effects() -> void:
	var sim := _make()
	var p := sim.get_player()
	p.hp = 1
	for effect_id in ["item_healing_salve", "item_machine_oil", "item_field_ration", "item_antidote_vial", "item_combat_stim", "item_smoke_bomb"]:
		var it := SimItem.from_definition(sim.content.get_item(effect_id), "c_" + effect_id)
		InventorySystem.add_item(p, it)
		var res := InventorySystem.use_item(p, it)
		_ok(res.get("ok", false), "use " + effect_id)
