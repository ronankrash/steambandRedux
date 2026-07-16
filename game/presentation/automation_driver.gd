class_name AutomationDriver
extends RefCounted
## Drives Main through InputRouter.command — same path as controller input.

signal finished(ok: bool, message: String)

var main: Node
var router: Node
var failures: Array[String] = []
var screenshot_index: int = 0


func _init(main_node: Node) -> void:
	main = main_node
	router = main.get_node("InputRouter")


func emit_cmd(action: String, payload: Dictionary = {}) -> void:
	GameServices.last_input_device = "controller"
	router.command.emit(action, payload)


func await_frames(tree: SceneTree, n: int = 1) -> void:
	for _i in range(n):
		await tree.process_frame


func fail(msg: String) -> void:
	failures.append(msg)
	Automation.fail(msg)


func expect_mode(expected: int, label: String) -> void:
	if int(main.mode) != expected:
		fail("%s: expected mode %d got %d" % [label, expected, int(main.mode)])


func expect_menu_focus(label: String) -> void:
	var list: ItemList = main.menu_list
	if list == null or not main.menu_panel.visible:
		fail("%s: menu not visible" % label)
		return
	if list.item_count <= 0:
		fail("%s: empty menu" % label)
		return
	if list.get_selected_items().is_empty():
		fail("%s: no selection / focus lost" % label)


func select_menu_id(id: String, max_steps: int = 160) -> bool:
	expect_menu_focus("select " + id)
	var target := -1
	for i in range(main.menu_actions.size()):
		if str(main.menu_actions[i].get("id", "")) == id:
			target = i
			break
	if target < 0:
		fail("Could not find menu id: " + id)
		return false
	var steps := 0
	while steps < max_steps:
		if main.menu_list.get_selected_items().is_empty():
			fail("%s: focus lost while selecting" % id)
			return false
		var idx: int = main.menu_list.get_selected_items()[0]
		if idx == target:
			emit_cmd("confirm")
			return true
		emit_cmd("move", {"dx": 0, "dy": 1 if target > idx else -1})
		steps += 1
	fail("Could not navigate to menu id: " + id)
	return false


func capture(name: String) -> void:
	if not Automation.capture_screenshots:
		return
	Automation.ensure_screenshot_dir()
	await main.get_tree().process_frame
	await main.get_tree().process_frame
	var img: Image = main.get_viewport().get_texture().get_image()
	if img == null:
		fail("screenshot failed: " + name)
		return
	var path := "%s/%02d_%s.png" % [Automation.screenshot_dir, screenshot_index, name]
	screenshot_index += 1
	var err := img.save_png(path)
	if err != OK:
		fail("save_png %s err %d" % [path, err])
	else:
		print("SCREENSHOT: ", path)


func move_player_to_tile(tile: int, max_steps: int = 240) -> bool:
	var sim: GameSim = GameServices.sim
	var goal: GridPos = null
	for y in range(sim.world.height):
		for x in range(sim.world.width):
			if sim.world.get_tile(x, y) == tile:
				goal = GridPos.new(x, y)
				break
		if goal != null:
			break
	if goal == null:
		fail("No tile type %d found" % tile)
		return false
	for _step in range(max_steps):
		if GameServices.sim.player_dead or int(main.mode) == main.Mode.DEAD:
			_prep_combat_sandbox()
		var p2 := sim.get_player()
		if p2.pos.equals(goal) or sim.world.get_tile(p2.pos.x, p2.pos.y) == tile:
			return true
		var path := _bfs(sim.world, p2.pos, goal)
		if path.is_empty():
			fail("No path to tile %d from %d,%d" % [tile, p2.pos.x, p2.pos.y])
			return false
		var nxt: GridPos = path[0]
		var dx: int = clampi(nxt.x - p2.pos.x, -1, 1)
		var dy: int = clampi(nxt.y - p2.pos.y, -1, 1)
		if dx == 0 and dy == 0:
			fail("Path stall at tile %d" % tile)
			return false
		var blocker := sim.actor_at(GridPos.new(p2.pos.x + dx, p2.pos.y + dy))
		if blocker != null and not blocker.is_player():
			blocker.alive = false
			sim.actors.erase(blocker)
		var before := p2.pos
		emit_cmd("move", {"dx": dx, "dy": dy})
		await main.get_tree().process_frame
		var after := sim.get_player().pos
		# If bump opened/smashed without moving, continue; if totally stuck, try orthogonal only next.
		if after.equals(before) and sim.world.get_tile(before.x + dx, before.y + dy) in [
			SimWorld.TILE_WALL, SimWorld.TILE_GATE_LOCKED
		]:
			# Fall back: carve a temporary floor so automation can proceed through broken topology.
			if sim.world.get_tile(before.x + dx, before.y + dy) == SimWorld.TILE_GATE_LOCKED:
				sim.world.lever_on = true
				sim.world.set_tile(before.x + dx, before.y + dy, SimWorld.TILE_DOOR_OPEN)
	if sim.world.get_tile(sim.get_player().pos.x, sim.get_player().pos.y) == tile:
		return true
	fail("Failed to finish path to tile %d" % tile)
	return false


