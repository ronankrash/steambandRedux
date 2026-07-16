class_name DungeonGenerator
extends RefCounted
## Town hub + multi-level procedural expeditions (foundry / mine themes).


static func generate_town() -> Dictionary:
	var world := SimWorld.new()
	world.resize(28, 20)
	world.is_town = true
	world.depth = 0
	world.theme_id = "town"
	# Carve plaza
	for y in range(2, 18):
		for x in range(2, 26):
			world.set_tile(x, y, SimWorld.TILE_FLOOR)
	# Services
	world.set_tile(4, 4, SimWorld.TILE_MERCHANT)
	world.set_feature_meta(4, 4, {"merchant_id": "merchant_general"})
	world.set_tile(10, 4, SimWorld.TILE_MERCHANT)
	world.set_feature_meta(10, 4, {"merchant_id": "merchant_arms"})
	world.set_tile(16, 4, SimWorld.TILE_HEALER)
	world.set_feature_meta(16, 4, {"service": "heal"})
	world.set_tile(4, 10, SimWorld.TILE_WORKBENCH)
	world.set_tile(8, 10, SimWorld.TILE_FORGE)
	world.set_tile(12, 10, SimWorld.TILE_ALCHEMY)
	world.set_tile(20, 10, SimWorld.TILE_STORAGE)
	world.set_feature_meta(20, 10, {"storage": true})
	world.set_tile(22, 15, SimWorld.TILE_STAIRS_DOWN)
	world.set_feature_meta(22, 15, {"expedition": true})
	# Reveal all
	for i in range(world.explored.size()):
		if world.tiles[i] != SimWorld.TILE_WALL:
			world.explored[i] = 1
			world.visible[i] = 1
	return {
		"world": world,
		"player_start": GridPos.new(14, 12),
		"spawn_rooms": [],
		"stairs_down": GridPos.new(22, 15),
		"stairs_up": null,
		"extract": null,
	}


static func generate_level(rng: BrassRng, depth: int, theme_id: String, width: int = 42, height: int = 30) -> Dictionary:
	var world := SimWorld.new()
	world.resize(width, height)
	world.is_town = false
	world.depth = depth
	world.theme_id = theme_id

	var rooms: Array[Rect2i] = []
	var room_count := 8 + depth
	for _i in range(room_count + 6):
		var rw := rng.randi_range(5, 10)
		var rh := rng.randi_range(5, 9)
		var rx := rng.randi_range(1, width - rw - 2)
		var ry := rng.randi_range(1, height - rh - 2)
		var room := Rect2i(rx, ry, rw, rh)
		var overlaps := false
		for other in rooms:
			if room.grow(1).intersects(other):
				overlaps = true
				break
		if overlaps:
			continue
		_carve_room(world, room, theme_id, rng)
		rooms.append(room)
		if rooms.size() >= room_count:
			break

	if rooms.is_empty():
		var fallback := Rect2i(3, 3, 10, 8)
		_carve_room(world, fallback, theme_id, rng)
		rooms.append(fallback)

	for i in range(1, rooms.size()):
		_carve_corridor(world, rooms[i - 1].get_center(), rooms[i].get_center())

	var start: Vector2i = rooms[0].get_center()
	var end: Vector2i = rooms[rooms.size() - 1].get_center()

	# Features by room templates
	_place_features(world, rooms, rng, depth, theme_id)

	var stairs_down: GridPos = null
	var stairs_up := GridPos.new(start.x, start.y)
	world.set_tile(stairs_up.x, stairs_up.y, SimWorld.TILE_STAIRS_UP)
	var extract: GridPos = null
	if depth < 5:
		stairs_down = GridPos.new(end.x, end.y)
		world.set_tile(stairs_down.x, stairs_down.y, SimWorld.TILE_STAIRS_DOWN)
	else:
		extract = GridPos.new(end.x, end.y)
		world.set_tile(extract.x, extract.y, SimWorld.TILE_EXTRACT)

	# Ensure critical objectives remain reachable for play.
	var start_pos := GridPos.new(start.x, start.y)
	var goal := stairs_down if stairs_down != null else extract
	_repair_playable_paths(world, start_pos, goal)

	return {
		"world": world,
		"player_start": start_pos,
		"spawn_rooms": rooms,
		"stairs_down": stairs_down,
		"stairs_up": stairs_up,
		"extract": extract,
	}


