class_name SaveSystem
extends RefCounted

const SCHEMA_VERSION := 2


static func serialize(sim: GameSim) -> Dictionary:
	var actors: Array = []
	for actor in sim.actors:
		actors.append((actor as SimActor).to_dict())
	var storage: Array = []
	for entry in sim.storage:
		storage.append((entry as SimItem).to_dict())
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
		"depth": sim.depth,
		"max_depth_reached": sim.max_depth_reached,
		"expedition_active": sim.expedition_active,
		"merchants": sim.merchants.duplicate(true),
		"storage": storage,
		"settings": sim.settings.duplicate(true),
		"progression_model": "save_reload_hub",
	}


static func deserialize(data: Dictionary, content: ContentDB) -> Dictionary:
	var version := int(data.get("schema_version", -1))
	if version == 1:
		return _migrate_v1(data, content)
	if version != SCHEMA_VERSION:
		return {
			"ok": false,
			"reason": "unsupported_schema",
			"version": version,
			"message": "Save schema %d is not supported. Current schema is %d." % [version, SCHEMA_VERSION],
		}
	if not data.has("world") or not data.has("actors"):
		return {"ok": false, "reason": "invalid_save", "message": "Save file is missing required fields."}
	var sim := _sim_from_common(data, content)
	sim.depth = int(data.get("depth", 0))
	sim.max_depth_reached = int(data.get("max_depth_reached", sim.depth))
	sim.expedition_active = bool(data.get("expedition_active", false))
	sim.merchants = data.get("merchants", {}).duplicate(true)
	if sim.merchants.is_empty():
		for mid in content.merchants.keys():
			sim.merchants[mid] = EconomySystem.build_merchant_state(content.merchants[mid])
	sim.storage.clear()
	for entry in data.get("storage", []):
		sim.storage.append(SimItem.from_dict(entry))
	sim.settings = data.get("settings", sim.settings).duplicate(true)
	return {"ok": true, "sim": sim, "migrated": false}


static func _migrate_v1(data: Dictionary, content: ContentDB) -> Dictionary:
	if not data.has("world") or not data.has("actors"):
		return {"ok": false, "reason": "invalid_save", "message": "Legacy save is corrupt."}
	var sim := _sim_from_common(data, content)
	sim.depth = 1 if not bool(data.get("world", {}).get("is_town", false)) else 0
	sim.max_depth_reached = maxi(1, sim.depth)
	sim.expedition_active = sim.depth > 0
	for mid in content.merchants.keys():
		sim.merchants[mid] = EconomySystem.build_merchant_state(content.merchants[mid])
	sim.storage.clear()
	# Ensure equipment slots migrated via SimActor.from_dict
	return {
		"ok": true,
		"sim": sim,
		"migrated": true,
		"message": "Migrated save from schema 1 to schema 2.",
	}


static func _sim_from_common(data: Dictionary, content: ContentDB) -> GameSim:
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
	return sim


static func save_to_file(path: String, sim: GameSim) -> bool:
	var payload := JSON.stringify(serialize(sim), "\t")
	var f := FileAccess.open(path, FileAccess.WRITE)
	if f == null:
		return false
	f.store_string(payload)
	return true


static func load_from_file(path: String, content: ContentDB) -> Dictionary:
	if not FileAccess.file_exists(path):
		return {"ok": false, "reason": "missing", "message": "No save file found."}
	var f := FileAccess.open(path, FileAccess.READ)
	if f == null:
		return {"ok": false, "reason": "unreadable", "message": "Save file unreadable."}
	var parsed = JSON.parse_string(f.get_as_text())
	if typeof(parsed) != TYPE_DICTIONARY:
		return {"ok": false, "reason": "corrupt", "message": "Save file is corrupt."}
	return deserialize(parsed, content)
