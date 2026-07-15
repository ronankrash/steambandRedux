class_name SimActor
extends RefCounted

var id: String = ""
var kind: String = "monster" # player | monster
var name: String = ""
var pos: GridPos = GridPos.new()
var hp: int = 10
var max_hp: int = 10
var melee_accuracy: int = 50
var melee_damage: int = 2
var ranged_accuracy: int = 40
var ranged_damage: int = 2
var defense: int = 0
var engineering: int = 0
var skill_points: int = 0
var alive: bool = true
var status_effects: Dictionary = {} # id -> turns remaining
var inventory: Array = [] # Array[SimItem]
var equipment: Dictionary = {"weapon": null, "armor": null, "offhand": null}
var quick_item_id: String = ""
var behavior: String = "melee_pursuer"
var vision_range: int = 8
var race_id: String = ""
var class_id: String = ""


func is_player() -> bool:
	return kind == "player"


func get_weapon() -> SimItem:
	return equipment.get("weapon")


func get_armor() -> SimItem:
	return equipment.get("armor")


func total_melee_damage() -> int:
	var w := get_weapon()
	var bonus := w.damage if w != null else 0
	return melee_damage + bonus


func total_melee_accuracy() -> int:
	var w := get_weapon()
	var bonus := w.accuracy if w != null else 0
	return melee_accuracy + bonus


func total_defense() -> int:
	var a := get_armor()
	var bonus := a.defense if a != null else 0
	return defense + bonus


func total_ranged_damage() -> int:
	var w := get_weapon()
	if w != null and w.range_tiles > 0:
		return ranged_damage + w.damage
	return ranged_damage


func total_ranged_accuracy() -> int:
	var w := get_weapon()
	if w != null and w.range_tiles > 0:
		return ranged_accuracy + w.accuracy
	return ranged_accuracy


func weapon_range() -> int:
	var w := get_weapon()
	if w != null and w.range_tiles > 0:
		return w.range_tiles
	return 0


func add_status(effect_id: String, turns: int) -> void:
	status_effects[effect_id] = maxi(int(status_effects.get(effect_id, 0)), turns)


func tick_statuses() -> Array[String]:
	var expired: Array[String] = []
	var keys := status_effects.keys()
	for key in keys:
		status_effects[key] = int(status_effects[key]) - 1
		if int(status_effects[key]) <= 0:
			status_effects.erase(key)
			expired.append(str(key))
	return expired


func find_inventory_item(def_id: String) -> SimItem:
	for entry in inventory:
		var item: SimItem = entry
		if item.def_id == def_id and item.stack > 0:
			return item
	return null


func count_item(def_id: String) -> int:
	var total := 0
	for entry in inventory:
		var item: SimItem = entry
		if item.def_id == def_id:
			total += item.stack
	return total


func to_dict() -> Dictionary:
	var inv: Array = []
	for entry in inventory:
		inv.append((entry as SimItem).to_dict())
	var eq: Dictionary = {}
	for slot in equipment.keys():
		var e = equipment[slot]
		eq[slot] = e.to_dict() if e != null else null
	return {
		"id": id,
		"kind": kind,
		"name": name,
		"pos": pos.to_dict(),
		"hp": hp,
		"max_hp": max_hp,
		"melee_accuracy": melee_accuracy,
		"melee_damage": melee_damage,
		"ranged_accuracy": ranged_accuracy,
		"ranged_damage": ranged_damage,
		"defense": defense,
		"engineering": engineering,
		"skill_points": skill_points,
		"alive": alive,
		"status_effects": status_effects.duplicate(true),
		"inventory": inv,
		"equipment": eq,
		"quick_item_id": quick_item_id,
		"behavior": behavior,
		"vision_range": vision_range,
		"race_id": race_id,
		"class_id": class_id,
	}


static func from_dict(data: Dictionary) -> SimActor:
	var actor := SimActor.new()
	actor.id = str(data.get("id", ""))
	actor.kind = str(data.get("kind", "monster"))
	actor.name = str(data.get("name", ""))
	actor.pos = GridPos.from_dict(data.get("pos", {}))
	actor.hp = int(data.get("hp", 10))
	actor.max_hp = int(data.get("max_hp", 10))
	actor.melee_accuracy = int(data.get("melee_accuracy", 50))
	actor.melee_damage = int(data.get("melee_damage", 2))
	actor.ranged_accuracy = int(data.get("ranged_accuracy", 40))
	actor.ranged_damage = int(data.get("ranged_damage", 2))
	actor.defense = int(data.get("defense", 0))
	actor.engineering = int(data.get("engineering", 0))
	actor.skill_points = int(data.get("skill_points", 0))
	actor.alive = bool(data.get("alive", true))
	actor.status_effects = data.get("status_effects", {}).duplicate(true)
	actor.quick_item_id = str(data.get("quick_item_id", ""))
	actor.behavior = str(data.get("behavior", "melee_pursuer"))
	actor.vision_range = int(data.get("vision_range", 8))
	actor.race_id = str(data.get("race_id", ""))
	actor.class_id = str(data.get("class_id", ""))
	actor.inventory.clear()
	for entry in data.get("inventory", []):
		actor.inventory.append(SimItem.from_dict(entry))
	actor.equipment = {"weapon": null, "armor": null, "offhand": null}
	var eq: Dictionary = data.get("equipment", {})
	for slot in actor.equipment.keys():
		var raw = eq.get(slot, null)
		actor.equipment[slot] = SimItem.from_dict(raw) if typeof(raw) == TYPE_DICTIONARY else null
	return actor