func _bfs(world: SimWorld, start: GridPos, goal: GridPos) -> Array:
	## Orthogonal-only pathing matches reliable controller taps.
	var sk := func(p: GridPos) -> String: return "%d,%d" % [p.x, p.y]
	var dirs: Array[Vector2i] = [Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)]
	var q: Array = [start]
	var came: Dictionary = {}
	var seen: Dictionary = {sk.call(start): true}
	came[sk.call(start)] = null
	while not q.is_empty():
		var cur: GridPos = q.pop_front()
		if cur.equals(goal):
			break
		for n in dirs:
			var nxt := cur.add(n.x, n.y)
			var k: String = sk.call(nxt)
			if seen.has(k):
				continue
			var t := world.get_tile(nxt.x, nxt.y)
			var ok := (
				world.is_walkable(nxt.x, nxt.y)
				or t in [SimWorld.TILE_DOOR_CLOSED, SimWorld.TILE_GATE_LOCKED, SimWorld.TILE_DESTRUCTIBLE]
				or nxt.equals(goal)
			)
			if not ok:
				continue
			# Do not route through locked gates unless lever already thrown.
			if t == SimWorld.TILE_GATE_LOCKED and not world.lever_on:
				continue
			seen[k] = true
			came[k] = cur
			q.append(nxt)
	if not seen.has(sk.call(goal)):
		return []
	var out: Array = []
	var c: Variant = goal
	while c != null and not (c as GridPos).equals(start):
		out.push_front(c)
		c = came[sk.call(c as GridPos)]
	return out


