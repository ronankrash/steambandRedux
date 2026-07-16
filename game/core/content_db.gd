class_name ContentDB
extends RefCounted
## Loads data-driven content by stable string IDs. No array-index identities.

const CONTENT_ROOT := "res://content"
const VALID_SLOTS := [
	"mainhand", "offhand", "ranged", "ammo", "head", "body", "hands", "feet", "accessory", "none", ""
]
const VALID_ABILITIES := [
	"melee", "ranged", "poison_spit", "steam_burst", "fear", "repair", "summon", "suppress", "aoe"
]
const VALID_STATIONS := ["workbench", "forge", "alchemy"]

var races: Dictionary = {}
var classes: Dictionary = {}
var skills: Dictionary = {}
var monsters: Dictionary = {}
var items: Dictionary = {}
var recipes: Dictionary = {}
var affixes: Dictionary = {}
var merchants: Dictionary = {}
var environments: Dictionary = {}
var errors: Array[String] = []


func load_all() -> bool:
	errors.clear()
	races = _load_folder("races")
	classes = _load_folder("classes")
	skills = _load_folder("skills")
	monsters = _load_folder("monsters")
	items = _load_folder("items")
	recipes = _load_folder("recipes")
	affixes = _load_folder("affixes")
	merchants = _load_folder("merchants")
	environments = _load_folder("environments")
	_validate()
	return errors.is_empty()


func get_race(id: String) -> Dictionary:
	return races.get(id, {})


func get_class_def(id: String) -> Dictionary:
	return classes.get(id, {})


func get_skill(id: String) -> Dictionary:
	return skills.get(id, {})


func get_monster(id: String) -> Dictionary:
	return monsters.get(id, {})


func get_item(id: String) -> Dictionary:
	return items.get(id, {})


func get_recipe(id: String) -> Dictionary:
	return recipes.get(id, {})


func get_affix(id: String) -> Dictionary:
	return affixes.get(id, {})


func get_merchant(id: String) -> Dictionary:
	return merchants.get(id, {})


func get_environment(id: String) -> Dictionary:
	return environments.get(id, {})


func _load_folder(folder: String) -> Dictionary:
	var out: Dictionary = {}
	var path: String = "%s/%s" % [CONTENT_ROOT, folder]
	var dir := DirAccess.open(path)
	if dir == null:
		errors.append("Missing content folder: %s" % path)
		return out
	dir.list_dir_begin()
	var file_name := dir.get_next()
	while file_name != "":
		if not dir.current_is_dir() and file_name.ends_with(".json"):
			var full := "%s/%s" % [path, file_name]
			var parsed := _load_json(full)
			if parsed.is_empty():
				errors.append("Failed to parse %s" % full)
			else:
				var id: String = str(parsed.get("id", ""))
				if id.is_empty():
					errors.append("Missing id in %s" % full)
				elif out.has(id):
					errors.append("Duplicate id '%s' in %s" % [id, folder])
				else:
					out[id] = parsed
		file_name = dir.get_next()
	dir.list_dir_end()
	return out


func _load_json(path: String) -> Dictionary:
	var f := FileAccess.open(path, FileAccess.READ)
	if f == null:
		return {}
	var text := f.get_as_text()
	var data = JSON.parse_string(text)
	if typeof(data) != TYPE_DICTIONARY:
		return {}
	return data


func _validate() -> void:
	if races.size() < 6:
		errors.append("Need at least 6 races, found %d" % races.size())
	if classes.size() < 6:
		errors.append("Need at least 6 classes, found %d" % classes.size())
	if monsters.size() < 12:
		errors.append("Need at least 12 monsters, found %d" % monsters.size())
	if recipes.size() < 20:
		errors.append("Need at least 20 recipes, found %d" % recipes.size())
	if skills.size() < 8:
		errors.append("Need at least 8 skills, found %d" % skills.size())
	if merchants.size() < 3:
		errors.append("Need at least 3 merchants, found %d" % merchants.size())

	var ability_count := 0
	for sid in skills.keys():
		ability_count += (skills[sid].get("abilities", []) as Array).size()
	if ability_count < 12:
		errors.append("Need at least 12 skill abilities, found %d" % ability_count)

	for id in items.keys():
		var item: Dictionary = items[id]
		var slot: String = str(item.get("equip_slot", ""))
		if slot not in VALID_SLOTS:
			errors.append("Item '%s' has invalid equip_slot '%s'" % [id, slot])
		for field in ["name", "description"]:
			if not item.has(field):
				errors.append("Item '%s' missing %s" % [id, field])
		for affix_id in item.get("allowed_affixes", []):
			if not affixes.has(str(affix_id)):
				errors.append("Item '%s' references missing affix '%s'" % [id, affix_id])

	for id in recipes.keys():
		var recipe: Dictionary = recipes[id]
		var station := str(recipe.get("station", "workbench"))
		if station not in VALID_STATIONS:
			errors.append("Recipe '%s' has invalid station '%s'" % [id, station])
		for ingredient in recipe.get("ingredients", []):
			var iid: String = str(ingredient.get("item_id", ""))
			if not items.has(iid):
				errors.append("Recipe '%s' has invalid ingredient '%s'" % [id, iid])
		var result_id: String = str(recipe.get("result_id", ""))
		if not items.has(result_id):
			errors.append("Recipe '%s' has invalid result '%s'" % [id, result_id])
		var sid := str(recipe.get("skill_required", ""))
		if sid != "" and not skills.has(sid):
			errors.append("Recipe '%s' references missing skill '%s'" % [id, sid])

	var boss_count := 0
	for id in monsters.keys():
		var mon: Dictionary = monsters[id]
		for ability in mon.get("abilities", []):
			if str(ability) not in VALID_ABILITIES:
				errors.append("Monster '%s' references invalid ability '%s'" % [id, ability])
		var drop: String = str(mon.get("drop_item_id", ""))
		if drop != "" and not items.has(drop):
			errors.append("Monster '%s' references missing drop '%s'" % [id, drop])
		if bool(mon.get("is_boss", false)) or str(mon.get("behavior", "")) == "boss":
			boss_count += 1
	if boss_count < 1:
		errors.append("Need at least one boss monster")

	for id in merchants.keys():
		var mer: Dictionary = merchants[id]
		for entry in mer.get("inventory", []):
			var iid := str(entry.get("item_id", ""))
			if not items.has(iid):
				errors.append("Merchant '%s' sells missing item '%s'" % [id, iid])
