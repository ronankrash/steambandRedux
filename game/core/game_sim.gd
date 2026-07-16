class_name GameSim
extends RefCounted
## Turn-based simulation orchestrator. Presentation-free.

signal log_message(text: String)
signal actor_moved(actor_id: String, from: Dictionary, to: Dictionary)
signal actor_attacked(result: Dictionary)
signal actor_died(actor_id: String)
signal game_over(summary: String)
signal area_changed(depth: int, is_town: bool)

var content: ContentDB
var rng: BrassRng = BrassRng.new()
var world: SimWorld = SimWorld.new()
var actors: Array = []
var seed_value: int = 1
var turn_index: int = 0
var combat_log: Array[String] = []
var next_instance: int = 1
var player_id: String = "player"
var player_dead: bool = false
var death_summary: String = ""
var awaiting_visual: bool = false
var depth: int = 0
var max_depth_reached: int = 0
var expedition_active: bool = false
var theme_schedule: Array = ["env_foundry", "env_mine", "env_foundry", "env_mine", "env_foundry"]
var merchants: Dictionary = {} # merchant_id -> state
var storage: Array = [] # Array[SimItem] hub stash
var settings: Dictionary = {"anim_speed": "normal", "ui_scale": 1.0, "text_size": 16}


func _next_id(prefix: String) -> String:
	var id := "%s_%d" % [prefix, next_instance]
	next_instance += 1
	return id


func get_player() -> SimActor:
	for entry in actors:
		var actor: SimActor = entry
		if actor.id == player_id:
			return actor
	return null


func get_actor(id: String) -> SimActor:
	for entry in actors:
		var actor: SimActor = entry
		if actor.id == id:
			return actor
	return null


func actor_at(pos: GridPos) -> SimActor:
	for entry in actors:
		var actor: SimActor = entry
		if actor.alive and actor.pos.equals(pos):
			return actor
	return null


func push_log(text: String) -> void:
	combat_log.append(text)
	if combat_log.size() > 120:
		combat_log.remove_at(0)
	log_message.emit(text)


func new_game(seed_in: int, race_id: String, class_id: String) -> bool:
	if content == null:
		return false
	seed_value = seed_in if seed_in != 0 else 1
	rng = BrassRng.new()
	rng.reseed(seed_value)
	turn_index = 0
	combat_log.clear()
	actors.clear()
	player_dead = false
	death_summary = ""
	next_instance = 1
	depth = 0
	max_depth_reached = 0
	expedition_active = false
	storage.clear()
	merchants.clear()
	for mid in content.merchants.keys():
		merchants[mid] = EconomySystem.build_merchant_state(content.merchants[mid])

	var gen := DungeonGenerator.generate_town()
	world = gen["world"]
	var start: GridPos = gen["player_start"]
	var player := _create_player(race_id, class_id, start)
	actors.append(player)
	world.recompute_fov(player.pos, player.vision_range)
	push_log("You arrive in Brassharbor. Prepare, then descend.")
	area_changed.emit(0, true)
	return true


func _create_player(race_id: String, class_id: String, start: GridPos) -> SimActor:
	var race := content.get_race(race_id)
	var klass := content.get_class_def(class_id)
	var player := SimActor.new()
	player.id = player_id
	player.kind = "player"
	player.name = "Operative"
	player.pos = start
	player.race_id = race_id
	player.class_id = class_id
	player.max_hp = int(race.get("base_hp", 20)) + int(klass.get("hp_bonus", 0))
	player.hp = player.max_hp
	player.melee_accuracy = int(race.get("melee_accuracy", 50)) + int(klass.get("melee_accuracy", 0))
	player.melee_damage = int(race.get("melee_damage", 2)) + int(klass.get("melee_damage", 0))
	player.ranged_accuracy = int(race.get("ranged_accuracy", 45)) + int(klass.get("ranged_accuracy", 0))
	player.ranged_damage = int(race.get("ranged_damage", 2)) + int(klass.get("ranged_damage", 0))
	player.defense = int(race.get("defense", 0)) + int(klass.get("defense", 0))
	player.engineering = int(race.get("engineering", 0)) + int(klass.get("engineering", 0))
	player.marksmanship = int(race.get("marksmanship", 0)) + int(klass.get("marksmanship", 0))
	player.smithing = int(race.get("smithing", 0)) + int(klass.get("smithing", 0))
	player.medicine = int(race.get("medicine", 0)) + int(klass.get("medicine", 0))
	player.stealth = int(race.get("stealth", 0)) + int(klass.get("stealth", 0))
	player.occult = int(race.get("occult", 0)) + int(klass.get("occult", 0))
	player.resistances = race.get("resistances", {}).duplicate(true)
	player.skill_points = 2
	player.gold = 40 + int(klass.get("starting_gold", 0))
	player.vision_range = 8
	player.encumbrance_cap = 42.0
	SkillSystem.apply_starting_skills(player, klass.get("starting_skills", {}), content)
	_give_starting_gear(player, race, klass)
	return player