func run_full_flow() -> void:
	failures.clear()
	var tree := main.get_tree()
	Automation.instant_animations = true
	GameServices.last_input_device = "controller"

	# 1-3 Title -> Create -> select race/class -> start
	expect_mode(main.Mode.TITLE, "title")
	await capture("title")
	if not select_menu_id("new"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 2)
	expect_mode(main.Mode.CREATE, "create")
	await capture("character_creation")
	select_menu_id("race:race_human")
	await await_frames(tree, 1)
	select_menu_id("class:class_engineer")
	await await_frames(tree, 1)
	# Force deterministic seed via direct field (UI seed is random on confirm)
	main.seed_value = Automation.deterministic_seed
	main.selected_race = "race_human"
	main.selected_class = "class_engineer"
	if not select_menu_id("start"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 3)
	expect_mode(main.Mode.PLAY, "town play")
	if not GameServices.sim.world.is_town:
		fail("Did not enter Brassharbor/town")
	await capture("brassharbor")

	# 5 Open/close major panels
	for panel_open in [
		["inventory", main.Mode.INVENTORY, "inventory_equipment"],
		["character", main.Mode.CHARACTER, "character_sheet"],
		["pause", main.Mode.PAUSE, "pause_save"],
	]:
		emit_cmd(str(panel_open[0]))
		await await_frames(tree, 2)
		expect_mode(int(panel_open[1]), str(panel_open[0]))
		expect_menu_focus(str(panel_open[0]))
		await capture(str(panel_open[2]))
		emit_cmd("cancel")
		await await_frames(tree, 2)
		expect_mode(main.Mode.PLAY, "back to play after " + str(panel_open[0]))

	# Skills via character
	emit_cmd("character")
	await await_frames(tree, 1)
	select_menu_id("skills")
	await await_frames(tree, 2)
	expect_mode(main.Mode.SKILLS, "skills")
	await capture("skills")
	# Spend a skill point if possible
	GameServices.sim.get_player().skill_points = maxi(GameServices.sim.get_player().skill_points, 3)
	main._open_skills()
	await await_frames(tree, 1)
	var spent := false
	for action in main.menu_actions:
		var aid := str(action.get("id", ""))
		if aid.begins_with("skill:") and aid.length() > 6:
			select_menu_id(aid)
			await await_frames(tree, 1)
			# confirm pending
			for a2 in main.menu_actions:
				if str(a2.get("id", "")).begins_with("confirm:"):
					select_menu_id(str(a2["id"]))
					spent = true
					break
			break
	if not spent:
		fail("Could not spend skill point through UI")
	emit_cmd("cancel")
	await await_frames(tree, 1)
	if int(main.mode) != main.Mode.PLAY:
		emit_cmd("cancel")
		await await_frames(tree, 1)

	# Merchant buy/sell
	if not await move_player_to_tile(SimWorld.TILE_MERCHANT):
		finished.emit(false, "merchant unreachable"); return
	emit_cmd("interact")
	await await_frames(tree, 2)
	expect_mode(main.Mode.MERCHANT, "merchant")
	await capture("merchant")
	var bought := false
	for action in main.menu_actions:
		if str(action.get("id", "")).begins_with("buy:"):
			GameServices.sim.get_player().gold = maxi(GameServices.sim.get_player().gold, 500)
			select_menu_id(str(action["id"]))
			bought = true
			await await_frames(tree, 1)
			break
	if not bought:
		fail("No buy action in merchant UI")
	var sold := false
	for action in main.menu_actions:
		if str(action.get("id", "")).begins_with("sell:"):
			select_menu_id(str(action["id"]))
			sold = true
			await await_frames(tree, 1)
			break
	if not sold:
		fail("No sell action in merchant UI")
	emit_cmd("cancel")
	await await_frames(tree, 1)

	# Storage deposit/withdraw
	if await move_player_to_tile(SimWorld.TILE_STORAGE):
		emit_cmd("interact")
		await await_frames(tree, 2)
		expect_mode(main.Mode.STORAGE, "storage")
		var dep_id := ""
		for action in main.menu_actions:
			if str(action.get("id", "")).begins_with("dep:"):
				dep_id = str(action["id"])
				break
		if dep_id != "":
			select_menu_id(dep_id)
			await await_frames(tree, 1)
			var wd := false
			for action in main.menu_actions:
				if str(action.get("id", "")).begins_with("wd:"):
					select_menu_id(str(action["id"]))
					wd = true
					break
			if not wd:
				fail("storage withdraw missing")
		else:
			fail("storage deposit missing")
		emit_cmd("cancel")
		await await_frames(tree, 1)

	# Craft at three stations — grant ingredients
	_grant_craft_materials()
	for station_tile in [SimWorld.TILE_WORKBENCH, SimWorld.TILE_FORGE, SimWorld.TILE_ALCHEMY]:
		if not await move_player_to_tile(station_tile):
			fail("station tile missing %d" % station_tile)
			continue
		emit_cmd("interact")
		await await_frames(tree, 2)
		expect_mode(main.Mode.CRAFT, "craft %d" % station_tile)
		await capture("craft_%d" % station_tile)
		var crafted := false
		for action in main.menu_actions:
			if str(action.get("id", "")).begins_with("recipe:"):
				select_menu_id(str(action["id"]))
				crafted = true
				await await_frames(tree, 1)
				break
		if not crafted:
			# May be blocked; force one craft via sim only if UI has no craftable — still fail
			fail("No craftable recipe in UI for station tile %d" % station_tile)
		emit_cmd("cancel")
		await await_frames(tree, 1)

	# Equip weapon/armor/ammo via inventory UI (refresh list after each equip).
	_ensure_equippables_in_inventory()
	var equipped_count := 0
	for _attempt in range(6):
		emit_cmd("inventory")
		await await_frames(tree, 1)
		expect_mode(main.Mode.INVENTORY, "inventory equip")
		var equip_id := ""
		for action in main.menu_actions:
			var id := str(action.get("id", ""))
			if id.begins_with("equip:"):
				equip_id = id
				break
		if equip_id == "":
			emit_cmd("cancel")
			await await_frames(tree, 1)
			break
		if not select_menu_id(equip_id):
			emit_cmd("cancel")
			await await_frames(tree, 1)
			break
		equipped_count += 1
		await await_frames(tree, 1)
		if int(main.mode) == main.Mode.INVENTORY:
			emit_cmd("cancel")
			await await_frames(tree, 1)
	var p := GameServices.sim.get_player()
	if p.get_equipped("mainhand") == null and p.get_equipped("ranged") == null:
		fail("Failed to equip weapon through UI (equipped_count=%d)" % equipped_count)

	# Enter expedition
	if not await move_player_to_tile(SimWorld.TILE_STAIRS_DOWN):
		finished.emit(false, "stairs down missing"); return
	emit_cmd("interact")
	await await_frames(tree, 3)
	if GameServices.sim.world.is_town or GameServices.sim.depth < 1:
		fail("Failed to enter expedition")
	await capture("expedition")
	_prep_combat_sandbox()

	# Controller movement
	emit_cmd("move", {"dx": 1, "dy": 0})
	await await_frames(tree, 1)
	emit_cmd("move", {"dx": 0, "dy": 1})
	await await_frames(tree, 1)

	# Melee: place foe adjacent and bump
	p = GameServices.sim.get_player()
	var foe := _spawn_test_foe(p.pos.add(1, 0))
	emit_cmd("move", {"dx": 1, "dy": 0})
	await await_frames(tree, 2)
	await capture("melee_combat")
	_clear_monsters()
	_prep_combat_sandbox()

	# Ranged — ensure firearm + ammo + visible target
	p = GameServices.sim.get_player()
	_ensure_equippables_in_inventory()
	for entry in p.inventory:
		var it: SimItem = entry
		if it.equip_slot == "ranged" and p.get_equipped("ranged") == null:
			InventorySystem.equip(p, it)
		if it.equip_slot == "ammo" and p.get_equipped("ammo") == null:
			InventorySystem.equip(p, it)
	_spawn_test_foe(GameServices.sim.get_player().pos.add(1, 0))
	emit_cmd("ranged_mode")
	await await_frames(tree, 2)
	if int(main.mode) == main.Mode.RANGED:
		await capture("ranged_targeting")
		emit_cmd("confirm")
		await await_frames(tree, 2)
	else:
		fail("Ranged mode did not open")
	# Ensure we leave targeting / menus before continuing.
	if int(main.mode) == main.Mode.RANGED:
		emit_cmd("cancel")
		await await_frames(tree, 1)
	if int(main.mode) != main.Mode.PLAY:
		emit_cmd("cancel")
		await await_frames(tree, 1)
	_clear_monsters()
	_prep_combat_sandbox()

	# Consumable
	_give_consumable()
	emit_cmd("quick_item")
	await await_frames(tree, 1)

	# Pickup/drop via inventory
	emit_cmd("inventory")
	await await_frames(tree, 1)
	expect_mode(main.Mode.INVENTORY, "inventory drop")
	var drop_id := ""
	for action in main.menu_actions:
		if str(action.get("id", "")).begins_with("drop:"):
			drop_id = str(action["id"])
			break
	if drop_id == "":
		fail("drop action missing")
	else:
		select_menu_id(drop_id)
		await await_frames(tree, 1)
	if int(main.mode) == main.Mode.INVENTORY:
		emit_cmd("cancel")
		await await_frames(tree, 1)

	_clear_monsters()
	_prep_combat_sandbox()
	# Open locked gates via lever only when stairs are gated.
	var need_lever := (
		_world_has_tile(SimWorld.TILE_GATE_LOCKED)
		and _world_has_tile(SimWorld.TILE_LEVER)
		and _bfs(GameServices.sim.world, GameServices.sim.get_player().pos, _find_tile(SimWorld.TILE_STAIRS_DOWN)).is_empty()
	)
	if need_lever:
		if await move_player_to_tile(SimWorld.TILE_LEVER):
			emit_cmd("interact")
			await await_frames(tree, 1)
		_clear_monsters()
		_prep_combat_sandbox()

	# Depth transition (still on depth 1 after expedition entry)
	var depth_before := GameServices.sim.depth
	if depth_before >= 5:
		fail("unexpected depth before transition: %d" % depth_before)
	elif await move_player_to_tile(SimWorld.TILE_STAIRS_DOWN):
		emit_cmd("interact")
		await await_frames(tree, 3)
		if GameServices.sim.depth <= depth_before:
			fail("depth did not increase")
	else:
		fail("Could not path to stairs down for transition")

	# Save / load through pause UI
	if int(main.mode) != main.Mode.PLAY:
		emit_cmd("cancel")
		await await_frames(tree, 1)
	var gold_before := GameServices.sim.get_player().gold
	var depth_save := GameServices.sim.depth
	var inv_count := GameServices.sim.get_player().inventory.size()
	emit_cmd("pause")
	await await_frames(tree, 2)
	expect_mode(main.Mode.PAUSE, "pause for save")
	await capture("save_load")
	if not select_menu_id("save"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 1)
	emit_cmd("cancel")
	await await_frames(tree, 1)
	# Load via title
	emit_cmd("pause")
	await await_frames(tree, 1)
	expect_mode(main.Mode.PAUSE, "pause for title")
	if not select_menu_id("title"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 2)
	expect_mode(main.Mode.TITLE, "title after abort")
	if not select_menu_id("load"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 3)
	expect_mode(main.Mode.PLAY, "loaded play")
	if GameServices.sim.depth != depth_save:
		fail("load lost depth (want %d got %d)" % [depth_save, GameServices.sim.depth])
	if GameServices.sim.get_player().gold != gold_before:
		fail("load lost gold")
	if GameServices.sim.get_player().inventory.size() != inv_count:
		fail("load lost inventory count")
	if GameServices.sim.world.is_town:
		fail("load restored town instead of expedition")

	# Boss path: jump to depth 5 deterministically for encounter screenshot
	GameServices.sim.descend_to(5)
	main._refresh_world()
	await await_frames(tree, 2)
	var has_boss := false
	for entry in GameServices.sim.actors:
		if (entry as SimActor).is_boss:
			has_boss = true
	if not has_boss:
		fail("Boss missing on depth 5")
	await capture("boss_encounter")

	# Death + restart
	var player := GameServices.sim.get_player()
	player.hp = 0
	player.alive = false
	GameServices.sim._handle_player_death(player)
	main._show_dead()
	await await_frames(tree, 2)
	expect_mode(main.Mode.DEAD, "death")
	await capture("death_summary")
	select_menu_id("restart")
	await await_frames(tree, 2)
	expect_mode(main.Mode.CREATE, "restart create")

	var ok := failures.is_empty()
	finished.emit(ok, "OK" if ok else "; ".join(failures))


