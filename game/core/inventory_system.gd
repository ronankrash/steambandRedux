class_name InventorySystem
extends RefCounted


static func add_item(actor: SimActor, item: SimItem) -> bool:
	if item.max_stack > 1:
		for entry in actor.inventory:
			var existing: SimItem = entry
			if existing.def_id == item.def_id and existing.affix_id == item.affix_id and existing.stack < existing.max_stack:
				var space: int = existing.max_stack - existing.stack
				var moved: int = mini(space, item.stack)
				existing.stack += moved
				item.stack -= moved
				if item.stack <= 0:
					return true
	if actor.inventory.size() >= 24:
		return false
	actor.inventory.append(item)
	return true


static func remove_item_count(actor: SimActor, def_id: String, count: int) -> bool:
	if actor.count_item(def_id) < count:
		return false
	var remaining := count
	var i := 0
	while i < actor.inventory.size() and remaining > 0:
		var item: SimItem = actor.inventory[i]
		if item.def_id != def_id:
			i += 1
			continue
		var take := mini(item.stack, remaining)
		item.stack -= take
		remaining -= take
		if item.stack <= 0:
			actor.inventory.remove_at(i)
		else:
			i += 1
	return remaining == 0


static func equip(actor: SimActor, item: SimItem) -> Dictionary:
	if item.equip_slot == "" or item.equip_slot == "none":
		return {"ok": false, "reason": "not_equippable"}
	if item not in actor.inventory:
		return {"ok": false, "reason": "not_in_inventory"}
	var previous = actor.equipment.get(item.equip_slot)
	actor.inventory.erase(item)
	if previous != null:
		actor.inventory.append(previous)
	actor.equipment[item.equip_slot] = item
	return {"ok": true, "unequipped": previous}


static func unequip(actor: SimActor, slot: String) -> Dictionary:
	var item = actor.equipment.get(slot)
	if item == null:
		return {"ok": false, "reason": "empty"}
	if not add_item(actor, item):
		return {"ok": false, "reason": "inventory_full"}
	actor.equipment[slot] = null
	return {"ok": true, "item": item}
