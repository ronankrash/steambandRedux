class_name SaveSystem
extends RefCounted

const SCHEMA_VERSION := 1


static func serialize(sim: GameSim) -> Dictionary:
	var actors: Array = []
	for actor in sim.actors:
		actors.append((actor as SimActor).to_dict())
	return {
		"schema_version": SCHEMA_VERSION,
		"seed": sim.seed_value,
		"turn": sim.turn_index,
		"log": sim.combat_log.duplicate(),
		"rng_state": sim.rng.get_state(),
		"world": sim.world.to_dict(),
		"actors": actors,
		"next_instance": sim.next_instance,
		"player_id": sim.player_id,
		"dead": sim.player_dead,
		"death_summary": sim.death_summary,
	}


static func deserialize(data: Dictionary, content: ContentDB) -> Dictionary:
	var version := int(data.get("schema_version", -1))
	if version != SCHEMA_VERSION:
		return {"ok": false, "reason": "unsupported_schema", "version": version}
	if not data.has("world") or not data.has("actors"):
		return {"ok": false, "reason": "invalid_save"}
	var sim := GameSim.new()
	sim.content = content
	sim.seed_value = int(data.get("seed", 1))
	sim.rng = BrassRng.new()
	sim.rng.set_state(int(data.get("rng_state", 1)))
	sim.turn_index = int(data.get("turn", 0))
	sim.combat_log.clear()
	for entry in data.get("log", []):
		sim.combat_log.append(str(entry))
	sim.world = SimWorld.new()
	sim.world.from_dict(data.get("world", {}))
	sim.actors.clear()
	for entry in data.get("actors", []):
		sim.actors.append(SimActor.from_dict(entry))
	sim.next_instance = int(data.get("next_instance", 1))
	sim.player_id = str(data.get("player_id", "player"))
	sim.player_dead = bool(data.get("dead", false))
	sim.death_summary = str(data.get("death_summary", ""))
	return {"ok": true, "sim": sim}


static func save_to_file(path: String, sim: GameSim) -> bool:
	var payload := JSON.stringify(serialize(sim), "\t")
	var f := FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return false
	f.store_string(payload)
	return true


static func load_from_file(path: String, content: ContentDB) -> Dictionary:
	if not FileAccess.file_exists(path):
		return {"ok": false, "reason": "missing"}
	var f := FileAccess.open(path, FileAccess.READ)
	if f == null:
		return {"ok": false, "reason": "unreadable"}
	var parsed = JSON.parse_string(f.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		return {"ok": false, "reason": "corrupt"}
	return deserialize(parsed, content)
