class_name GameSim
extends RefCounted
## Turn-based simulation orchestrator. Presentation-free.

signal log_message(text: String)
signal actor_moved(actor_id: String, from: Dictionary, to: Dictionary)
signal actor_attacked(result: Dictionary)
signal actor_died(actor_id: String)
signal game_over(summary: String)

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
	if combat_log.size() > 100:
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

	var gen := DungeonGenerator.generate(rng)
	world = gen["world"]
	var start: GridPos = gen["player_start"]

	var player := _create_player(race_id, class_id, start)
	actors.append(player)
	_spawn_enemies(gen["spawn_rooms"])
	_spawn_starter_loot(start)
	world.recompute_fov(player.pos, player.vision_range)
	push_log("You descend into the brass deep.")
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
	player.skill_points = 0
	player.vision_range = 8

	var wrench := SimItem.from_definition(content.get_item("item_wrench"), _next_id("item"))
	var pistol := SimItem.from_definition(content.get_item("item_pepperbox"), _next_id("item"))
	var ammo := SimItem.from_definition(content.get_item("item_ball_ammo"), _next_id("item"))
	ammo.stack = 12
	var scrap := SimItem.from_definition(content.get_item("item_scrap"), _next_id("item"))
	scrap.stack = 3
	var oil := SimItem.from_definition(content.get_item("item_machine_oil"), _next_id("item"))
	InventorySystem.add_item(player, wrench)
	InventorySystem.add_item(player, pistol)
	InventorySystem.add_item(player, ammo)
	InventorySystem.add_item(player, scrap)
	InventorySystem.add_item(player, oil)
	InventorySystem.equip(player, wrench)
	player.quick_item_id = "item_machine_oil"
	return player


func _spawn_enemies(rooms: Array) -> void:
	var defs := ["mon_gear_rat", "mon_clockwork_sentry", "mon_steam_wraith"]
	for i in range(mini(rooms.size(), defs.size())):
		var room: Rect2i = rooms[i]
		var pos := GridPos.new(room.get_center().x + 1, room.get_center().y)
		if not world.is_walkable(pos.x, pos.y) or actor_at(pos) != null:
			pos = GridPos.new(room.get_center().x, room.get_center().y + 1)
		if actor_at(pos) != null:
			continue
		actors.append(_create_monster(defs[i], pos))


func _create_monster(def_id: String, pos: GridPos) -> SimActor:
	var def := content.get_monster(def_id)
	var mon := SimActor.new()
	mon.id = _next_id("mon")
	mon.kind = "monster"
	mon.name = str(def.get("name", def_id))
	mon.pos = pos
	mon.max_hp = int(def.get("hp", 8))
	mon.hp = mon.max_hp
	mon.melee_accuracy = int(def.get("melee_accuracy", 40))
	mon.melee_damage = int(def.get("melee_damage", 2))
	mon.ranged_accuracy = int(def.get("ranged_accuracy", 35))
	mon.ranged_damage = int(def.get("ranged_damage", 2))
	mon.defense = int(def.get("defense", 0))
	mon.behavior = str(def.get("behavior", "melee_pursuer"))
	mon.vision_range = int(def.get("vision", 6))
	return mon


func _spawn_starter_loot(near: GridPos) -> void:
	var drop_pos := near.add(1, 0)
	if world.is_walkable(drop_pos.x, drop_pos.y):
		var affix := content.get_affix("affix_reinforced")
		var brass := SimItem.from_definition(content.get_item("item_brass_plate"), _next_id("item"), affix)
		world.drop_item(drop_pos.x, drop_pos.y, brass)


