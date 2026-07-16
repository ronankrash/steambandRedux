class_name SimActor
extends RefCounted

const DEFAULT_SLOTS := {
	"mainhand": null,
	"offhand": null,
	"ranged": null,
	"ammo": null,
	"head": null,
	"body": null,
	"hands": null,
	"feet": null,
	"accessory": null,
}

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
var marksmanship: int = 0
var smithing: int = 0
var medicine: int = 0
var stealth: int = 0
var occult: int = 0
var skill_points: int = 0
var skill_ranks: Dictionary = {} # skill_id -> rank
var unlocked_abilities: Array = [] # ability ids
var resistances: Dictionary = {}
var gold: int = 0
var xp: int = 0
var level: int = 1
var encumbrance_cap: float = 40.0
var alive: bool = true
var status_effects: Dictionary = {} # id -> turns remaining
var inventory: Array = [] # Array[SimItem]
var equipment: Dictionary = DEFAULT_SLOTS.duplicate()
var quick_slots: Array = ["", "", ""] # up to 3 quick item def ids
var quick_item_id: String = ""
var behavior: String = "melee_pursuer"
var vision_range: int = 8
var hear_radius: int = 6
var race_id: String = ""
var class_id: String = ""
var ai_state: String = "idle"
var alert_turns: int = 0
var is_elite: bool = false
var is_boss: bool = false
var monster_def_id: String = ""
var xp_value: int = 0


func is_player() -> bool:
	return kind == "player"


func get_equipped(slot: String) -> SimItem:
	return equipment.get(slot)


func get_weapon() -> SimItem:
	var m: SimItem = equipment.get("mainhand")
	if m != null:
		return m
	return equipment.get("ranged")


func get_ranged_weapon() -> SimItem:
	return equipment.get("ranged")


func get_armor() -> SimItem:
	return equipment.get("body")


func _equip_bonus(stat: String) -> int:
	var total := 0
	for slot in equipment.keys():
		var item: SimItem = equipment[slot]
		if item == null:
			continue
		match stat:
			"damage":
				total += item.damage
			"accuracy":
				total += item.accuracy
			"defense":
				total += item.defense
	return total


func skill_bonus(stat: String) -> int:
	var total := 0
	for ability_id in unlocked_abilities:
		# Effects applied via SkillSystem cached on actor stats when ranks change;
		# additional flat bonuses stored in skill_ranks-derived helpers.
		pass
	# Rank-derived soft bonuses
	match stat:
		"melee_accuracy":
			total += int(skill_ranks.get("skill_melee", 0)) * 2
		"melee_damage":
			total += int(skill_ranks.get("skill_melee", 0))
		"ranged_accuracy":
			total += int(skill_ranks.get("skill_marksmanship", 0)) * 2
		"ranged_damage":
			total += int(skill_ranks.get("skill_marksmanship", 0))
		"defense":
			total += int(skill_ranks.get("skill_defense", 0))
		"engineering":
			total += int(skill_ranks.get("skill_engineering", 0))
		"crit_chance":
			total += int(skill_ranks.get("skill_melee", 0)) + int(skill_ranks.get("skill_marksmanship", 0))
	return total


func ability_bonus(stat: String) -> int:
	var total := 0
	# Ability effect values are looked up when SkillSystem unlocks; stored as status-like keys
	for key in status_effects.keys():
		if str(key).begins_with("abilstat_" + stat):
			total += int(status_effects[key])
	return total


func has_ability(ability_id: String) -> bool:
	return ability_id in unlocked_abilities


func total_melee_damage() -> int:
	var w: SimItem = equipment.get("mainhand")
	var bonus := w.damage if w != null else 0
	return melee_damage + bonus + skill_bonus("melee_damage") + ability_bonus("melee_damage")


func total_melee_accuracy() -> int:
	var w: SimItem = equipment.get("mainhand")
	var bonus := w.accuracy if w != null else 0
	return melee_accuracy + bonus + skill_bonus("melee_accuracy") + ability_bonus("melee_accuracy")


func total_defense() -> int:
	return defense + _equip_bonus("defense") + skill_bonus("defense") + ability_bonus("defense")


func total_ranged_damage() -> int:
	var w := get_ranged_weapon()
	var bonus := w.damage if w != null else 0
	return ranged_damage + bonus + skill_bonus("ranged_damage") + ability_bonus("ranged_damage")


func total_ranged_accuracy() -> int:
	var w := get_ranged_weapon()
	var bonus := w.accuracy if w != null else 0
	return ranged_accuracy + bonus + skill_bonus("ranged_accuracy") + ability_bonus("ranged_accuracy")


func weapon_range() -> int:
	var w := get_ranged_weapon()
	if w != null and w.range_tiles > 0:
		return w.range_tiles
	return 0


func ammo_type_needed() -> String:
	var w := get_ranged_weapon()
	if w == null:
		return ""
	return w.ammo_type if w.ammo_type != "" else w.ammo_for


