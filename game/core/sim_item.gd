class_name SimItem
extends RefCounted

const EQUIP_SLOTS := [
	"mainhand", "offhand", "ranged", "ammo", "head", "body", "hands", "feet", "accessory"
]

var instance_id: String = ""
var def_id: String = ""
var name: String = ""
var stack: int = 1
var max_stack: int = 1
var equip_slot: String = "none"
var damage: int = 0
var accuracy: int = 0
var defense: int = 0
var range_tiles: int = 0
var is_ammo: bool = false
var ammo_for: String = ""
var ammo_type: String = ""
var description: String = ""
var affix_id: String = ""
var weight: float = 0.0
var value: int = 0
var category: String = "misc"
var use_effect: String = ""
var quality: int = 0 # crafting quality bonus


func to_dict() -> Dictionary:
	return {
		"instance_id": instance_id,
		"def_id": def_id,
		"name": name,
		"stack": stack,
		"max_stack": max_stack,
		"equip_slot": equip_slot,
		"damage": damage,
		"accuracy": accuracy,
		"defense": defense,
		"range_tiles": range_tiles,
		"is_ammo": is_ammo,
		"ammo_for": ammo_for,
		"ammo_type": ammo_type,
		"description": description,
		"affix_id": affix_id,
		"weight": weight,
		"value": value,
		"category": category,
		"use_effect": use_effect,
		"quality": quality,
	}


static func from_dict(data: Dictionary) -> SimItem:
	var item := SimItem.new()
	item.instance_id = str(data.get("instance_id", ""))
	item.def_id = str(data.get("def_id", ""))
	item.name = str(data.get("name", ""))
	item.stack = int(data.get("stack", 1))
	item.max_stack = int(data.get("max_stack", 1))
	item.equip_slot = _normalize_slot(str(data.get("equip_slot", "none")))
	item.damage = int(data.get("damage", 0))
	item.accuracy = int(data.get("accuracy", 0))
	item.defense = int(data.get("defense", 0))
	item.range_tiles = int(data.get("range_tiles", 0))
	item.is_ammo = bool(data.get("is_ammo", false))
	item.ammo_for = str(data.get("ammo_for", ""))
	item.ammo_type = str(data.get("ammo_type", ""))
	item.description = str(data.get("description", ""))
	item.affix_id = str(data.get("affix_id", ""))
	item.weight = float(data.get("weight", 0.0))
	item.value = int(data.get("value", 0))
	item.category = str(data.get("category", "misc"))
	item.use_effect = str(data.get("use_effect", ""))
	item.quality = int(data.get("quality", 0))
	return item


static func _normalize_slot(slot: String) -> String:
	match slot:
		"weapon":
			return "mainhand"
		"armor":
			return "body"
		_:
			return slot


static func from_definition(def: Dictionary, instance_id: String, affix: Dictionary = {}) -> SimItem:
	var item := SimItem.new()
	item.instance_id = instance_id
	item.def_id = str(def.get("id", ""))
	item.name = str(def.get("name", item.def_id))
	item.stack = 1
	item.max_stack = int(def.get("max_stack", 1))
	item.equip_slot = _normalize_slot(str(def.get("equip_slot", "none")))
	item.damage = int(def.get("damage", 0))
	item.accuracy = int(def.get("accuracy", 0))
	item.defense = int(def.get("defense", 0))
	item.range_tiles = int(def.get("range_tiles", 0))
	item.is_ammo = bool(def.get("is_ammo", false))
	item.ammo_for = str(def.get("ammo_for", ""))
	item.ammo_type = str(def.get("ammo_type", def.get("ammo_for", "")))
	item.description = str(def.get("description", ""))
	item.weight = float(def.get("weight", 1.0))
	item.value = int(def.get("value", 1))
	item.category = str(def.get("category", "misc"))
	item.use_effect = str(def.get("use_effect", ""))
	if not affix.is_empty():
		item.affix_id = str(affix.get("id", ""))
		item.name = "%s %s" % [affix.get("name_prefix", ""), item.name]
		item.damage += int(affix.get("damage", 0))
		item.accuracy += int(affix.get("accuracy", 0))
		item.defense += int(affix.get("defense", 0))
		item.value += int(affix.get("value", 0))
		item.description += " " + str(affix.get("description", ""))
	return item


func total_weight() -> float:
	return weight * float(stack)


func compare_summary(other: SimItem) -> String:
	if other == null:
		return "No item equipped"
	var parts: Array[String] = []
	var dd := damage - other.damage
	var da := accuracy - other.accuracy
	var df := defense - other.defense
	if dd != 0:
		parts.append("DMG %+d" % dd)
	if da != 0:
		parts.append("ACC %+d" % da)
	if df != 0:
		parts.append("DEF %+d" % df)
	var dw := weight - other.weight
	if absf(dw) > 0.01:
		parts.append("WT %+.1f" % dw)
	return ", ".join(parts) if not parts.is_empty() else "Similar stats"
