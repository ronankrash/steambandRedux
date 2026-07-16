class_name CraftingSystem
extends RefCounted


static func missing_ingredients(actor: SimActor, recipe: Dictionary) -> Array:
	var missing: Array = []
	for ingredient in recipe.get("ingredients", []):
		var need := int(ingredient.get("count", 1))
		var id := str(ingredient.get("item_id", ""))
		var have := actor.count_item(id)
		if have < need:
			missing.append({"item_id": id, "need": need, "have": have})
	return missing


static func can_craft(actor: SimActor, recipe: Dictionary) -> bool:
	return explain_blocked(actor, recipe) == ""


static func explain_blocked(actor: SimActor, recipe: Dictionary) -> String:
	var skill_id := str(recipe.get("skill_required", ""))
	var need_rank := int(recipe.get("skill_rank", 0))
	if skill_id != "" and SkillSystem.rank(actor, skill_id) < need_rank:
		return "Requires %s rank %d." % [skill_id, need_rank]
	if int(actor.engineering) < int(recipe.get("engineering_required", 0)):
		return "Requires engineering %d." % int(recipe.get("engineering_required", 0))
	var missing := missing_ingredients(actor, recipe)
	if not missing.is_empty():
		var m: Dictionary = missing[0]
		return "Missing %s (%d/%d)." % [m["item_id"], m["have"], m["need"]]
	return ""


static func craft(actor: SimActor, recipe: Dictionary, content: ContentDB, instance_id: String) -> Dictionary:
	var blocked := explain_blocked(actor, recipe)
	if blocked != "":
		return {"ok": false, "reason": blocked}
	for ingredient in recipe.get("ingredients", []):
		InventorySystem.remove_item_count(actor, str(ingredient.get("item_id", "")), int(ingredient.get("count", 1)))
	var result_def := content.get_item(str(recipe.get("result_id", "")))
	if result_def.is_empty():
		return {"ok": false, "reason": "missing_result"}
	var crafted := SimItem.from_definition(result_def, instance_id)
	# Crafting skill quality
	var skill_id := str(recipe.get("skill_required", "skill_engineering"))
	var rank := SkillSystem.rank(actor, skill_id) + SkillSystem.rank(actor, "skill_smithing")
	crafted.quality = rank
	if actor.has_ability("craft_quality") or actor.has_ability("steam_tinker"):
		crafted.damage += 1 if crafted.damage > 0 else 0
		crafted.defense += 1 if crafted.defense > 0 or crafted.equip_slot in ["head", "body", "hands", "feet"] else 0
		crafted.name = "Fine " + crafted.name
		crafted.value += 5
	if not InventorySystem.add_item(actor, crafted):
		return {"ok": true, "dropped": true, "item": crafted}
	return {"ok": true, "dropped": false, "item": crafted}


static func recipes_for_station(content: ContentDB, station: String) -> Array:
	var out: Array = []
	for id in content.recipes.keys():
		var recipe: Dictionary = content.recipes[id]
		if str(recipe.get("station", "workbench")) == station:
			out.append(recipe)
	out.sort_custom(func(a, b): return str(a.get("name", "")) < str(b.get("name", "")))
	return out
