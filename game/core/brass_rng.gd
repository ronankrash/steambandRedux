class_name BrassRng
extends RefCounted
## Deterministic xorshift64* RNG. Seeded once for reproducible runs.

var _state: int = 1


func reseed(value: int) -> void:
	if value == 0:
		value = 1
	_state = value & 0x7FFFFFFFFFFFFFFF
	if _state == 0:
		_state = 1


func get_state() -> int:
	return _state


func set_state(value: int) -> void:
	_state = value if value != 0 else 1


func next_u64() -> int:
	var x: int = _state
	x ^= (x << 13) & 0x7FFFFFFFFFFFFFFF
	x ^= (x >> 7)
	x ^= (x << 17) & 0x7FFFFFFFFFFFFFFF
	_state = x if x != 0 else 1
	return _state


func randi_range(min_inclusive: int, max_inclusive: int) -> int:
	if max_inclusive <= min_inclusive:
		return min_inclusive
	var span: int = max_inclusive - min_inclusive + 1
	return min_inclusive + int(next_u64() % span)


func randf() -> float:
	return float(next_u64() % 1000000) / 1000000.0


func chance(probability: float) -> bool:
	return randf() < clampf(probability, 0.0, 1.0)