func _give_starting_gear(player: SimActor, race: Dictionary, klass: Dictionary) -> void:
	var packs: Array = []
	packs.append_array(race.get("starting_items", []))
	packs.append_array(klass.get("starting_items", []))
	# Guarantees
	packs.append({"item_id": "item_wrench", "count": 1})
	packs.append({"item_id": "item_pepperbox", "count": 1})
	packs.append({"item_id": "item_ball_ammo", "count": 12})
	packs.append({"item_id": "item_scrap", "count": 3})
	packs.append({"item_id": "item_machine_oil", "count": 1})
	for entry in packs:
		var def_id := str(entry.get("item_id", ""))
		var count := int(entry.get("count", 1))
		if def_id == "" or content.get_item(def_id).is_empty():
			continue
		var item := SimItem.from_definition(content.get_item(def_id), _next_id("item"))
		item.stack = mini(count, item.max_stack)
		InventorySystem.add_item(player, item)
		if count > item.max_stack:
			var extra := SimItem.from_definition(content.get_item(def_id), _next_id("item"))
			extra.stack = count - item.max_stack
			InventorySystem.add_item(player, extra)
	# Equip defaults
	var wrench := player.find_inventory_item("item_wrench")
	if wrench:
		InventorySystem.equip(player, wrench)
	var pistol := player.find_inventory_item("item_pepperbox")
	if pistol:
		InventorySystem.equip(player, pistol)
	var ammo := player.find_inventory_item("item_ball_ammo")
	if ammo:
		InventorySystem.equip(player, ammo)
	player.quick_item_id = "item_machine_oil"
	player.quick_slots[0] = "item_machine_oil"


func start_expedition() -> Dictionary:
	if player_dead:
		return {"ok": false}
	expedition_active = true
	return descend_to(1)


func descend_to(target_depth: int) -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	depth = target_depth
	max_depth_reached = maxi(max_depth_reached, depth)
	var theme := str(theme_schedule[(depth - 1) % theme_schedule.size()])
	# Keep player, clear monsters
	actors = [player]
	var gen := DungeonGenerator.generate_level(rng, depth, theme)
	world = gen["world"]
	player.pos = gen["player_start"]
	_spawn_enemies(gen["spawn_rooms"], depth, theme)
	_spawn_loot(gen["spawn_rooms"], depth)
	world.recompute_fov(player.pos, player.vision_range)
	push_log("You descend to depth %d (%s)." % [depth, theme])
	area_changed.emit(depth, false)
	return {"ok": true, "depth": depth}


func return_to_town(extracted: bool = false) -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	# Remove monsters
	actors = [player]
	var gen := DungeonGenerator.generate_town()
	world = gen["world"]
	player.pos = gen["player_start"]
	expedition_active = false
	depth = 0
	for mid in merchants.keys():
		EconomySystem.maybe_restock(merchants[mid], max_depth_reached)
	if extracted:
		player.xp += 25 + max_depth_reached * 10
		player.skill_points += 1
		push_log("Extraction successful. You recover in Brassharbor.")
		_try_level_up(player)
	else:
		push_log("You return to Brassharbor.")
	world.recompute_fov(player.pos, player.vision_range)
	area_changed.emit(0, true)
	return {"ok": true}


