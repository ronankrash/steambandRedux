class_name CombatResolver
extends RefCounted

const HIT_BASE := 50


static func melee_hit_chance(attacker: SimActor, defender: SimActor) -> int:
	var chance := attacker.total_melee_accuracy() - defender.total_defense() + HIT_BASE
	if defender.status_effects.has("fear"):
		chance += 10
	if attacker.status_effects.has("stimulated"):
		chance += 8
	return clampi(chance, 5, 95)


static func ranged_hit_chance(attacker: SimActor, defender: SimActor) -> int:
	var chance := attacker.total_ranged_accuracy() - defender.total_defense() + HIT_BASE
	if attacker.has_ability("steady_aim"):
		chance += 5
	if defender.status_effects.has("concealed"):
		chance -= 15
	return clampi(chance, 5, 95)


static func _crit_chance(attacker: SimActor) -> int:
	var c := 5 + attacker.skill_bonus("crit_chance") + attacker.ability_bonus("crit_chance")
	if attacker.has_ability("crit_chance"):
		c += 5
	return clampi(c, 0, 40)


static func resolve_melee(rng: BrassRng, attacker: SimActor, defender: SimActor) -> Dictionary:
	var chance := melee_hit_chance(attacker, defender)
	var roll := rng.randi_range(1, 100)
	var hit := roll <= chance
	var damage := 0
	var critical := false
	if hit:
		damage = maxi(1, attacker.total_melee_damage() - int(defender.total_defense() / 2))
		if attacker.has_ability("power_strike"):
			damage += 2
		if rng.randi_range(1, 100) <= _crit_chance(attacker):
			critical = true
			damage = int(ceil(damage * 1.5))
		if defender.status_effects.has("slow"):
			damage += 1
		defender.hp -= damage
		if defender.hp <= 0:
			defender.hp = 0
			defender.alive = false
	return {
		"type": "melee",
		"hit": hit,
		"critical": critical,
		"roll": roll,
		"chance": chance,
		"damage": damage,
		"attacker_id": attacker.id,
		"defender_id": defender.id,
		"defender_dead": not defender.alive,
	}


static func resolve_ranged(rng: BrassRng, attacker: SimActor, defender: SimActor) -> Dictionary:
	var chance := ranged_hit_chance(attacker, defender)
	var roll := rng.randi_range(1, 100)
	var hit := roll <= chance
	var damage := 0
	var critical := false
	if hit:
		damage = maxi(1, attacker.total_ranged_damage() - int(defender.total_defense() / 2))
		if rng.randi_range(1, 100) <= _crit_chance(attacker):
			critical = true
			damage = int(ceil(damage * 1.5))
		defender.hp -= damage
		if defender.hp <= 0:
			defender.hp = 0
			defender.alive = false
	return {
		"type": "ranged",
		"hit": hit,
		"critical": critical,
		"roll": roll,
		"chance": chance,
		"damage": damage,
		"attacker_id": attacker.id,
		"defender_id": defender.id,
		"defender_dead": not defender.alive,
	}


static func has_line_of_sight(world: SimWorld, from: GridPos, to: GridPos) -> bool:
	var x0 := from.x
	var y0 := from.y
	var x1 := to.x
	var y1 := to.y
	var dx := absi(x1 - x0)
	var dy := -absi(y1 - y0)
	var sx := 1 if x0 < x1 else -1
	var sy := 1 if y0 < y1 else -1
	var err := dx + dy
	while true:
		if not (x0 == from.x and y0 == from.y) and not (x0 == to.x and y0 == to.y):
			if world.blocks_sight(x0, y0):
				return false
		if x0 == x1 and y0 == y1:
			break
		var e2 := 2 * err
		if e2 >= dy:
			err += dy
			x0 += sx
		if e2 <= dx:
			err += dx
			y0 += sy
	return true
