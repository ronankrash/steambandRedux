class_name SimWorld
extends RefCounted

const TILE_FLOOR := 0
const TILE_WALL := 1
const TILE_DOOR_CLOSED := 2
const TILE_DOOR_OPEN := 3
const TILE_CONTAINER := 4
const TILE_LEVER := 5
const TILE_DESTRUCTIBLE := 6
const TILE_WORKBENCH := 7
const TILE_FORGE := 8
const TILE_ALCHEMY := 9
const TILE_STAIRS_DOWN := 10
const TILE_STAIRS_UP := 11
const TILE_MERCHANT := 12
const TILE_STORAGE := 13
const TILE_HEALER := 14
const TILE_EXTRACT := 15
const TILE_HAZARD := 16
const TILE_GATE_LOCKED := 17

var width: int = 0
var height: int = 0
var tiles: PackedInt32Array = PackedInt32Array()
var explored: PackedByteArray = PackedByteArray()
var visible: PackedByteArray = PackedByteArray()
var ground_items: Dictionary = {} # "x,y" -> Array[SimItem]
var feature_meta: Dictionary = {} # "x,y" -> Dictionary (merchant_id etc)
var lever_on: bool = false
var locked_door_opened: bool = false
var theme_id: String = "env_foundry"
var depth: int = 0
var is_town: bool = false


func resize(w: int, h: int) -> void:
	width = w
	height = h
	tiles.resize(w * h)
	explored.resize(w * h)
	visible.resize(w * h)
	for i in range(w * h):
		tiles[i] = TILE_WALL
		explored[i] = 0
		visible[i] = 0
	ground_items.clear()
	feature_meta.clear()


func in_bounds(x: int, y: int) -> bool:
	return x >= 0 and y >= 0 and x < width and y < height


func idx(x: int, y: int) -> int:
	return y * width + x


func get_tile(x: int, y: int) -> int:
	if not in_bounds(x, y):
		return TILE_WALL
	return tiles[idx(x, y)]


func set_tile(x: int, y: int, tile: int) -> void:
	if in_bounds(x, y):
		tiles[idx(x, y)] = tile


func set_feature_meta(x: int, y: int, data: Dictionary) -> void:
	feature_meta[key(x, y)] = data


func get_feature_meta(x: int, y: int) -> Dictionary:
	return feature_meta.get(key(x, y), {})


func is_walkable(x: int, y: int) -> bool:
	var t := get_tile(x, y)
	return t in [
		TILE_FLOOR, TILE_DOOR_OPEN, TILE_CONTAINER, TILE_LEVER, TILE_WORKBENCH,
		TILE_FORGE, TILE_ALCHEMY, TILE_STAIRS_DOWN, TILE_STAIRS_UP, TILE_MERCHANT,
		TILE_STORAGE, TILE_HEALER, TILE_EXTRACT, TILE_HAZARD
	]


func blocks_sight(x: int, y: int) -> bool:
	var t := get_tile(x, y)
	return t in [TILE_WALL, TILE_DOOR_CLOSED, TILE_DESTRUCTIBLE, TILE_GATE_LOCKED]


func key(x: int, y: int) -> String:
	return "%d,%d" % [x, y]


func drop_item(x: int, y: int, item: SimItem) -> void:
	var k := key(x, y)
	if not ground_items.has(k):
		ground_items[k] = []
	ground_items[k].append(item)


func take_items(x: int, y: int) -> Array:
	var k := key(x, y)
	var items: Array = ground_items.get(k, [])
	ground_items.erase(k)
	return items


func recompute_fov(origin: GridPos, radius: int) -> void:
	for i in range(visible.size()):
		visible[i] = 0
	if not in_bounds(origin.x, origin.y):
		return
	# Town is fully visible
	if is_town:
		for i in range(visible.size()):
			if tiles[i] != TILE_WALL:
				visible[i] = 1
				explored[i] = 1
		return
	visible[idx(origin.x, origin.y)] = 1
	explored[idx(origin.x, origin.y)] = 1
	for y in range(origin.y - radius, origin.y + radius + 1):
		for x in range(origin.x - radius, origin.x + radius + 1):
			if not in_bounds(x, y):
				continue
			if GridPos.new(x, y).chebyshev(origin) > radius:
				continue
			if CombatResolver.has_line_of_sight(self, origin, GridPos.new(x, y)):
				visible[idx(x, y)] = 1
				explored[idx(x, y)] = 1


func is_visible(x: int, y: int) -> bool:
	return in_bounds(x, y) and visible[idx(x, y)] == 1


func is_explored(x: int, y: int) -> bool:
	return in_bounds(x, y) and explored[idx(x, y)] == 1


func flood_walkable(start: GridPos) -> int:
	if not is_walkable(start.x, start.y):
		return 0
	var seen: Dictionary = {}
	var q: Array = [start]
	seen[key(start.x, start.y)] = true
	var count := 0
	while not q.is_empty():
		var p: GridPos = q.pop_front()
		count += 1
		for n in GridPos.neighbors8():
			var nx := p.x + n.x
			var ny := p.y + n.y
			var k := key(nx, ny)
			if seen.has(k):
				continue
			if is_walkable(nx, ny):
				seen[k] = true
				q.append(GridPos.new(nx, ny))
	return count


func to_dict() -> Dictionary:
	var gi: Dictionary = {}
	for k in ground_items.keys():
		var arr: Array = []
		for entry in ground_items[k]:
			arr.append((entry as SimItem).to_dict())
		gi[k] = arr
	return {
		"width": width,
		"height": height,
		"tiles": Array(tiles),
		"explored": Array(explored),
		"ground_items": gi,
		"feature_meta": feature_meta.duplicate(true),
		"lever_on": lever_on,
		"locked_door_opened": locked_door_opened,
		"theme_id": theme_id,
		"depth": depth,
		"is_town": is_town,
	}


func from_dict(data: Dictionary) -> void:
	resize(int(data.get("width", 0)), int(data.get("height", 0)))
	var t: Array = data.get("tiles", [])
	for i in range(mini(t.size(), tiles.size())):
		tiles[i] = int(t[i])
	var e: Array = data.get("explored", [])
	for i in range(mini(e.size(), explored.size())):
		explored[i] = int(e[i])
	ground_items.clear()
	var gi: Dictionary = data.get("ground_items", {})
	for k in gi.keys():
		var arr: Array = []
		for entry in gi[k]:
			arr.append(SimItem.from_dict(entry))
		ground_items[k] = arr
	feature_meta = data.get("feature_meta", {}).duplicate(true)
	lever_on = bool(data.get("lever_on", false))
	locked_door_opened = bool(data.get("locked_door_opened", false))
	theme_id = str(data.get("theme_id", "env_foundry"))
	depth = int(data.get("depth", 0))
	is_town = bool(data.get("is_town", false))