func _spawn_enemies(rooms: Array, level_depth: int, theme: String) -> void:
	var pool: Array = []
	for mid in content.monsters.keys():
		var def: Dictionary = content.monsters[mid]
		if bool(def.get("is_boss", false)):
			continue
		var tags: Array = def.get("theme_tags", [])
		var theme_key := theme.replace("env_", "")
		var tag_hit := tags.is_empty() or "any" in tags or theme in tags or theme_key in tags
		if tag_hit:
			pool.append(mid)
	if pool.is_empty():
		pool = content.monsters.keys()
	var count := mini(rooms.size(), 3 + level_depth)
	for i in range(count):
		var room: Rect2i = rooms[i % rooms.size()]
		var pos := GridPos.new(room.get_center().x + (i % 2), room.get_center().y)
		if not world.is_walkable(pos.x, pos.y) or actor_at(pos) != null:
			pos = GridPos.new(room.get_center().x, room.get_center().y + 1)
		if actor_at(pos) != null or not world.is_walkable(pos.x, pos.y):
			continue
		var pick := str(pool[rng.randi_range(0, pool.size() - 1)])
		# Scale elites deeper
		var mon := _create_monster(pick, pos, level_depth)
		if level_depth >= 4 and rng.randi_range(1, 100) <= 20:
			mon.is_elite = true
			mon.max_hp += 8
			mon.hp = mon.max_hp
			mon.name = "Elite " + mon.name
		actors.append(mon)
	if level_depth >= 5:
		var boss_id := ""
		for mid in content.monsters.keys():
			if bool(content.monsters[mid].get("is_boss", false)) or str(content.monsters[mid].get("behavior", "")) == "boss":
				boss_id = mid
				break
		if boss_id != "" and rooms.size() > 0:
			var room: Rect2i = rooms[rooms.size() - 1]
			var placed := false
			for oy in range(-2, 3):
				for ox in range(-2, 3):
					var bpos := GridPos.new(room.get_center().x + ox, room.get_center().y + oy)
					if world.is_walkable(bpos.x, bpos.y) and actor_at(bpos) == null:
						actors.append(_create_monster(boss_id, bpos, level_depth))
						placed = true
						break
				if placed:
					break
			if not placed:
				# Force-clear a tile near end room center
				var bpos := GridPos.new(room.get_center().x, room.get_center().y)
				world.set_tile(bpos.x, bpos.y, SimWorld.TILE_FLOOR)
				var blocker := actor_at(bpos)
				if blocker != null and not blocker.is_player():
					blocker.alive = false
				actors.append(_create_monster(boss_id, bpos, level_depth))


func _create_monster(def_id: String, pos: GridPos, level_depth: int = 1) -> SimActor:
	var def := content.get_monster(def_id)
	var mon := SimActor.new()
	mon.id = _next_id("mon")
	mon.kind = "monster"
	mon.monster_def_id = def_id
	mon.name = str(def.get("name", def_id))
	mon.pos = pos
	mon.max_hp = int(def.get("hp", 8)) + (level_depth - 1)
	mon.hp = mon.max_hp
	mon.melee_accuracy = int(def.get("melee_accuracy", 40))
	mon.melee_damage = int(def.get("melee_damage", 2)) + int((level_depth - 1) / 2)
	mon.ranged_accuracy = int(def.get("ranged_accuracy", 35))
	mon.ranged_damage = int(def.get("ranged_damage", 2))
	mon.defense = int(def.get("defense", 0))
	mon.behavior = str(def.get("behavior", "melee_pursuer"))
	mon.vision_range = int(def.get("vision", 6))
	mon.hear_radius = int(def.get("hear_radius", 5))
	mon.is_elite = bool(def.get("is_elite", false))
	mon.is_boss = bool(def.get("is_boss", false)) or mon.behavior == "boss"
	mon.xp_value = int(def.get("xp", 8)) + level_depth * 2
	return mon


func _spawn_loot(rooms: Array, level_depth: int) -> void:
	if rooms.is_empty():
		return
	var room: Rect2i = rooms[0]
	var drop_pos := GridPos.new(room.get_center().x + 1, room.get_center().y)
	if world.is_walkable(drop_pos.x, drop_pos.y):
		var item := ItemGenerator.create_item(rng, content, "item_brass_plate", _next_id("item"), 50)
		if item:
			world.drop_item(drop_pos.x, drop_pos.y, item)
	if level_depth >= 2 and rooms.size() > 1:
		var r: Rect2i = rooms[1]
		var p := GridPos.new(r.get_center().x, r.get_center().y)
		var mat_id := "item_gears" if content.items.has("item_gears") else "item_scrap"
		var mat := SimItem.from_definition(content.get_item(mat_id), _next_id("item"))
		mat.stack = 2
		world.drop_item(p.x, p.y, mat)


