extends SceneTree
## Connectivity / determinism checks across many expedition seeds.

const SEED_COUNT := 500
const DEPTHS := 5
## Seeds that previously failed connectivity before generator repair.
const REGRESSION_SEEDS: Array[int] = [
	1476, 1510, 1612, 3618, 7052, 8191, 9279, 1017, 1187, 1357
]

var failures: int = 0
var passes: int = 0
var failing_seeds: Array[Dictionary] = []


func _initialize() -> void:
	call_deferred("_run")


func _run() -> void:
	var content := ContentDB.new()
	if not content.load_all():
		print("FAIL: content load")
		quit(1)
		return
	for seed_value in REGRESSION_SEEDS:
		_check_seed(content, seed_value)
	for i in range(SEED_COUNT):
		var seed_value := 1000 + i * 17
		_check_seed(content, seed_value)
	print("Dungeon seeds: %d checked, %d seed-failures, asserts pass=%d fail=%d" % [
		SEED_COUNT, failing_seeds.size(), passes, failures
	])
	if not failing_seeds.is_empty():
		var report := {"failing_seeds": failing_seeds}
		var f := FileAccess.open("user://dungeon_seed_failures.json", FileAccess.WRITE)
		if f:
			f.store_string(JSON.stringify(report, "\t"))
		print("FAILING_SEEDS: ", JSON.stringify(failing_seeds))
	quit(1 if failures > 0 else 0)


func _ok(cond: bool, seed_value: int, msg: String) -> void:
	if cond:
		passes += 1
	else:
		failures += 1
		failing_seeds.append({"seed": seed_value, "error": msg})
		print("FAIL seed ", seed_value, ": ", msg)


func _check_seed(content: ContentDB, seed_value: int) -> void:
	var sim := GameSim.new()
	sim.content = content
	if not sim.new_game(seed_value, "race_human", "class_adventurer"):
		_ok(false, seed_value, "new_game failed")
		return
	_ok(sim.world.is_town, seed_value, "town missing")
	# Determinism: two gens same seed
	var a := DungeonGenerator.generate_level(BrassRng.new(), 1, "env_foundry")
	# Reseed properly
	var rng1 := BrassRng.new()
	rng1.reseed(seed_value)
	var rng2 := BrassRng.new()
	rng2.reseed(seed_value)
	var g1 := DungeonGenerator.generate_level(rng1, 1, "env_foundry")
	var g2 := DungeonGenerator.generate_level(rng2, 1, "env_foundry")
	var same := true
	for i in range(g1["world"].tiles.size()):
		if g1["world"].tiles[i] != g2["world"].tiles[i]:
			same = false
			break
	_ok(same, seed_value, "nondeterministic layout")
	_ok((g1["player_start"] as GridPos).equals(g2["player_start"]), seed_value, "nondeterministic start")

	sim.start_expedition()
	for depth in range(1, DEPTHS + 1):
		if sim.depth != depth:
			sim.descend_to(depth)
		var world: SimWorld = sim.world
		var start: GridPos = sim.get_player().pos
		_ok(world.is_walkable(start.x, start.y), seed_value, "spawn blocked depth %d" % depth)
		_ok(not world.blocks_sight(start.x, start.y) or world.is_walkable(start.x, start.y), seed_value, "spawn terrain depth %d" % depth)
		# No living actor in walls
		for entry in sim.actors:
			var actor: SimActor = entry
			if not actor.alive:
				continue
			if not world.is_walkable(actor.pos.x, actor.pos.y):
				_ok(false, seed_value, "actor in wall %s d%d" % [actor.id, depth])
		var reachable := world.flood_walkable(start)
		_ok(reachable > 10, seed_value, "tiny reachable area d%d" % depth)
		# Entry stairs up except we start on stairs_up tile
		var has_up := _has_tile(world, SimWorld.TILE_STAIRS_UP)
		_ok(has_up or depth == 0, seed_value, "missing stairs up d%d" % depth)
		if depth < 5:
			var has_down := _has_tile(world, SimWorld.TILE_STAIRS_DOWN)
			_ok(has_down, seed_value, "missing stairs down d%d" % depth)
			if has_down:
				var down := _find_tile(world, SimWorld.TILE_STAIRS_DOWN)
				var reach := _can_reach(world, start, down)
				_ok(reach, seed_value, "stairs down unreachable d%d" % depth)
		else:
			var has_extract := _has_tile(world, SimWorld.TILE_EXTRACT)
			_ok(has_extract, seed_value, "missing extract d5")
			if has_extract:
				var ex := _find_tile(world, SimWorld.TILE_EXTRACT)
				_ok(_can_reach(world, start, ex), seed_value, "extract unreachable")
			var boss := false
			for entry in sim.actors:
				if (entry as SimActor).is_boss:
					boss = true
					var bpos: GridPos = (entry as SimActor).pos
					_ok(_can_reach(world, start, bpos) or world.is_walkable(bpos.x, bpos.y), seed_value, "boss unreachable")
			_ok(boss, seed_value, "boss missing d5")
		# Locked gates: if gate exists, lever should exist and be reachable OR gate already openable
		if _has_tile(world, SimWorld.TILE_GATE_LOCKED):
			var lever := _find_tile(world, SimWorld.TILE_LEVER)
			_ok(lever != null, seed_value, "locked gate without lever d%d" % depth)
			if lever != null:
				_ok(_can_reach(world, start, lever), seed_value, "lever unreachable d%d" % depth)
		# Containers reachable if present
		if _has_tile(world, SimWorld.TILE_CONTAINER):
			var c := _find_tile(world, SimWorld.TILE_CONTAINER)
			_ok(_can_reach(world, start, c), seed_value, "container unreachable d%d" % depth)