func try_player_move(dx: int, dy: int) -> Dictionary:
	if player_dead or awaiting_visual:
		return {"ok": false, "reason": "busy"}
	var player := get_player()
	if player == null or not player.alive:
		return {"ok": false, "reason": "dead"}
	var target := player.pos.add(dx, dy)
	if not world.in_bounds(target.x, target.y):
		return {"ok": false, "reason": "oob"}
	var other := actor_at(target)
	if other != null and other.alive and not other.is_player():
		return _player_melee(player, other)
	var tile := world.get_tile(target.x, target.y)
	if tile == SimWorld.TILE_DOOR_CLOSED:
		world.set_tile(target.x, target.y, SimWorld.TILE_DOOR_OPEN)
		push_log("You force the iron door open.")
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
	var ammo := player.find_inventory_item("item_ball_ammo")
	if ammo == null:
		return {"ok": false, "reason": "no_ammo"}
	ammo.stack -= 1
	if ammo.stack <= 0:
		player.inventory.erase(ammo)
	var result := CombatResolver.resolve_ranged(rng, player, foe)
	_apply_attack_result(result, player, foe)
	_end_player_turn()
	return {"ok": true, "combat": result}


func _apply_attack_result(result: Dictionary, attacker: SimActor, defender: SimActor) -> void:
	actor_attacked.emit(result)
	if result["hit"]:
		push_log("%s hits %s for %d." % [attacker.name, defender.name, result["damage"]])
	else:
		push_log("%s misses %s." % [attacker.name, defender.name])
	if result["defender_dead"]:
		push_log("%s is destroyed." % defender.name)
		actor_died.emit(defender.id)
		if attacker.is_player():
			attacker.skill_points += 1
		_maybe_drop(defender)


func _maybe_drop(mon: SimActor) -> void:
	# Find definition by name match is fragile; use behavior mapping for vertical slice.
	var drop_id := ""
	if mon.behavior == "melee_pursuer":
		drop_id = "item_scrap"
	elif mon.behavior == "ranged_sentry":
		drop_id = "item_ball_ammo"
	elif mon.behavior == "poison_spitter":
		drop_id = "item_machine_oil"
	if drop_id == "":
		return
	var affix := {}
	var def := content.get_item(drop_id)
	var allowed: Array = def.get("allowed_affixes", [])
	if not allowed.is_empty() and rng.randi_range(1, 100) <= 35:
		affix = content.get_affix(str(allowed[rng.randi_range(0, allowed.size() - 1)]))
	var item := SimItem.from_definition(def, _next_id("item"), affix)
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
			push_log("Inventory full; left %s." % item.name)


func interact() -> Dictionary:
	var player := get_player()
	if player == null or player_dead:
		return {"ok": false}
	var tile := world.get_tile(player.pos.x, player.pos.y)
	if tile == SimWorld.TILE_LEVER:
		world.lever_on = not world.lever_on
		if world.lever_on:
			# Open a nearby closed door if any.
			for n in GridPos.neighbors8():
				var tx := player.pos.x + n.x
				var ty := player.pos.y + n.y
				if world.get_tile(tx, ty) == SimWorld.TILE_DOOR_CLOSED:
					world.set_tile(tx, ty, SimWorld.TILE_DOOR_OPEN)
					world.locked_door_opened = true
			push_log("You throw the lever. Machinery clanks.")
		else:
			push_log("You reset the lever.")
		_end_player_turn()
		return {"ok": true, "lever": world.lever_on}
	if tile == SimWorld.TILE_CONTAINER:
		var loot := SimItem.from_definition(content.get_item("item_scrap"), _next_id("item"))
		loot.stack = 2
		if InventorySystem.add_item(player, loot):
			push_log("You rifle the crate and take scrap.")
		world.set_tile(player.pos.x, player.pos.y, SimWorld.TILE_FLOOR)
		_end_player_turn()
		return {"ok": true, "container": true}
	if tile == SimWorld.TILE_WORKBENCH:
		return {"ok": true, "workbench": true}
	push_log("Nothing to interact with.")
	return {"ok": false}