func try_player_move(dx: int, dy: int) -> Dictionary:
	if player_dead or awaiting_visual:
		return {"ok": false, "reason": "busy"}
	var player := get_player()
	if player == null or not player.alive:
		return {"ok": false, "reason": "dead"}
	if player.status_effects.has("fear") and rng.randi_range(1, 100) <= 40:
		push_log("Fear roots you in place.")
		_end_player_turn()
		return {"ok": true, "feared": true}
	if player.is_encumbered() and (dx != 0 or dy != 0):
		# Encumbered: still move but message once
		pass
	var target := player.pos.add(dx, dy)
	if not world.in_bounds(target.x, target.y):
		return {"ok": false, "reason": "oob"}
	var other := actor_at(target)
	if other != null and other.alive and not other.is_player():
		return _player_melee(player, other)
	var tile := world.get_tile(target.x, target.y)
	if tile == SimWorld.TILE_DOOR_CLOSED or tile == SimWorld.TILE_GATE_LOCKED:
		if tile == SimWorld.TILE_GATE_LOCKED and not world.lever_on:
			push_log("The gate is locked. Find a lever.")
			return {"ok": false, "reason": "locked"}
		world.set_tile(target.x, target.y, SimWorld.TILE_DOOR_OPEN)
		push_log("You open the passage.")
		_end_player_turn()
		return {"ok": true, "opened_door": true}
	if not world.is_walkable(target.x, target.y):
		if tile == SimWorld.TILE_DESTRUCTIBLE:
			world.set_tile(target.x, target.y, SimWorld.TILE_FLOOR)
			push_log("You smash the rusted boiler.")
			_end_player_turn()
			return {"ok": true, "destroyed": true}
		return {"ok": false, "reason": "blocked"}
	var from := player.pos.to_dict()
	player.pos = target
	actor_moved.emit(player.id, from, player.pos.to_dict())
	_pickup_here(player)
	if world.get_tile(player.pos.x, player.pos.y) == SimWorld.TILE_HAZARD:
		player.hp -= 1
		push_log("Scorching vents burn you.")
		if player.hp <= 0:
			player.alive = false
	_end_player_turn()
	return {"ok": true, "moved": true}


func _player_melee(player: SimActor, foe: SimActor) -> Dictionary:
	var result := CombatResolver.resolve_melee(rng, player, foe)
	_apply_attack_result(result, player, foe)
	_end_player_turn()
	return {"ok": true, "combat": result}


func try_player_ranged(target_id: String) -> Dictionary:
	if player_dead or awaiting_visual:
		return {"ok": false, "reason": "busy"}
	var player := get_player()
	var foe := get_actor(target_id)
	if player == null or foe == null or not foe.alive:
		return {"ok": false, "reason": "invalid_target"}
	if player.weapon_range() <= 0:
		return {"ok": false, "reason": "no_firearm"}
	if player.pos.chebyshev(foe.pos) > player.weapon_range():
		return {"ok": false, "reason": "out_of_range"}
	if not CombatResolver.has_line_of_sight(world, player.pos, foe.pos):
		return {"ok": false, "reason": "no_los"}
	var ammo := player.find_ammo()
	if ammo == null:
		return {"ok": false, "reason": "no_ammo"}
	ammo.stack -= 1
	if ammo.stack <= 0:
		if player.equipment.get("ammo") == ammo:
			player.equipment["ammo"] = null
		else:
			player.inventory.erase(ammo)
	var result := CombatResolver.resolve_ranged(rng, player, foe)
	_apply_attack_result(result, player, foe)
	_end_player_turn()
	return {"ok": true, "combat": result}


func _apply_attack_result(result: Dictionary, attacker: SimActor, defender: SimActor) -> void:
	actor_attacked.emit(result)
	if result["hit"]:
		var crit := " critically" if result.get("critical", false) else ""
		push_log("%s hits%s %s for %d." % [attacker.name, crit, defender.name, result["damage"]])
	else:
		push_log("%s misses %s." % [attacker.name, defender.name])
	if result["defender_dead"]:
		push_log("%s is destroyed." % defender.name)
		actor_died.emit(defender.id)
		if attacker.is_player():
			attacker.xp += maxi(1, defender.xp_value)
			attacker.skill_points += 1 if defender.is_boss else 0
			_try_level_up(attacker)
		_maybe_drop(defender)