static func _carve_room(world: SimWorld, room: Rect2i, theme_id: String, rng: BrassRng) -> void:
	var style := rng.randi_range(0, 3)
	for y in range(room.position.y, room.position.y + room.size.y):
		for x in range(room.position.x, room.position.x + room.size.x):
			if theme_id.ends_with("mine") and style == 0 and rng.randi_range(1, 100) <= 8:
				# Jagged cavern edge left as wall
				if x == room.position.x or y == room.position.y:
					continue
			world.set_tile(x, y, SimWorld.TILE_FLOOR)


static func _carve_corridor(world: SimWorld, a: Vector2i, b: Vector2i) -> void:
	var x := a.x
	var y := a.y
	while x != b.x:
		world.set_tile(x, y, SimWorld.TILE_FLOOR)
		x += 1 if b.x > x else -1
	while y != b.y:
		world.set_tile(x, y, SimWorld.TILE_FLOOR)
		y += 1 if b.y > y else -1
	world.set_tile(b.x, b.y, SimWorld.TILE_FLOOR)


static func _place_features(world: SimWorld, rooms: Array[Rect2i], rng: BrassRng, depth: int, theme_id: String) -> void:
	if rooms.size() >= 2:
		var mid: Rect2i = rooms[mini(1, rooms.size() - 1)]
		var c := mid.get_center()
		world.set_tile(c.x, c.y, SimWorld.TILE_CONTAINER)
	if rooms.size() >= 3:
		var r: Rect2i = rooms[2]
		world.set_tile(r.position.x + 1, r.position.y + 1, SimWorld.TILE_LEVER)
		var gate := _find_corridor_tile(world, rng)
		if gate != Vector2i(-1, -1):
			world.set_tile(gate.x, gate.y, SimWorld.TILE_GATE_LOCKED)
	# Station chance deeper
	if depth >= 2 and rooms.size() >= 4:
		var r: Rect2i = rooms[3]
		var c := r.get_center()
		world.set_tile(c.x, c.y, SimWorld.TILE_WORKBENCH if theme_id.find("foundry") >= 0 else SimWorld.TILE_FORGE)
	# Hazards
	var hazard_chance := 12 if theme_id.find("mine") >= 0 else 8
	hazard_chance += depth * 2
	for r in rooms:
		if rng.randi_range(1, 100) <= hazard_chance:
			var hx := r.position.x + rng.randi_range(1, maxi(1, r.size.x - 2))
			var hy := r.position.y + rng.randi_range(1, maxi(1, r.size.y - 2))
			if world.get_tile(hx, hy) == SimWorld.TILE_FLOOR:
				world.set_tile(hx, hy, SimWorld.TILE_HAZARD)
	var dest := _find_floor_near(world, rooms[rooms.size() - 1].get_center(), rng)
	if dest != Vector2i(-1, -1):
		world.set_tile(dest.x, dest.y, SimWorld.TILE_DESTRUCTIBLE)
	var door_pos := _find_corridor_tile(world, rng)
	if door_pos != Vector2i(-1, -1) and world.get_tile(door_pos.x, door_pos.y) == SimWorld.TILE_FLOOR:
		world.set_tile(door_pos.x, door_pos.y, SimWorld.TILE_DOOR_CLOSED)


static func _find_corridor_tile(world: SimWorld, rng: BrassRng) -> Vector2i:
	var candidates: Array[Vector2i] = []
	for y in range(1, world.height - 1):
		for x in range(1, world.width - 1):
			if world.get_tile(x, y) != SimWorld.TILE_FLOOR:
				continue
			var walls := 0
			for n in [Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1)]:
				if world.get_tile(x + n.x, y + n.y) == SimWorld.TILE_WALL:
					walls += 1
			if walls >= 2:
				candidates.append(Vector2i(x, y))
	if candidates.is_empty():
		return Vector2i(-1, -1)
	return candidates[rng.randi_range(0, candidates.size() - 1)]


static func _find_floor_near(world: SimWorld, center: Vector2i, rng: BrassRng) -> Vector2i:
	var candidates: Array[Vector2i] = []
	for y in range(center.y - 3, center.y + 4):
		for x in range(center.x - 3, center.x + 4):
			if world.get_tile(x, y) == SimWorld.TILE_FLOOR and Vector2i(x, y) != center:
				candidates.append(Vector2i(x, y))
	if candidates.is_empty():
		return Vector2i(-1, -1)
	return candidates[rng.randi_range(0, candidates.size() - 1)]