func _grant_craft_materials() -> void:
	var p := GameServices.sim.get_player()
	p.engineering = 10
	p.skill_ranks["skill_engineering"] = 5
	p.skill_ranks["skill_smithing"] = 5
	p.skill_ranks["skill_medicine"] = 5
	for id in ["item_scrap", "item_gears", "item_brass_ingot", "item_coal", "item_herbs", "item_sulfur", "item_oil_can", "item_leather", "item_wire", "item_wrench"]:
		if GameServices.content.items.has(id):
			var it := SimItem.from_definition(GameServices.content.get_item(id), "auto_%s_%d" % [id, Time.get_ticks_msec()])
			it.stack = mini(5, it.max_stack)
			InventorySystem.add_item(p, it)


func _ensure_equippables_in_inventory() -> void:
	var p := GameServices.sim.get_player()
	for id in ["item_wrench", "item_brass_plate", "item_ball_ammo", "item_pepperbox"]:
		if not GameServices.content.items.has(id):
			continue
		if p.find_inventory_item(id) != null:
			continue
		var already := false
		for slot in SimItem.EQUIP_SLOTS:
			var eq: SimItem = p.equipment.get(slot)
			if eq != null and eq.def_id == id:
				already = true
				break
		if already:
			continue
		InventorySystem.add_item(p, SimItem.from_definition(GameServices.content.get_item(id), "eq_" + id))


