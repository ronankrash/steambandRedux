class_name IsoMath
extends RefCounted
## Classic 2:1 isometric projection helpers.

const TILE_W := 64
const TILE_H := 32


static func grid_to_screen(gx: int, gy: int, origin: Vector2 = Vector2.ZERO) -> Vector2:
	var sx := (gx - gy) * (TILE_W / 2)
	var sy := (gx + gy) * (TILE_H / 2)
	return origin + Vector2(sx, sy)


static func screen_to_grid(screen: Vector2, origin: Vector2 = Vector2.ZERO) -> Vector2i:
	var p := screen - origin
	var gx := int(floor((p.x / (TILE_W / 2.0) + p.y / (TILE_H / 2.0)) / 2.0))
	var gy := int(floor((p.y / (TILE_H / 2.0) - p.x / (TILE_W / 2.0)) / 2.0))
	return Vector2i(gx, gy)


static func depth_key(gx: int, gy: int) -> int:
	return gx + gy