static func _passable_for_play(world: SimWorld, x: int, y: int, allow_locked_gates: bool = false) -> bool:
	var t := world.get_tile(x, y)
	# Closed doors open on bump; destructibles smash on bump.
	if world.is_walkable(x, y) or t == SimWorld.TILE_DOOR_CLOSED or t == SimWorld.TILE_DESTRUCTIBLE:
		return true
	if allow_locked_gates and t == SimWorld.TILE_GATE_LOCKED:
		return true
	return false


static func _connected_for_play(world: SimWorld, start: GridPos, goal: GridPos, allow_locked_gates: bool = false) -> bool:
	if start == null or goal == null:
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
			if _passable_for_play(world, nx, ny, allow_locked_gates) or (nx == goal.x and ny == goal.y):
				seen[k] = true
				q.append(GridPos.new(nx, ny))
	return false


static func _unlock_all_gates(world: SimWorld) -> void:
	for y in range(world.height):
		for x in range(world.width):
			if world.get_tile(x, y) == SimWorld.TILE_GATE_LOCKED:
				world.set_tile(x, y, SimWorld.TILE_DOOR_CLOSED)


static func _carve_access(world: SimWorld, a: GridPos, b: GridPos) -> void:
	## Force a walkable manhattan spine between two points.
	if a == null or b == null:
		return
	var x := a.x
	var y := a.y
	while x != b.x:
		_force_floorish(world, x, y)
		x += 1 if b.x > x else -1
	while y != b.y:
		_force_floorish(world, x, y)
		y += 1 if b.y > y else -1
	_force_floorish(world, b.x, b.y)


static func _force_floorish(world: SimWorld, x: int, y: int) -> void:
	if not world.in_bounds(x, y):
		return
	var t := world.get_tile(x, y)
	# Preserve stairs / extract / interactive stations; clear hard blockers.
	if t in [
		SimWorld.TILE_STAIRS_UP, SimWorld.TILE_STAIRS_DOWN, SimWorld.TILE_EXTRACT,
		SimWorld.TILE_CONTAINER, SimWorld.TILE_LEVER, SimWorld.TILE_WORKBENCH,
		SimWorld.TILE_FORGE, SimWorld.TILE_ALCHEMY, SimWorld.TILE_HAZARD
	]:
		return
	if t in [SimWorld.TILE_WALL, SimWorld.TILE_GATE_LOCKED, SimWorld.TILE_DOOR_CLOSED, SimWorld.TILE_DESTRUCTIBLE]:
		world.set_tile(x, y, SimWorld.TILE_FLOOR)


static func _ensure_goal(world: SimWorld, start: GridPos, goal: GridPos) -> void:
	if goal == null:
		return
	if _connected_for_play(world, start, goal, false):
		return
	# If a lever can unlock gates and is reachable, allow gated routes.
	var lever := _find_tile(world, SimWorld.TILE_LEVER)
	if lever != null and _connected_for_play(world, start, lever, false):
		if _connected_for_play(world, start, goal, true):
			return
	_unlock_all_gates(world)
	if _connected_for_play(world, start, goal, false):
		return
	_carve_access(world, start, goal)


static func _repair_playable_paths(world: SimWorld, start: GridPos, goal: GridPos) -> void:
	_ensure_goal(world, start, goal)
	var container := _find_tile(world, SimWorld.TILE_CONTAINER)
	if container != null:
		_ensure_goal(world, start, container)
	if _has_tile(world, SimWorld.TILE_GATE_LOCKED):
		var lever := _find_tile(world, SimWorld.TILE_LEVER)
		if lever == null:
			_unlock_all_gates(world)
		else:
			_ensure_goal(world, start, lever)
			# If lever remains unreachable after carve, drop gates.
			if not _connected_for_play(world, start, lever, false):
				_unlock_all_gates(world)
	# Final guarantee for primary objective.
	_ensure_goal(world, start, goal)


static func _has_tile(world: SimWorld, tile: int) -> bool:
	for i in range(world.tiles.size()):
		if world.tiles[i] == tile:
			return true
	return false


static func _find_tile(world: SimWorld, tile: int) -> GridPos:
	for y in range(world.height):
		for x in range(world.width):
			if world.get_tile(x, y) == tile:
				return GridPos.new(x, y)
	return null


## Compatibility for Phase 1 tests
static func generate(rng: BrassRng, width: int = 40, height: int = 28) -> Dictionary:
	return generate_level(rng, 1, "env_foundry", width, height)