func _try_level_up(player: SimActor) -> void:
	var need := player.level * 40
	while player.xp >= need:
		player.xp -= need
		player.level += 1
		player.skill_points += 2
		player.max_hp += 3
		player.hp = player.max_hp
		push_log("You reach operative level %d." % player.level)
		need = player.level * 40


func _maybe_drop(mon: SimActor) -> void:
	var def := content.get_monster(mon.monster_def_id)
	var drop_id := str(def.get("drop_item_id", ""))
	if drop_id == "" or content.get_item(drop_id).is_empty():
		if mon.behavior == "melee_pursuer":
			drop_id = "item_scrap"
		elif mon.behavior == "ranged_sentry":
			drop_id = "item_ball_ammo"
		else:
			drop_id = "item_scrap"
	var item := ItemGenerator.create_item(rng, content, drop_id, _next_id("item"), 30)
	if item == null:
		return
	if item.max_stack > 1:
		item.stack = 1
	world.drop_item(mon.pos.x, mon.pos.y, item)


func _pickup_here(player: SimActor) -> void:
	var items := world.take_items(player.pos.x, player.pos.y)
	for entry in items:
		var item: SimItem = entry
		if InventorySystem.add_item(player, item):
			push_log("Picked up %s." % item.name)
		else:
			world.drop_item(player.pos.x, player.pos.y, item)
			push_log("Too encumbered; left %s." % item.name)


func interact() -> Dictionary:
	var player := get_player()
	if player == null or player_dead:
		return {"ok": false}
	var tile := world.get_tile(player.pos.x, player.pos.y)
	var meta := world.get_feature_meta(player.pos.x, player.pos.y)
	match tile:
		SimWorld.TILE_LEVER:
			world.lever_on = not world.lever_on
			if world.lever_on:
				for n in GridPos.neighbors8():
					var tx := player.pos.x + n.x
					var ty := player.pos.y + n.y
					if world.get_tile(tx, ty) == SimWorld.TILE_GATE_LOCKED:
						world.set_tile(tx, ty, SimWorld.TILE_DOOR_OPEN)
						world.locked_door_opened = true
				push_log("You throw the lever. Machinery clanks.")
			else:
				push_log("You reset the lever.")
			_end_player_turn()
			return {"ok": true, "lever": world.lever_on}
		SimWorld.TILE_CONTAINER:
			var loot := ItemGenerator.create_item(rng, content, "item_scrap", _next_id("item"), 10)
			if loot:
				loot.stack = 2
				if InventorySystem.add_item(player, loot):
					push_log("You rifle the crate.")
			world.set_tile(player.pos.x, player.pos.y, SimWorld.TILE_FLOOR)
			_end_player_turn()
			return {"ok": true, "container": true}
		SimWorld.TILE_WORKBENCH:
			return {"ok": true, "station": "workbench"}
		SimWorld.TILE_FORGE:
			return {"ok": true, "station": "forge"}
		SimWorld.TILE_ALCHEMY:
			return {"ok": true, "station": "alchemy"}
		SimWorld.TILE_MERCHANT:
			return {"ok": true, "merchant": str(meta.get("merchant_id", "merchant_general"))}
		SimWorld.TILE_HEALER:
			return {"ok": true, "healer": true}
		SimWorld.TILE_STORAGE:
			return {"ok": true, "storage": true}
		SimWorld.TILE_STAIRS_DOWN:
			if world.is_town or meta.get("expedition", false):
				return start_expedition()
			return descend_to(depth + 1)
		SimWorld.TILE_STAIRS_UP:
			if depth <= 1:
				return return_to_town(false)
			return descend_to(depth - 1)
		SimWorld.TILE_EXTRACT:
			return return_to_town(true)
	push_log("Nothing to interact with.")
	return {"ok": false}