func craft_recipe(recipe_id: String) -> Dictionary:
	var player := get_player()
	if player == null or player_dead:
		return {"ok": false}
	if world.get_tile(player.pos.x, player.pos.y) != SimWorld.TILE_WORKBENCH:
		return {"ok": false, "reason": "need_workbench"}
	var recipe := content.get_recipe(recipe_id)
	if recipe.is_empty():
		return {"ok": false, "reason": "unknown_recipe"}
	var result := CraftingSystem.craft(player, recipe, content, _next_id("item"))
	if not result.get("ok", false):
		push_log("Cannot craft that.")
		return result
	var item: SimItem = result["item"]
	if result.get("dropped", false):
		world.drop_item(player.pos.x, player.pos.y, item)
		push_log("Crafted %s but inventory was full; dropped it." % item.name)
	else:
		push_log("Crafted %s." % item.name)
	_end_player_turn()
	return result


func spend_skill_point(stat: String) -> Dictionary:
	var player := get_player()
	if player == null or player.skill_points <= 0:
		return {"ok": false}
	match stat:
		"melee":
			player.melee_accuracy += 2
			player.melee_damage += 1
		"ranged":
			player.ranged_accuracy += 2
			player.ranged_damage += 1
		"defense":
			player.defense += 1
		"engineering":
			player.engineering += 1
		_:
			return {"ok": false, "reason": "unknown_stat"}
	player.skill_points -= 1
	push_log("You hone your %s." % stat)
	return {"ok": true}


func use_quick_item() -> Dictionary:
	var player := get_player()
	if player == null:
		return {"ok": false}
	var item := player.find_inventory_item(player.quick_item_id)
	if item == null:
		return {"ok": false, "reason": "missing"}
	if item.def_id == "item_machine_oil":
		player.hp = mini(player.max_hp, player.hp + 8)
		InventorySystem.remove_item_count(player, item.def_id, 1)
		push_log("You apply machine oil and recover.")
		_end_player_turn()
		return {"ok": true}
	return {"ok": false, "reason": "unusable"}


func _end_player_turn() -> void:
	turn_index += 1
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
	for entry in actors:
		var mon: SimActor = entry
		if not mon.alive or mon.is_player():
			continue
		_enemy_act(mon, player)


func _enemy_act(mon: SimActor, player: SimActor) -> void:
	if mon.pos.chebyshev(player.pos) > mon.vision_range:
		return
	if not CombatResolver.has_line_of_sight(world, mon.pos, player.pos):
		return
	match mon.behavior:
		"ranged_sentry":
			if mon.pos.chebyshev(player.pos) <= 5 and CombatResolver.has_line_of_sight(world, mon.pos, player.pos):
				var result := CombatResolver.resolve_ranged(rng, mon, player)
				_apply_attack_result(result, mon, player)
				return
		"poison_spitter":
			if mon.pos.chebyshev(player.pos) <= 4:
				var result := CombatResolver.resolve_ranged(rng, mon, player)
				_apply_attack_result(result, mon, player)
				if result["hit"]:
					player.add_status("poison", 3)
					push_log("Venomous steam clings to you.")
				return
		_:
			pass
	if mon.pos.chebyshev(player.pos) <= 1:
		var result := CombatResolver.resolve_melee(rng, mon, player)
		_apply_attack_result(result, mon, player)
		return
	# Step toward player
	var dx := clampi(player.pos.x - mon.pos.x, -1, 1)
	var dy := clampi(player.pos.y - mon.pos.y, -1, 1)
	var next := mon.pos.add(dx, dy)
	if world.is_walkable(next.x, next.y) and actor_at(next) == null:
		var from := mon.pos.to_dict()
		mon.pos = next
		actor_moved.emit(mon.id, from, mon.pos.to_dict())


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
		actor.tick_statuses()


func _handle_player_death(player: SimActor) -> void:
	player_dead = true
	death_summary = "Operative fell on turn %d in Brassdeep (seed %d)." % [turn_index, seed_value]
	push_log(death_summary)
	game_over.emit(death_summary)
