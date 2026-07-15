class_name SimItem
extends RefCounted

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
var description: String = ""
var affix_id: String = ""


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
		"description": description,
		"affix_id": affix_id,
	}


static func from_dict(data: Dictionary) -> SimItem:
	var item := SimItem.new()
	item.instance_id = str(data.get("instance_id", ""))
	item.def_id = str(data.get("def_id", ""))
	item.name = str(data.get("name", ""))
	item.stack = int(data.get("stack", 1))
	item.max_stack = int(data.get("max_stack", 1))
	item.equip_slot = str(data.get("equip_slot", "none"))
	item.damage = int(data.get("damage", 0))
	item.accuracy = int(data.get("accuracy", 0))
	item.defense = int(data.get("defense", 0))
	item.range_tiles = int(data.get("range_tiles", 0))
	item.is_ammo = bool(data.get("is_ammo", false))
	item.ammo_for = str(data.get("ammo_for", ""))
	item.description = str(data.get("description", ""))
	item.affix_id = str(data.get("affix_id", ""))
	return item


static func from_definition(def: Dictionary, instance_id: String, affix: Dictionary = {}) -> SimItem:
	var item := SimItem.new()
	item.instance_id = instance_id
	item.def_id = str(def.get("id", ""))
	item.name = str(def.get("name", item.def_id))
	item.stack = 1
	item.max_stack = int(def.get("max_stack", 1))
	item.equip_slot = str(def.get("equip_slot", "none"))
	item.damage = int(def.get("damage", 0))
	item.accuracy = int(def.get("accuracy", 0))
	item.defense = int(def.get("defense", 0))
	item.range_tiles = int(def.get("range_tiles", 0))
	item.is_ammo = bool(def.get("is_ammo", false))
	item.ammo_for = str(def.get("ammo_for", ""))
	item.description = str(def.get("description", ""))
	if not affix.is_empty():
		item.affix_id = str(affix.get("id", ""))
		item.name = "%s %s" % [affix.get("name_prefix", ""), item.name]
		item.damage += int(affix.get("damage", 0))
		item.accuracy += int(affix.get("accuracy", 0))
		item.defense += int(affix.get("defense", 0))
		item.description += " " + str(affix.get("description", ""))
	return item
