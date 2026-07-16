class_name InventorySystem
extends RefCounted

const MAX_SLOTS := 36


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
	if actor.inventory.size() >= MAX_SLOTS:
		return false
	if actor.carry_weight() + item.total_weight() > actor.encumbrance_cap + 8.0:
		# Soft fail only when far over; allow slight overflow for pickup edge cases
		if actor.is_encumbered() and item.total_weight() > 2.0:
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


static func remove_instance(actor: SimActor, instance_id: String) -> SimItem:
	for i in range(actor.inventory.size()):
		var item: SimItem = actor.inventory[i]
		if item.instance_id == instance_id:
			actor.inventory.remove_at(i)
			return item
	return null


static func split_stack(actor: SimActor, instance_id: String, amount: int, new_id: String) -> Dictionary:
	var item := _find_instance(actor, instance_id)
	if item == null:
		return {"ok": false, "reason": "missing"}
	if amount <= 0 or amount >= item.stack:
		return {"ok": false, "reason": "bad_amount"}
	if actor.inventory.size() >= MAX_SLOTS:
		return {"ok": false, "reason": "inventory_full"}
	item.stack -= amount
	var copy := SimItem.from_dict(item.to_dict())
	copy.instance_id = new_id
	copy.stack = amount
	actor.inventory.append(copy)
	return {"ok": true, "item": copy}


static func drop_item(actor: SimActor, world: SimWorld, instance_id: String, amount: int = -1) -> Dictionary:
	var item := _find_instance(actor, instance_id)
	if item == null:
		return {"ok": false, "reason": "missing"}
	var drop: SimItem
	if amount > 0 and amount < item.stack:
		item.stack -= amount
		drop = SimItem.from_dict(item.to_dict())
		drop.instance_id = instance_id + "_drop"
		drop.stack = amount
	else:
		actor.inventory.erase(item)
		drop = item
	world.drop_item(actor.pos.x, actor.pos.y, drop)
	return {"ok": true, "item": drop}


static func equip(actor: SimActor, item: SimItem) -> Dictionary:
	var slot := item.equip_slot
	if slot == "" or slot == "none":
		return {"ok": false, "reason": "not_equippable"}
	if not SimItem.EQUIP_SLOTS.has(slot):
		return {"ok": false, "reason": "invalid_slot"}
	if item not in actor.inventory:
		return {"ok": false, "reason": "not_in_inventory"}
	var previous = actor.equipment.get(slot)
	actor.inventory.erase(item)
	if previous != null:
		actor.inventory.append(previous)
	actor.equipment[slot] = item
	return {"ok": true, "unequipped": previous}


static func unequip(actor: SimActor, slot: String) -> Dictionary:
	var item = actor.equipment.get(slot)
	if item == null:
		return {"ok": false, "reason": "empty"}
	if not add_item(actor, item):
		return {"ok": false, "reason": "inventory_full"}
	actor.equipment[slot] = null
	return {"ok": true, "item": item}


static func use_item(actor: SimActor, item: SimItem) -> Dictionary:
	if item == null:
		return {"ok": false, "reason": "missing"}
	match item.use_effect:
		"heal_8", "heal":
			actor.hp = mini(actor.max_hp, actor.hp + 8 + int(actor.skill_ranks.get("skill_medicine", 0)))
		"heal_16":
			actor.hp = mini(actor.max_hp, actor.hp + 16 + int(actor.skill_ranks.get("skill_medicine", 0)) * 2)
		"antidote":
			actor.status_effects.erase("poison")
		"stim":
			actor.add_status("stimulated", 5)
			actor.melee_accuracy += 0 # handled via status in combat later
		"ration":
			actor.hp = mini(actor.max_hp, actor.hp + 4)
		"smoke":
			actor.add_status("concealed", 3)
		"repair":
			actor.hp = mini(actor.max_hp, actor.hp + 6)
		_:
			if item.use_effect == "":
				return {"ok": false, "reason": "unusable"}
			actor.hp = mini(actor.max_hp, actor.hp + 5)
	remove_item_count(actor, item.def_id, 1)
	return {"ok": true, "effect": item.use_effect}


static func set_quick_slot(actor: SimActor, index: int, def_id: String) -> bool:
	if index < 0 or index >= actor.quick_slots.size():
		return false
	actor.quick_slots[index] = def_id
	if index == 0:
		actor.quick_item_id = def_id
	return true


static func sort_inventory(actor: SimActor, mode: String) -> void:
	var items: Array = actor.inventory.duplicate()
	items.sort_custom(func(a: SimItem, b: SimItem) -> bool:
		match mode:
			"name":
				return a.name.to_lower() < b.name.to_lower()
			"weight":
				return a.total_weight() < b.total_weight()
			"value":
				return a.value * a.stack > b.value * b.stack
			"category":
				if a.category == b.category:
					return a.name.to_lower() < b.name.to_lower()
				return a.category < b.category
			_:
				return a.name.to_lower() < b.name.to_lower()
	)
	actor.inventory = items


static func filter_inventory(actor: SimActor, category: String) -> Array:
	if category == "" or category == "all":
		return actor.inventory.duplicate()
	var out: Array = []
	for entry in actor.inventory:
		var item: SimItem = entry
		if item.category == category:
			out.append(item)
	return out


static func _find_instance(actor: SimActor, instance_id: String) -> SimItem:
	for entry in actor.inventory:
		var item: SimItem = entry
		if item.instance_id == instance_id:
			return item
	return null
