extends Node2D
## Isometric world view. Draws tiles and actors with correct depth order.

@onready var camera: Camera2D = $Camera2D

var origin: Vector2 = Vector2(640, 120)
var animating: bool = false
var tile_colors := {
	SimWorld.TILE_FLOOR: Color(0.35, 0.32, 0.28),
	SimWorld.TILE_WALL: Color(0.18, 0.16, 0.14),
	SimWorld.TILE_DOOR_CLOSED: Color(0.45, 0.28, 0.12),
	SimWorld.TILE_DOOR_OPEN: Color(0.55, 0.4, 0.22),
	SimWorld.TILE_CONTAINER: Color(0.5, 0.35, 0.15),
	SimWorld.TILE_LEVER: Color(0.7, 0.55, 0.2),
	SimWorld.TILE_DESTRUCTIBLE: Color(0.4, 0.2, 0.15),
	SimWorld.TILE_WORKBENCH: Color(0.3, 0.45, 0.5),
}


func _ready() -> void:
	if camera:
		camera.make_current()


func center_on_player() -> void:
	var player := GameServices.sim.get_player()
	if player == null or camera == null:
		return
	camera.position = IsoMath.grid_to_screen(player.pos.x, player.pos.y, origin)


func peek_camera(dx: float, dy: float) -> void:
	if camera == null:
		return
	var player := GameServices.sim.get_player()
	if player == null:
		return
	var base := IsoMath.grid_to_screen(player.pos.x, player.pos.y, origin)
	camera.position = base + Vector2(dx, dy) * 48.0


func _draw() -> void:
	var sim := GameServices.sim
	if sim == null or sim.world.width == 0:
		return
	var world := sim.world
	var draw_list: Array = []
	for y in range(world.height):
		for x in range(world.width):
			if not world.is_explored(x, y):
				continue
			draw_list.append({"kind": "tile", "x": x, "y": y, "depth": IsoMath.depth_key(x, y)})
	for entry in sim.actors:
		var actor: SimActor = entry
		if not actor.alive:
			continue
		if not world.is_visible(actor.pos.x, actor.pos.y):
			continue
		draw_list.append({"kind": "actor", "actor": actor, "depth": IsoMath.depth_key(actor.pos.x, actor.pos.y)})
	draw_list.sort_custom(func(a, b): return a["depth"] < b["depth"])
	for entry in draw_list:
		if entry["kind"] == "tile":
			_draw_tile(entry["x"], entry["y"], world)
		else:
			_draw_actor(entry["actor"])


func _draw_tile(x: int, y: int, world: SimWorld) -> void:
	var screen := IsoMath.grid_to_screen(x, y, origin)
	var tile := world.get_tile(x, y)
	var color: Color = tile_colors.get(tile, Color.GRAY)
	if not world.is_visible(x, y):
		color = color.darkened(0.45)
	var pts := PackedVector2Array([
		screen + Vector2(0, 0),
		screen + Vector2(IsoMath.TILE_W / 2.0, IsoMath.TILE_H / 2.0),
		screen + Vector2(0, IsoMath.TILE_H),
		screen + Vector2(-IsoMath.TILE_W / 2.0, IsoMath.TILE_H / 2.0),
	])
	draw_colored_polygon(pts, color)
	if tile == SimWorld.TILE_WALL:
		draw_colored_polygon(PackedVector2Array([
			screen + Vector2(-IsoMath.TILE_W / 2.0, IsoMath.TILE_H / 2.0),
			screen + Vector2(0, 0),
			screen + Vector2(0, -16),
			screen + Vector2(-IsoMath.TILE_W / 2.0, IsoMath.TILE_H / 2.0 - 16),
		]), color.lightened(0.08))


func _draw_actor(actor: SimActor) -> void:
	var screen := IsoMath.grid_to_screen(actor.pos.x, actor.pos.y, origin) + Vector2(0, 4)
	var color := Color(0.85, 0.75, 0.35) if actor.is_player() else Color(0.75, 0.25, 0.2)
	if actor.behavior == "ranged_sentry":
		color = Color(0.35, 0.55, 0.75)
	elif actor.behavior == "poison_spitter":
		color = Color(0.45, 0.7, 0.35)
	draw_circle(screen, 10.0, color)
	draw_rect(Rect2(screen + Vector2(-6, -18), Vector2(12, 14)), color.darkened(0.2))
