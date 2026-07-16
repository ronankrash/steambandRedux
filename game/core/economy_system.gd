class_name EconomySystem
extends RefCounted


static func buy_price(item_def: Dictionary, merchant: Dictionary, actor: SimActor) -> int:
	var base := int(item_def.get("value", 1))
	var rate := float(merchant.get("buy_rate", 1.2))
	var charm := 1.0 - clampf(float(actor.skill_ranks.get("skill_stealth", 0)) * 0.02, 0.0, 0.15)
	return maxi(1, int(ceil(float(base) * rate * charm)))


static func sell_price(item: SimItem, merchant: Dictionary, actor: SimActor) -> int:
	var base := item.value * item.stack
	var rate := float(merchant.get("sell_rate", 0.45))
	var bonus := 1.0 + clampf(float(actor.skill_ranks.get("skill_engineering", 0)) * 0.02, 0.0, 0.2)
	return maxi(1, int(floor(float(base) * rate * bonus)))


static func buy(actor: SimActor, merchant_state: Dictionary, item_def: Dictionary, content: ContentDB, instance_id: String) -> Dictionary:
	var def_id := str(item_def.get("id", ""))
	var stock: Dictionary = merchant_state.get("stock", {})
	var have := int(stock.get(def_id, 0))
	if have <= 0:
		return {"ok": false, "reason": "out_of_stock"}
	var price := buy_price(item_def, merchant_state.get("def", {}), actor)
	if actor.gold < price:
		return {"ok": false, "reason": "no_gold", "price": price}
	var item := SimItem.from_definition(item_def, instance_id)
	if not InventorySystem.add_item(actor, item):
		return {"ok": false, "reason": "inventory_full"}
	actor.gold -= price
	stock[def_id] = have - 1
	merchant_state["stock"] = stock
	return {"ok": true, "price": price, "item": item}


static func sell(actor: SimActor, merchant_state: Dictionary, item: SimItem) -> Dictionary:
	if item == null:
		return {"ok": false, "reason": "missing"}
	var price := sell_price(item, merchant_state.get("def", {}), actor)
	actor.inventory.erase(item)
	actor.gold += price
	var stock: Dictionary = merchant_state.get("stock", {})
	stock[item.def_id] = int(stock.get(item.def_id, 0)) + item.stack
	merchant_state["stock"] = stock
	return {"ok": true, "price": price}


static func build_merchant_state(def: Dictionary) -> Dictionary:
	var stock: Dictionary = {}
	for entry in def.get("inventory", []):
		var iid := str(entry.get("item_id", ""))
		stock[iid] = int(entry.get("count", 1))
	return {"id": str(def.get("id", "")), "def": def, "stock": stock, "restocked_depth": 0}


static func maybe_restock(merchant_state: Dictionary, depth_cleared: int) -> bool:
	var def: Dictionary = merchant_state.get("def", {})
	var every := int(def.get("restock_on_depth", 2))
	if every <= 0:
		return false
	var last := int(merchant_state.get("restocked_depth", 0))
	if depth_cleared < last + every:
		return false
	var fresh := build_merchant_state(def)
	merchant_state["stock"] = fresh["stock"]
	merchant_state["restocked_depth"] = depth_cleared
	return true


static func heal_service(actor: SimActor, price: int = 15) -> Dictionary:
	if actor.gold < price:
		return {"ok": false, "reason": "no_gold", "price": price}
	actor.gold -= price
	actor.hp = actor.max_hp
	actor.status_effects.erase("poison")
	actor.status_effects.erase("fear")
	return {"ok": true, "price": price}
