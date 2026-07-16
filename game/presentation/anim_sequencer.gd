class_name AnimSequencer
extends Node
## Event-driven presentation queue. Simulation commits first; this only visualizes.

signal queue_empty
signal flash(world_pos: Vector2, color: Color)
signal projectile(from: Vector2, to: Vector2, color: Color)
signal damage_number(world_pos: Vector2, amount: int, critical: bool)

enum Speed { NORMAL, FAST }

var speed: Speed = Speed.NORMAL
var _queue: Array[Dictionary] = []
var _playing: bool = false
var world_view: Node2D = null
var origin: Vector2 = Vector2(640, 120)

# Actor visual offsets for tweening (actor_id -> Vector2 screen offset from grid)
var actor_offsets: Dictionary = {}
var actor_flash: Dictionary = {}
var actor_scale: Dictionary = {}
var dying: Dictionary = {} # actor_id -> fade 0..1


func duration_scale() -> float:
	return 0.45 if speed == Speed.FAST else 1.0


func is_busy() -> bool:
	return _playing or not _queue.is_empty()


func clear() -> void:
	_queue.clear()
	_playing = false
	actor_offsets.clear()
	actor_flash.clear()
	actor_scale.clear()
	dying.clear()
	_set_lock(false)


func bind_sim(sim: GameSim) -> void:
	if sim == null:
		return
	if not sim.actor_moved.is_connected(_on_moved):
		sim.actor_moved.connect(_on_moved)
	if not sim.actor_attacked.is_connected(_on_attacked):
		sim.actor_attacked.connect(_on_attacked)
	if not sim.actor_died.is_connected(_on_died):
		sim.actor_died.connect(_on_died)


func enqueue(event: Dictionary) -> void:
	_queue.append(event)
	_set_lock(true)
	if not _playing:
		_play_next()


func _set_lock(locked: bool) -> void:
	if GameServices.sim != null:
		GameServices.sim.awaiting_visual = locked


func _on_moved(actor_id: String, from: Dictionary, to: Dictionary) -> void:
	enqueue({"type": "move", "actor_id": actor_id, "from": from, "to": to})


func _on_attacked(result: Dictionary) -> void:
	enqueue({"type": "attack", "result": result})


func _on_died(actor_id: String) -> void:
	enqueue({"type": "death", "actor_id": actor_id})


func _play_next() -> void:
	if _queue.is_empty():
		_playing = false
		_set_lock(false)
		queue_empty.emit()
		return
	_playing = true
	var event: Dictionary = _queue.pop_front()
	match str(event.get("type", "")):
		"move":
			await _animate_move(event)
		"attack":
			await _animate_attack(event)
		"death":
			await _animate_death(event)
		_:
			await get_tree().create_timer(0.01).timeout
	_play_next()


func _grid_screen(pos: Dictionary) -> Vector2:
	return IsoMath.grid_to_screen(int(pos.get("x", 0)), int(pos.get("y", 0)), origin)


func _animate_move(event: Dictionary) -> void:
	var id := str(event.get("actor_id", ""))
	var from_s := _grid_screen(event.get("from", {}))
	var to_s := _grid_screen(event.get("to", {}))
	var delta := from_s - to_s
	actor_offsets[id] = delta
	var tween := create_tween()
	tween.set_trans(Tween.TRANS_SINE).set_ease(Tween.EASE_OUT)
	tween.tween_method(func(v: Vector2): actor_offsets[id] = v, delta, Vector2.ZERO, 0.14 * duration_scale())
	await tween.finished
	actor_offsets.erase(id)
	if world_view:
		world_view.queue_redraw()
		if world_view.has_method("center_on_player"):
			world_view.center_on_player()


func _animate_attack(event: Dictionary) -> void:
	var result: Dictionary = event.get("result", {})
	var attacker_id := str(result.get("attacker_id", ""))
	var defender_id := str(result.get("defender_id", ""))
	var atk := GameServices.sim.get_actor(attacker_id) if GameServices.sim else null
	var def := GameServices.sim.get_actor(defender_id) if GameServices.sim else null
	if atk == null or def == null:
		await get_tree().create_timer(0.05 * duration_scale()).timeout
		return
	var from_s := IsoMath.grid_to_screen(atk.pos.x, atk.pos.y, origin)
	var to_s := IsoMath.grid_to_screen(def.pos.x, def.pos.y, origin)
	if str(result.get("type", "")) == "ranged":
		projectile.emit(from_s, to_s, Color(0.95, 0.75, 0.25))
		await get_tree().create_timer(0.12 * duration_scale()).timeout
	else:
		var lunge := (to_s - from_s).normalized() * 10.0
		actor_offsets[attacker_id] = Vector2.ZERO
		var tween := create_tween()
		tween.tween_method(func(v: Vector2): actor_offsets[attacker_id] = v, Vector2.ZERO, lunge, 0.07 * duration_scale())
		tween.tween_method(func(v: Vector2): actor_offsets[attacker_id] = v, lunge, Vector2.ZERO, 0.07 * duration_scale())
		await tween.finished
		actor_offsets.erase(attacker_id)
	if result.get("hit", false):
		actor_flash[defender_id] = Color(1, 0.3, 0.2)
		flash.emit(to_s, Color(1, 0.4, 0.2, 0.8))
		damage_number.emit(to_s + Vector2(0, -18), int(result.get("damage", 0)), false)
		await get_tree().create_timer(0.1 * duration_scale()).timeout
		actor_flash.erase(defender_id)
	else:
		damage_number.emit(to_s + Vector2(0, -12), 0, false)
		await get_tree().create_timer(0.06 * duration_scale()).timeout
	if world_view:
		world_view.queue_redraw()


func _animate_death(event: Dictionary) -> void:
	var id := str(event.get("actor_id", ""))
	dying[id] = 0.0
	var tween := create_tween()
	tween.tween_method(func(v: float): dying[id] = v, 0.0, 1.0, 0.22 * duration_scale())
	await tween.finished
	dying.erase(id)
	actor_offsets.erase(id)
	actor_flash.erase(id)
	if world_view:
		world_view.queue_redraw()
