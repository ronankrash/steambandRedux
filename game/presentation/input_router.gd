class_name InputRouter
extends Node
## Maps keyboard and XInput-style controller to simulation commands.

signal command(action: String, payload: Dictionary)

var move_cooldown: float = 0.0


func _process(delta: float) -> void:
	move_cooldown = maxf(0.0, move_cooldown - delta)
	_poll_controller_presence()


func _poll_controller_presence() -> void:
	for device in Input.get_connected_joypads():
		if Input.is_joy_button_pressed(device, JOY_BUTTON_A) or Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down").length() > 0.4:
			GameServices.last_input_device = "controller"
			break


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventKey or event is InputEventMouse:
		GameServices.last_input_device = "keyboard"
	elif event is InputEventJoypadButton or event is InputEventJoypadMotion:
		GameServices.last_input_device = "controller"

	if event.is_action_pressed("ui_cancel"):
		command.emit("cancel", {})
		get_viewport().set_input_as_handled()
		return
	if event.is_action_pressed("ui_confirm"):
		command.emit("confirm", {})
		get_viewport().set_input_as_handled()
		return

	if event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_I:
				command.emit("inventory", {})
			KEY_C:
				command.emit("character", {})
			KEY_ESCAPE:
				command.emit("pause", {})
			KEY_SPACE:
				command.emit("interact", {})
			KEY_F:
				command.emit("ranged_mode", {})
			KEY_Q:
				command.emit("quick_item", {})
			KEY_TAB:
				command.emit("inspect", {})
			KEY_1, KEY_2, KEY_3:
				command.emit("craft_slot", {"slot": event.keycode - KEY_1})
			KEY_KP_1, KEY_Z, KEY_END:
				_emit_move(-1, 1)
			KEY_KP_2, KEY_DOWN, KEY_S:
				_emit_move(0, 1)
			KEY_KP_3:
				_emit_move(1, 1)
			KEY_KP_4, KEY_LEFT, KEY_A:
				_emit_move(-1, 0)
			KEY_KP_6, KEY_RIGHT, KEY_D:
				_emit_move(1, 0)
			KEY_KP_7, KEY_HOME:
				_emit_move(-1, -1)
			KEY_KP_8, KEY_UP, KEY_W:
				_emit_move(0, -1)
			KEY_KP_9, KEY_PAGEUP:
				_emit_move(1, -1)
			KEY_E:
				_emit_move(1, -1)
			KEY_X:
				_emit_move(1, 1)
			_:
				pass

	if event is InputEventJoypadButton and event.pressed:
		match event.button_index:
			JOY_BUTTON_A:
				command.emit("confirm", {})
			JOY_BUTTON_B:
				command.emit("cancel", {})
			JOY_BUTTON_X:
				command.emit("quick_item", {})
			JOY_BUTTON_Y:
				command.emit("inventory", {})
			JOY_BUTTON_LEFT_SHOULDER:
				command.emit("cycle_prev", {})
			JOY_BUTTON_RIGHT_SHOULDER:
				command.emit("cycle_next", {})
			JOY_BUTTON_BACK:
				command.emit("character", {})
			JOY_BUTTON_START:
				command.emit("pause", {})
			JOY_BUTTON_DPAD_UP:
				_emit_move(0, -1)
			JOY_BUTTON_DPAD_DOWN:
				_emit_move(0, 1)
			JOY_BUTTON_DPAD_LEFT:
				_emit_move(-1, 0)
			JOY_BUTTON_DPAD_RIGHT:
				_emit_move(1, 0)

	if event is InputEventJoypadMotion:
		if move_cooldown > 0.0:
			return
		var lx := Input.get_joy_axis(event.device, JOY_AXIS_LEFT_X)
		var ly := Input.get_joy_axis(event.device, JOY_AXIS_LEFT_Y)
		if absf(lx) > 0.55 or absf(ly) > 0.55:
			var dx := 0
			var dy := 0
			if absf(lx) > 0.55:
				dx = 1 if lx > 0.0 else -1
			if absf(ly) > 0.55:
				dy = 1 if ly > 0.0 else -1
			_emit_move(dx, dy)
			move_cooldown = 0.18
		var trigger := Input.get_joy_axis(event.device, JOY_AXIS_TRIGGER_RIGHT)
		if trigger > 0.6 and event.axis == JOY_AXIS_TRIGGER_RIGHT:
			command.emit("ranged_mode", {})
		var lt := Input.get_joy_axis(event.device, JOY_AXIS_TRIGGER_LEFT)
		if lt > 0.6 and event.axis == JOY_AXIS_TRIGGER_LEFT:
			command.emit("inspect", {})
		var rx := Input.get_joy_axis(event.device, JOY_AXIS_RIGHT_X)
		var ry := Input.get_joy_axis(event.device, JOY_AXIS_RIGHT_Y)
		if absf(rx) > 0.35 or absf(ry) > 0.35:
			command.emit("camera_peek", {"dx": rx, "dy": ry})
		elif event.axis == JOY_AXIS_RIGHT_X or event.axis == JOY_AXIS_RIGHT_Y:
			command.emit("camera_peek", {"dx": 0.0, "dy": 0.0})


func _emit_move(dx: int, dy: int) -> void:
	if dx == 0 and dy == 0:
		return
	command.emit("move", {"dx": dx, "dy": dy})