func find_ammo() -> SimItem:
	var needed := ammo_type_needed()
	var equipped_ammo: SimItem = equipment.get("ammo")
	if equipped_ammo != null and equipped_ammo.stack > 0:
		if needed == "" or equipped_ammo.ammo_type == needed or equipped_ammo.ammo_for == needed or equipped_ammo.def_id.contains(needed):
			return equipped_ammo
	for entry in inventory:
		var item: SimItem = entry
		if not item.is_ammo:
			continue
		if needed == "" or item.ammo_type == needed or item.ammo_for == needed or item.def_id.contains(needed):
			return item
	return null


func carry_weight() -> float:
	var total := 0.0
	for entry in inventory:
		total += (entry as SimItem).total_weight()
	for slot in equipment.keys():
		var item: SimItem = equipment[slot]
		if item != null:
			total += item.total_weight()
	return total


func is_encumbered() -> bool:
	return carry_weight() > encumbrance_cap


func resistance(effect_id: String) -> int:
	return int(resistances.get(effect_id, 0))


func add_status(effect_id: String, turns: int) -> void:
	var resist := resistance(effect_id)
	if resist >= 100:
		return
	if resist > 0 and turns > 1:
		turns = maxi(1, turns - int(turns * resist / 100.0))
	status_effects[effect_id] = maxi(int(status_effects.get(effect_id, 0)), turns)


func tick_statuses() -> Array[String]:
	var expired: Array[String] = []
	var keys := status_effects.keys()
	for key in keys:
		if str(key).begins_with("abilstat_"):
			continue
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
		"marksmanship": marksmanship,
		"smithing": smithing,
		"medicine": medicine,
		"stealth": stealth,
		"occult": occult,
		"skill_points": skill_points,
		"skill_ranks": skill_ranks.duplicate(true),
		"unlocked_abilities": unlocked_abilities.duplicate(),
		"resistances": resistances.duplicate(true),
		"gold": gold,
		"xp": xp,
		"level": level,
		"encumbrance_cap": encumbrance_cap,
		"alive": alive,
		"status_effects": status_effects.duplicate(true),
		"inventory": inv,
		"equipment": eq,
		"quick_slots": quick_slots.duplicate(),
		"quick_item_id": quick_item_id,
		"behavior": behavior,
		"vision_range": vision_range,
		"hear_radius": hear_radius,
		"race_id": race_id,
		"class_id": class_id,
		"ai_state": ai_state,
		"alert_turns": alert_turns,
		"is_elite": is_elite,
		"is_boss": is_boss,
		"monster_def_id": monster_def_id,
		"xp_value": xp_value,
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
	actor.marksmanship = int(data.get("marksmanship", 0))
	actor.smithing = int(data.get("smithing", 0))
	actor.medicine = int(data.get("medicine", 0))
	actor.stealth = int(data.get("stealth", 0))
	actor.occult = int(data.get("occult", 0))
	actor.skill_points = int(data.get("skill_points", 0))
	actor.skill_ranks = data.get("skill_ranks", {}).duplicate(true)
	actor.unlocked_abilities = data.get("unlocked_abilities", []).duplicate()
	actor.resistances = data.get("resistances", {}).duplicate(true)
	actor.gold = int(data.get("gold", 0))
	actor.xp = int(data.get("xp", 0))
	actor.level = int(data.get("level", 1))
	actor.encumbrance_cap = float(data.get("encumbrance_cap", 40.0))
	actor.alive = bool(data.get("alive", true))
	actor.status_effects = data.get("status_effects", {}).duplicate(true)
	actor.quick_slots = data.get("quick_slots", ["", "", ""]).duplicate()
	actor.quick_item_id = str(data.get("quick_item_id", ""))
	actor.behavior = str(data.get("behavior", "melee_pursuer"))
	actor.vision_range = int(data.get("vision_range", 8))
	actor.hear_radius = int(data.get("hear_radius", 6))
	actor.race_id = str(data.get("race_id", ""))
	actor.class_id = str(data.get("class_id", ""))
	actor.ai_state = str(data.get("ai_state", "idle"))
	actor.alert_turns = int(data.get("alert_turns", 0))
	actor.is_elite = bool(data.get("is_elite", false))
	actor.is_boss = bool(data.get("is_boss", false))
	actor.monster_def_id = str(data.get("monster_def_id", ""))
	actor.xp_value = int(data.get("xp_value", 0))
	actor.inventory.clear()
	for entry in data.get("inventory", []):
		actor.inventory.append(SimItem.from_dict(entry))
	actor.equipment = DEFAULT_SLOTS.duplicate()
	var eq: Dictionary = data.get("equipment", {})
	# Migrate v1 slots
	if eq.has("weapon") and not eq.has("mainhand"):
		eq["mainhand"] = eq["weapon"]
	if eq.has("armor") and not eq.has("body"):
		eq["body"] = eq["armor"]
	for slot in actor.equipment.keys():
		var raw = eq.get(slot, null)
		actor.equipment[slot] = SimItem.from_dict(raw) if typeof(raw) == TYPE_DICTIONARY else null
	return actor