func craft_recipe(recipe_id: String, station: String = "") -> Dictionary:
	var player := get_player()
	if player == null or player_dead:
		return {"ok": false}
	var tile := world.get_tile(player.pos.x, player.pos.y)
	var here := ""
	match tile:
		SimWorld.TILE_WORKBENCH:
			here = "workbench"
		SimWorld.TILE_FORGE:
			here = "forge"
		SimWorld.TILE_ALCHEMY:
			here = "alchemy"
	if here == "":
		return {"ok": false, "reason": "need_station"}
	var recipe := content.get_recipe(recipe_id)
	if recipe.is_empty():
		return {"ok": false, "reason": "unknown_recipe"}
	if str(recipe.get("station", "workbench")) != here:
		return {"ok": false, "reason": "wrong_station"}
	var result := CraftingSystem.craft(player, recipe, content, _next_id("item"))
	if not result.get("ok", false):
		push_log(str(result.get("reason", "Cannot craft that.")))
		return result
	var item: SimItem = result["item"]
	if result.get("dropped", false):
		world.drop_item(player.pos.x, player.pos.y, item)
		push_log("Crafted %s but inventory was full; dropped it." % item.name)
	else:
		push_log("Crafted %s." % item.name)
	if not world.is_town:
		_end_player_turn()
	return result


func spend_skill_point(stat: String) -> Dictionary:
	# Legacy quick spend maps to skill ranks
	var map := {
		"melee": "skill_melee",
		"ranged": "skill_marksmanship",
		"defense": "skill_defense",
		"engineering": "skill_engineering",
	}
	var skill_id := str(map.get(stat, stat))
	return raise_skill(skill_id)


func raise_skill(skill_id: String) -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	var def := content.get_skill(skill_id)
	if def.is_empty():
		return {"ok": false, "reason": "unknown_skill"}
	var affinities: Dictionary = {}
	var race := content.get_race(player.race_id)
	var klass := content.get_class_def(player.class_id)
	affinities.merge(race.get("skill_affinities", {}), true)
	affinities.merge(klass.get("skill_affinities", {}), true)
	var result := SkillSystem.raise_skill(player, def, content, affinities)
	if result.get("ok", false):
		push_log("Advanced %s to rank %d." % [def.get("name", skill_id), result["rank"]])
	return result


func use_quick_item(slot: int = 0) -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	var def_id := player.quick_item_id
	if slot >= 0 and slot < player.quick_slots.size() and str(player.quick_slots[slot]) != "":
		def_id = str(player.quick_slots[slot])
	var item := player.find_inventory_item(def_id)
	if item == null:
		return {"ok": false, "reason": "missing"}
	var result := InventorySystem.use_item(player, item)
	if result.get("ok", false):
		push_log("You use %s." % def_id)
		if not world.is_town:
			_end_player_turn()
	return result


func buy_from_merchant(merchant_id: String, item_id: String) -> Dictionary:
	var player := get_player()
	if player == null or not merchants.has(merchant_id):
		return {"ok": false}
	var result := EconomySystem.buy(player, merchants[merchant_id], content.get_item(item_id), content, _next_id("item"))
	if result.get("ok", false):
		push_log("Bought %s for %d brass." % [item_id, result["price"]])
	return result


func sell_to_merchant(merchant_id: String, instance_id: String) -> Dictionary:
	var player := get_player()
	if player == null or not merchants.has(merchant_id):
		return {"ok": false}
	var item: SimItem = null
	for entry in player.inventory:
		if (entry as SimItem).instance_id == instance_id:
			item = entry
			break
	if item == null:
		return {"ok": false, "reason": "missing"}
	var result := EconomySystem.sell(player, merchants[merchant_id], item)
	if result.get("ok", false):
		push_log("Sold for %d brass." % result["price"])
	return result

func heal_at_service() -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	var result := EconomySystem.heal_service(player, 15)
	if result.get("ok", false):
		push_log("The healer patches you up.")
	return result


func storage_deposit(instance_id: String) -> Dictionary:
	var player := get_player()
	var item := InventorySystem.remove_instance(player, instance_id)
	if item == null:
		return {"ok": false}
	storage.append(item)
	return {"ok": true}


func storage_withdraw(index: int) -> Dictionary:
	var player := get_player()
	if index < 0 or index >= storage.size():
		return {"ok": false}
	var item: SimItem = storage[index]
	if not InventorySystem.add_item(player, item):
		return {"ok": false, "reason": "inventory_full"}
	storage.remove_at(index)
	return {"ok": true}


func _end_player_turn() -> void:
	turn_index += 1
	if not world.is_town:
		_run_enemy_turns()
	_tick_world()
	var player := get_player()
	if player != null:
		world.recompute_fov(player.pos, player.vision_range)
		if not player.alive:
			_handle_player_death(player)


