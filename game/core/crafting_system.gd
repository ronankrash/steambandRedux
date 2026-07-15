class_name CraftingSystem
extends RefCounted


static func can_craft(actor: SimActor, recipe: Dictionary) -> bool:
	if int(actor.engineering) < int(recipe.get("engineering_required", 0)):
		return false
	for ingredient in recipe.get("ingredients", []):
		var need := int(ingredient.get("count", 1))
		var id := str(ingredient.get("item_id", ""))
		if actor.count_item(id) < need:
			return false
	return true


static func craft(actor: SimActor, recipe: Dictionary, content: ContentDB, instance_id: String) -> Dictionary:
	if not can_craft(actor, recipe):
		return {"ok": false, "reason": "requirements"}
	for ingredient in recipe.get("ingredients", []):
		InventorySystem.remove_item_count(actor, str(ingredient.get("item_id", "")), int(ingredient.get("count", 1)))
	var result_def := content.get_item(str(recipe.get("result_id", "")))
	if result_def.is_empty():
		return {"ok": false, "reason": "missing_result"}
	var crafted := SimItem.from_definition(result_def, instance_id)
	if not InventorySystem.add_item(actor, crafted):
		return {"ok": true, "dropped": true, "item": crafted}
	return {"ok": true, "dropped": false, "item": crafted}