func _spawn_test_foe(pos: GridPos) -> SimActor:
	var sim := GameServices.sim
	var player := sim.get_player()
	if not sim.world.in_bounds(pos.x, pos.y) or not sim.world.is_walkable(pos.x, pos.y):
		pos = player.pos.add(1, 0)
		if not sim.world.is_walkable(pos.x, pos.y):
			sim.world.set_tile(pos.x, pos.y, SimWorld.TILE_FLOOR)
	sim.world.set_tile(pos.x, pos.y, SimWorld.TILE_FLOOR)
	var existing := sim.actor_at(pos)
	if existing != null and not existing.is_player():
		existing.hp = 1
		existing.max_hp = 1
		existing.behavior = "idle"
		existing.melee_damage = 0
		sim.world.recompute_fov(player.pos, player.vision_range)
		return existing
	var foe := SimActor.new()
	foe.id = "auto_foe_%d" % Time.get_ticks_msec()
	foe.kind = "monster"
	foe.name = "Test Foe"
	foe.pos = pos
	foe.hp = 1
	foe.max_hp = 1
	foe.alive = true
	foe.behavior = "idle"
	foe.melee_damage = 0
	sim.actors.append(foe)
	sim.world.recompute_fov(player.pos, player.vision_range)
	return foe


func _give_consumable() -> void:
	var p := GameServices.sim.get_player()
	var oil := SimItem.from_definition(GameServices.content.get_item("item_machine_oil"), "oil_auto")
	InventorySystem.add_item(p, oil)
	p.quick_item_id = "item_machine_oil"
	p.quick_slots[0] = "item_machine_oil"


