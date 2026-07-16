class_name GridPos
extends RefCounted
## Integer grid coordinate for the turn-based simulation.

var x: int = 0
var y: int = 0


func _init(px: int = 0, py: int = 0) -> void:
	x = px
	y = py


func equals(other: GridPos) -> bool:
	return x == other.x and y == other.y


func add(dx: int, dy: int) -> GridPos:
	return GridPos.new(x + dx, y + dy)


func manhattan(other: GridPos) -> int:
	return absi(x - other.x) + absi(y - other.y)


func chebyshev(other: GridPos) -> int:
	return maxi(absi(x - other.x), absi(y - other.y))


func to_dict() -> Dictionary:
	return {"x": x, "y": y}


static func from_dict(data: Dictionary) -> GridPos:
	return GridPos.new(int(data.get("x", 0)), int(data.get("y", 0)))


static func neighbors8() -> Array[Vector2i]:
	return [
		Vector2i(0, -1), Vector2i(1, -1), Vector2i(1, 0), Vector2i(1, 1),
		Vector2i(0, 1), Vector2i(-1, 1), Vector2i(-1, 0), Vector2i(-1, -1),
	]