func _run_enemy_turns() -> void:
	var player := get_player()
	if player == null:
		return
	# Copy list in case summons append
	var snapshot: Array = actors.duplicate()
	for entry in snapshot:
		var mon: SimActor = entry
		if not mon.alive or mon.is_player():
			continue
		_execute_ai(mon, player)


func _execute_ai(mon: SimActor, player: SimActor) -> void:
	var action := AiController.decide(self, mon, player)
	var occupied := func(pos: GridPos) -> bool: return actor_at(pos) != null
	match str(action.get("type", "wait")):
		"melee":
			var result := CombatResolver.resolve_melee(rng, mon, player)
			_apply_attack_result(result, mon, player)
		"ranged":
			var result := CombatResolver.resolve_ranged(rng, mon, player)
			_apply_attack_result(result, mon, player)
			if result["hit"] and action.has("status"):
				player.add_status(str(action["status"]), int(action.get("status_turns", 2)))
				push_log("%s afflicts you with %s." % [mon.name, action["status"]])
		"approach":
			var approach_target: GridPos = action.get("target", player.pos) as GridPos
			if approach_target == null:
				approach_target = player.pos
			var next := AiController.step_toward(world, mon.pos, approach_target, occupied)
			if not next.equals(mon.pos):
				var from := mon.pos.to_dict()
				mon.pos = next
				actor_moved.emit(mon.id, from, mon.pos.to_dict())
		"flee", "kite":
			var threat: GridPos = action.get("from", player.pos) as GridPos
			if threat == null:
				threat = player.pos
			var next := AiController.step_away(world, mon.pos, threat, occupied)
			if not next.equals(mon.pos):
				var from := mon.pos.to_dict()
				mon.pos = next
				actor_moved.emit(mon.id, from, mon.pos.to_dict())
		"aoe":
			var radius := int(action.get("radius", 1))
			if mon.pos.chebyshev(player.pos) <= radius:
				var dmg := int(action.get("damage", 3))
				player.hp -= dmg
				push_log("%s unleashes %s for %d!" % [mon.name, str(action.get("ability", "burst")), dmg])
				if action.has("status"):
					player.add_status(str(action["status"]), int(action.get("status_turns", 2)))
				if player.hp <= 0:
					player.hp = 0
					player.alive = false
					actor_died.emit(player.id)
		"repair":
			var ally := get_actor(str(action.get("target", "")))
			if ally:
				ally.hp = mini(ally.max_hp, ally.hp + int(action.get("heal", 4)))
				push_log("%s repairs %s." % [mon.name, ally.name])
		"summon":
			var spawn := mon.pos.add(1, 0)
			if world.is_walkable(spawn.x, spawn.y) and actor_at(spawn) == null:
				actors.append(_create_monster(str(action.get("monster_id", "mon_gear_rat")), spawn, depth))
				push_log("%s deploys a minion!" % mon.name)
		"patrol":
			var dirs := GridPos.neighbors8()
			var n: Vector2i = dirs[rng.randi_range(0, dirs.size() - 1)]
			var next := mon.pos.add(n.x, n.y)
			if world.is_walkable(next.x, next.y) and actor_at(next) == null:
				var from := mon.pos.to_dict()
				mon.pos = next
				actor_moved.emit(mon.id, from, mon.pos.to_dict())
		_:
			pass


func _tick_world() -> void:
	for entry in actors:
		var actor: SimActor = entry
		if not actor.alive:
			continue
		if actor.status_effects.has("poison"):
			actor.hp -= 1
			push_log("%s suffers poison." % actor.name)
			if actor.hp <= 0:
				actor.hp = 0
				actor.alive = false
				actor_died.emit(actor.id)
		if actor.status_effects.has("steam"):
			actor.hp -= 1
			if actor.hp <= 0:
				actor.hp = 0
				actor.alive = false
				actor_died.emit(actor.id)
		actor.tick_statuses()


func _handle_player_death(player: SimActor) -> void:
	player_dead = true
	death_summary = "Operative fell on turn %d at depth %d (seed %d, Lv%d)." % [turn_index, depth, seed_value, player.level]
	push_log(death_summary)
	game_over.emit(death_summary)


func affinities_for_player() -> Dictionary:
	var player := get_player()
	if player == null:
		return {}
	var out: Dictionary = {}
	out.merge(content.get_race(player.race_id).get("skill_affinities", {}), true)
	out.merge(content.get_class_def(player.class_id).get("skill_affinities", {}), true)
	return out