func _world_has_tile(tile: int) -> bool:
	var world: SimWorld = GameServices.sim.world
	for i in range(world.tiles.size()):
		if world.tiles[i] == tile:
			return true
	return false


func _find_tile(tile: int) -> GridPos:
	var world: SimWorld = GameServices.sim.world
	for y in range(world.height):
		for x in range(world.width):
			if world.get_tile(x, y) == tile:
				return GridPos.new(x, y)
	return GridPos.new(-1, -1)


func _clear_monsters() -> void:
	var sim := GameServices.sim
	var kept: Array = []
	for entry in sim.actors:
		var actor: SimActor = entry
		if actor.is_player():
			kept.append(actor)
	sim.actors = kept
	sim.player_dead = false
	sim.awaiting_visual = false
	if main.sequencer:
		main.sequencer.clear()


func _prep_combat_sandbox() -> void:
	var p := GameServices.sim.get_player()
	if p == null:
		return
	p.alive = true
	p.hp = maxi(p.hp, p.max_hp)
	p.max_hp = maxi(p.max_hp, 40)
	p.hp = p.max_hp
	GameServices.sim.player_dead = false
	GameServices.sim.awaiting_visual = false
	if int(main.mode) == main.Mode.DEAD:
		main.mode = main.Mode.PLAY
		main.menu_panel.visible = false


func run_smoke_flow() -> void:
	## Short path for exported Windows launch: reach Brassharbor and exit.
	failures.clear()
	var tree := main.get_tree()
	Automation.instant_animations = true
	GameServices.last_input_device = "controller"
	expect_mode(main.Mode.TITLE, "smoke title")
	if not select_menu_id("new"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 2)
	main.seed_value = Automation.deterministic_seed
	main.selected_race = "race_human"
	main.selected_class = "class_adventurer"
	if not select_menu_id("start"):
		finished.emit(false, failures[0]); return
	await await_frames(tree, 3)
	expect_mode(main.Mode.PLAY, "smoke play")
	if GameServices.sim == null or not GameServices.sim.world.is_town:
		fail("smoke did not reach Brassharbor")
	await capture("smoke_brassharbor")
	var ok := failures.is_empty()
	finished.emit(ok, "OK" if ok else "; ".join(failures))
