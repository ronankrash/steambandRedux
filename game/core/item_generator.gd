class_name ItemGenerator
extends RefCounted
## Deterministic affix rolls for a supplied BrassRng state.


static func roll_affix(rng: BrassRng, content: ContentDB, item_def: Dictionary, chance: int = 35) -> Dictionary:
	var allowed: Array = item_def.get("allowed_affixes", [])
	if allowed.is_empty():
		return {}
	if rng.randi_range(1, 100) > chance:
		return {}
	var pick := str(allowed[rng.randi_range(0, allowed.size() - 1)])
	return content.get_affix(pick)


static func create_item(rng: BrassRng, content: ContentDB, def_id: String, instance_id: String, force_affix_chance: int = 35) -> SimItem:
	var def := content.get_item(def_id)
	if def.is_empty():
		return null
	var affix := roll_affix(rng, content, def, force_affix_chance)
	return SimItem.from_definition(def, instance_id, affix)
