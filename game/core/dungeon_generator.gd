class_name DungeonGenerator
extends RefCounted
## Procedural subterranean industrial ruin: caverns, masonry, machinery.


static func generate(rng: BrassRng, width: int = 40, height: int = 28) -> Dictionary:
	var world := SimWorld.new()
	world.resize(width, height)

	# Carve rooms
	var rooms: Array[Rect2i] = []
	for _i in range(10):
		var rw := rng.randi_range(5, 9)
		var rh := rng.randi_range(5, 8)
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
		_carve_room(world, room)
		rooms.append(room)

	if rooms.is_empty():
		var fallback := Rect2i(3, 3, 8, 6)
		_carve_room(world, fallback)
		rooms.append(fallback)

	for i in range(1, rooms.size()):
		_carve_corridor(world, rooms[i - 1].get_center(), rooms[i].get_center())

	var start: Vector2i = rooms[0].get_center()
	var end: Vector2i = rooms[rooms.size() - 1].get_center()

	# Place features
	world.set_tile(end.x, end.y, SimWorld.TILE_WORKBENCH)
	var door_pos := _find_corridor_tile(world, rng)
	if door_pos != Vector2i(-1, -1):
		world.set_tile(door_pos.x, door_pos.y, SimWorld.TILE_DOOR_CLOSED)
	var lever_pos := GridPos.new(rooms[0].position.x + 1, rooms[0].position.y + 1)
	world.set_tile(lever_pos.x, lever_pos.y, SimWorld.TILE_LEVER)
	var crate_pos := GridPos.new(rooms[mini(1, rooms.size() - 1)].get_center().x, rooms[mini(1, rooms.size() - 1)].get_center().y)
	if world.is_walkable(crate_pos.x, crate_pos.y):
		world.set_tile(crate_pos.x, crate_pos.y, SimWorld.TILE_CONTAINER)
	var dest_pos := _find_floor_near(world, end, rng)
	if dest_pos != Vector2i(-1, -1):
		world.set_tile(dest_pos.x, dest_pos.y, SimWorld.TILE_DESTRUCTIBLE)

	return {
		"world": world,
		"player_start": GridPos.new(start.x, start.y),
		"spawn_rooms": rooms,
	}


static func _carve_room(world: SimWorld, room: Rect2i) -> void:
	for y in range(room.position.y, room.position.y + room.size.y):
		for x in range(room.position.x, room.position.x + room.size.x):
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