func _has_tile(world: SimWorld, tile: int) -> bool:
	for i in range(world.tiles.size()):
		if world.tiles[i] == tile:
			return true
	return false


func _find_tile(world: SimWorld, tile: int) -> GridPos:
	for y in range(world.height):
		for x in range(world.width):
			if world.get_tile(x, y) == tile:
				return GridPos.new(x, y)
	return null


func _passable(world: SimWorld, x: int, y: int, allow_locked_gates: bool = false) -> bool:
	var t := world.get_tile(x, y)
	# Closed doors open on bump; destructibles smash on bump.
	if world.is_walkable(x, y) or t == SimWorld.TILE_DOOR_CLOSED or t == SimWorld.TILE_DESTRUCTIBLE:
		return true
	if allow_locked_gates and t == SimWorld.TILE_GATE_LOCKED:
		return true
	return false


func _flood(world: SimWorld, start: GridPos, goal: GridPos, allow_locked_gates: bool) -> bool:
	if goal == null:
		return false
	var seen: Dictionary = {}
	var q: Array = [start]
	seen["%d,%d" % [start.x, start.y]] = true
	while not q.is_empty():
		var p: GridPos = q.pop_front()
		if p.equals(goal):
			return true
		for n in GridPos.neighbors8():
			var nx := p.x + n.x
			var ny := p.y + n.y
			var k := "%d,%d" % [nx, ny]
			if seen.has(k):
				continue
			if _passable(world, nx, ny, allow_locked_gates) or (nx == goal.x and ny == goal.y):
				seen[k] = true
				q.append(GridPos.new(nx, ny))
	return false


func _can_reach(world: SimWorld, start: GridPos, goal: GridPos) -> bool:
	if _flood(world, start, goal, false):
		return true
	# Gated routes are valid when the lever is obtainable without crossing a gate.
	if not _has_tile(world, SimWorld.TILE_GATE_LOCKED):
		return false
	var lever := _find_tile(world, SimWorld.TILE_LEVER)
	if lever == null:
		return false
	if not _flood(world, start, lever, false):
		return false
	return _flood(world, start, goal, true)
