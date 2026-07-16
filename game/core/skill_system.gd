class_name SkillSystem
extends RefCounted
## Data-driven skill ranks, costs, prerequisites, and ability unlocks.


static func rank(actor: SimActor, skill_id: String) -> int:
	return int(actor.skill_ranks.get(skill_id, 0))


static func next_cost(actor: SimActor, skill_def: Dictionary, affinities: Dictionary = {}) -> int:
	var current := rank(actor, str(skill_def.get("id", "")))
	var max_rank := int(skill_def.get("max_rank", 5))
	if current >= max_rank:
		return -1
	var base := int(skill_def.get("base_cost", 1))
	var growth := int(skill_def.get("cost_growth", 1))
	var cost := base + current * growth
	var mod := float(affinities.get(str(skill_def.get("id", "")), 1.0) as Variant)
	return maxi(1, int(ceilf(float(cost) * mod)))


static func can_raise(actor: SimActor, skill_def: Dictionary, content: ContentDB, affinities: Dictionary = {}) -> Dictionary:
	var skill_id := str(skill_def.get("id", ""))
	var current := rank(actor, skill_id)
	var max_rank := int(skill_def.get("max_rank", 5))
	if current >= max_rank:
		return {"ok": false, "reason": "max_rank"}
	var prereq_raw: Variant = skill_def.get("prerequisite", {})
	if typeof(prereq_raw) == TYPE_DICTIONARY and not (prereq_raw as Dictionary).is_empty():
		var prereq: Dictionary = prereq_raw
		var need_id := str(prereq.get("skill_id", ""))
		var need_rank := int(prereq.get("rank", 0))
		if rank(actor, need_id) < need_rank:
			return {"ok": false, "reason": "prerequisite", "need": need_id, "rank": need_rank}
	var cost := next_cost(actor, skill_def, affinities)
	if cost < 0:
		return {"ok": false, "reason": "max_rank"}
	if actor.skill_points < cost:
		return {"ok": false, "reason": "no_points", "cost": cost}
	return {"ok": true, "cost": cost}


static func raise_skill(actor: SimActor, skill_def: Dictionary, content: ContentDB, affinities: Dictionary = {}) -> Dictionary:
	var check := can_raise(actor, skill_def, content, affinities)
	if not check.get("ok", false):
		return check
	var cost: int = int(check["cost"])
	var skill_id := str(skill_def.get("id", ""))
	actor.skill_points -= cost
	actor.skill_ranks[skill_id] = rank(actor, skill_id) + 1
	_unlock_abilities(actor, skill_def)
	_sync_ability_stats(actor, content)
	return {"ok": true, "rank": rank(actor, skill_id), "spent": cost}


static func _unlock_abilities(actor: SimActor, skill_def: Dictionary) -> void:
	var current := rank(actor, str(skill_def.get("id", "")))
	for ability in skill_def.get("abilities", []):
		var aid := str(ability.get("id", ""))
		if aid == "":
			continue
		if current >= int(ability.get("rank_required", 99)) and aid not in actor.unlocked_abilities:
			actor.unlocked_abilities.append(aid)


static func _sync_ability_stats(actor: SimActor, content: ContentDB) -> void:
	# Clear prior ability stat keys
	var keys := actor.status_effects.keys()
	for key in keys:
		if str(key).begins_with("abilstat_"):
			actor.status_effects.erase(key)
	for skill_id in content.skills.keys():
		var skill_def: Dictionary = content.skills[skill_id]
		for ability in skill_def.get("abilities", []):
			var aid := str(ability.get("id", ""))
			if aid not in actor.unlocked_abilities:
				continue
			var effects: Dictionary = ability.get("effects", {})
			for stat in effects.keys():
				if typeof(effects[stat]) == TYPE_BOOL:
					continue
				var key := "abilstat_%s" % str(stat)
				actor.status_effects[key] = int(actor.status_effects.get(key, 0)) + int(effects[stat])


static func apply_starting_skills(actor: SimActor, starting: Dictionary, content: ContentDB) -> void:
	for skill_id in starting.keys():
		actor.skill_ranks[str(skill_id)] = int(starting[skill_id])
		var def := content.get_skill(str(skill_id))
		if not def.is_empty():
			_unlock_abilities(actor, def)
	_sync_ability_stats(actor, content)


static func explain_unavailable(check: Dictionary) -> String:
	match str(check.get("reason", "")):
		"max_rank":
			return "Already at maximum rank."
		"no_points":
			return "Need %d skill points." % int(check.get("cost", 0))
		"prerequisite":
			return "Requires %s rank %d." % [check.get("need", "?"), int(check.get("rank", 0))]
		_:
			return "Unavailable."
