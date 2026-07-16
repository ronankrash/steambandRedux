extends Node2D
## Isometric world view with depth sorting and animation offsets.

@onready var camera: Camera2D = $Camera2D

var origin: Vector2 = Vector2(640, 120)
var animating: bool = false
var sequencer: AnimSequencer = null
var floating_texts: Array = [] # {pos, text, life, color}
var tracers: Array = [] # {from, to, life, color}

var tile_colors := {
	SimWorld.TILE_FLOOR: Color(0.35, 0.32, 0.28),
	SimWorld.TILE_WALL: Color(0.18, 0.16, 0.14),
	SimWorld.TILE_DOOR_CLOSED: Color(0.45, 0.28, 0.12),
	SimWorld.TILE_DOOR_OPEN: Color(0.55, 0.4, 0.22),
	SimWorld.TILE_CONTAINER: Color(0.5, 0.35, 0.15),
	SimWorld.TILE_LEVER: Color(0.7, 0.55, 0.2),
	SimWorld.TILE_DESTRUCTIBLE: Color(0.4, 0.2, 0.15),
	SimWorld.TILE_WORKBENCH: Color(0.3, 0.45, 0.5),
	SimWorld.TILE_FORGE: Color(0.65, 0.3, 0.15),
	SimWorld.TILE_ALCHEMY: Color(0.35, 0.55, 0.35),
	SimWorld.TILE_STAIRS_DOWN: Color(0.25, 0.25, 0.4),
	SimWorld.TILE_STAIRS_UP: Color(0.3, 0.35, 0.55),
	SimWorld.TILE_MERCHANT: Color(0.55, 0.5, 0.25),
	SimWorld.TILE_STORAGE: Color(0.4, 0.35, 0.3),
	SimWorld.TILE_HEALER: Color(0.45, 0.65, 0.55),
	SimWorld.TILE_EXTRACT: Color(0.7, 0.65, 0.3),
	SimWorld.TILE_HAZARD: Color(0.7, 0.25, 0.1),
	SimWorld.TILE_GATE_LOCKED: Color(0.35, 0.2, 0.25),
}


func _ready() -> void:
	if camera:
		camera.make_current()


func bind_sequencer(seq: AnimSequencer) -> void:
	sequencer = seq
	seq.world_view = self
	seq.origin = origin
	if not seq.flash.is_connected(_on_flash):
		seq.flash.connect(_on_flash)
	if not seq.projectile.is_connected(_on_projectile):
		seq.projectile.connect(_on_projectile)
	if not seq.damage_number.is_connected(_on_damage_number):
		seq.damage_number.connect(_on_damage_number)


func center_on_player() -> void:
	var player := GameServices.sim.get_player()
	if player == null or camera == null:
		return
	var target := IsoMath.grid_to_screen(player.pos.x, player.pos.y, origin)
	if sequencer and sequencer.actor_offsets.has(player.id):
		target += sequencer.actor_offsets[player.id]
	camera.position = camera.position.lerp(target, 0.35)


func peek_camera(dx: float, dy: float) -> void:
	if camera == null:
		return
	var player := GameServices.sim.get_player()
	if player == null:
		return
	var base := IsoMath.grid_to_screen(player.pos.x, player.pos.y, origin)
	camera.position = base + Vector2(dx, dy) * 48.0


func _process(delta: float) -> void:
	var dirty := false
	for i in range(floating_texts.size() - 1, -1, -1):
		floating_texts[i]["life"] -= delta
		floating_texts[i]["pos"] += Vector2(0, -18.0 * delta)
		if floating_texts[i]["life"] <= 0:
			floating_texts.remove_at(i)
		dirty = true
	for i in range(tracers.size() - 1, -1, -1):
		tracers[i]["life"] -= delta
		if tracers[i]["life"] <= 0:
			tracers.remove_at(i)
		dirty = true
	if dirty or (sequencer and sequencer.is_busy()):
		queue_redraw()
		center_on_player()


func _on_flash(world_pos: Vector2, color: Color) -> void:
	floating_texts.append({"pos": world_pos, "text": "*", "life": 0.2, "color": color})
	queue_redraw()


func _on_projectile(from: Vector2, to: Vector2, color: Color) -> void:
	tracers.append({"from": from, "to": to, "life": 0.15, "color": color})
	queue_redraw()


func _on_damage_number(world_pos: Vector2, amount: int, _critical: bool) -> void:
	var text := "miss" if amount <= 0 else str(amount)
	floating_texts.append({"pos": world_pos, "text": text, "life": 0.55, "color": Color(1, 0.85, 0.4)})
	queue_redraw()


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
		if not actor.alive and not (sequencer and sequencer.dying.has(actor.id)):
			continue
		if not world.is_visible(actor.pos.x, actor.pos.y) and not world.is_town:
			continue
		draw_list.append({"kind": "actor", "actor": actor, "depth": IsoMath.depth_key(actor.pos.x, actor.pos.y)})
	draw_list.sort_custom(func(a, b): return a["depth"] < b["depth"])
	for entry in draw_list:
		if entry["kind"] == "tile":
			_draw_tile(entry["x"], entry["y"], world)
		else:
			_draw_actor(entry["actor"])
	for t in tracers:
		draw_line(t["from"], t["to"], t["color"], 2.0)
	for ft in floating_texts:
		draw_string(ThemeDB.fallback_font, ft["pos"], str(ft["text"]), HORIZONTAL_ALIGNMENT_LEFT, -1, 14, ft["color"])


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
	if sequencer and sequencer.actor_offsets.has(actor.id):
		screen += sequencer.actor_offsets[actor.id]
	var color := Color(0.85, 0.75, 0.35) if actor.is_player() else Color(0.75, 0.25, 0.2)
	if actor.behavior == "ranged_sentry":
		color = Color(0.35, 0.55, 0.75)
	elif actor.behavior == "poison_spitter":
		color = Color(0.45, 0.7, 0.35)
	elif actor.is_boss:
		color = Color(0.85, 0.2, 0.55)
	elif actor.is_elite:
		color = Color(0.9, 0.5, 0.2)
	if sequencer and sequencer.actor_flash.has(actor.id):
		color = sequencer.actor_flash[actor.id]
	var alpha := 1.0
	if sequencer and sequencer.dying.has(actor.id):
		alpha = 1.0 - float(sequencer.dying[actor.id])
		color.a = alpha
	draw_circle(screen, 10.0 if not actor.is_boss else 14.0, color)
	draw_rect(Rect2(screen + Vector2(-6, -18), Vector2(12, 14)), Color(color.r, color.g, color.b, alpha).darkened(0.2))
