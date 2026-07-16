class_name AiController
extends RefCounted
## Deterministic monster AI decisions. Returns action dictionaries for GameSim.


static func decide(sim: GameSim, mon: SimActor, player: SimActor) -> Dictionary:
	if not mon.alive or player == null or not player.alive:
		return {"type": "wait"}
	var dist := mon.pos.chebyshev(player.pos)
	var sees := dist <= mon.vision_range and CombatResolver.has_line_of_sight(sim.world, mon.pos, player.pos)
	var hears := dist <= mon.hear_radius
	if sees or hears:
		mon.ai_state = "alert"
		mon.alert_turns = 4
	elif mon.alert_turns > 0:
		mon.alert_turns -= 1
		mon.ai_state = "search"
	else:
		mon.ai_state = "idle"

	if mon.ai_state == "idle":
		return {"type": "patrol"}

	match mon.behavior:
		"flee":
			if dist <= 5:
				return {"type": "flee", "from": player.pos}
			return {"type": "wait"}
		"defensive":
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			if dist <= 3:
				return {"type": "wait"}
			return {"type": "approach", "target": player.pos}
		"ranged_sentry":
			if dist <= 5 and sees:
				return {"type": "ranged", "target": player.id}
			if dist > 4:
				return {"type": "approach", "target": player.pos}
			return {"type": "kite", "from": player.pos}
		"poison_spitter":
			if dist <= 4 and sees:
				return {"type": "ranged", "target": player.id, "status": "poison", "status_turns": 3}
			return {"type": "approach", "target": player.pos}
		"steam_burster":
			if dist <= 2:
				return {"type": "aoe", "ability": "steam_burst", "radius": 1, "damage": 3, "status": "steam", "status_turns": 2}
			return {"type": "approach", "target": player.pos}
		"fearer":
			if dist <= 3 and sees:
				return {"type": "ranged", "target": player.id, "status": "fear", "status_turns": 2, "damage_bonus": 0}
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			return {"type": "approach", "target": player.pos}
		"repairer":
			var ally := _nearest_hurt_ally(sim, mon)
			if ally != null and mon.pos.chebyshev(ally.pos) <= 1:
				return {"type": "repair", "target": ally.id, "heal": 4}
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			return {"type": "approach", "target": player.pos}
		"summoner":
			if mon.alert_turns >= 3 and sim.rng.randi_range(1, 100) <= 25:
				return {"type": "summon", "monster_id": "mon_gear_rat"}
			if dist <= 4 and sees:
				return {"type": "ranged", "target": player.id}
			return {"type": "approach", "target": player.pos}
		"pack":
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			return {"type": "approach", "target": player.pos}
		"elite", "boss":
			if dist <= 2 and sim.rng.randi_range(1, 100) <= 40:
				return {"type": "aoe", "ability": "steam_burst", "radius": 1, "damage": 4 if mon.is_boss else 3, "status": "steam", "status_turns": 2}
			if dist <= 5 and sees and sim.rng.randi_range(1, 100) <= 45:
				return {"type": "ranged", "target": player.id}
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			return {"type": "approach", "target": player.pos}
		_:
			if dist <= 1:
				return {"type": "melee", "target": player.id}
			return {"type": "approach", "target": player.pos}


static func _nearest_hurt_ally(sim: GameSim, mon: SimActor) -> SimActor:
	var best: SimActor = null
	var best_d := 999
	for entry in sim.actors:
		var other: SimActor = entry
		if other.id == mon.id or not other.alive or other.is_player():
			continue
		if other.hp >= other.max_hp:
			continue
		var d := mon.pos.chebyshev(other.pos)
		if d < best_d:
			best_d = d
			best = other
	return best


static func step_toward(world: SimWorld, from: GridPos, target: GridPos, occupied: Callable) -> GridPos:
	var dx := clampi(target.x - from.x, -1, 1)
	var dy := clampi(target.y - from.y, -1, 1)
	var next := from.add(dx, dy)
	if world.is_walkable(next.x, next.y) and not occupied.call(next):
		return next
	# Try axis fallbacks deterministically
	for n in [GridPos.new(dx, 0), GridPos.new(0, dy)]:
		if n.x == 0 and n.y == 0:
			continue
		next = from.add(n.x, n.y)
		if world.is_walkable(next.x, next.y) and not occupied.call(next):
			return next
	return from


static func step_away(world: SimWorld, from: GridPos, threat: GridPos, occupied: Callable) -> GridPos:
	var dx := clampi(from.x - threat.x, -1, 1)
	var dy := clampi(from.y - threat.y, -1, 1)
	return step_toward(world, from, from.add(dx, dy), occupied)
