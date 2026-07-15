# Steamband Feature Matrix

Evidence base: `reference/steamband/lib/help/*.txt`, `reference/steamband/lib/edit/*.txt`, and prior handoff notes.  
**This is not a claim of complete parity.** Brassdeep reimplements selected behaviors in Godot.

| Feature | Evidence | Understanding | Priority | Phase | Status | Licensing / provenance |
|---------|----------|---------------|----------|-------|--------|------------------------|
| Character creation | `help/birth.txt`, `edit/p_race.txt`, `edit/p_class.txt` | Sex/race/class birth with stat/skill bases | High | 1 | **Partial** — 2 races × 2 classes, no sex/history | Race/class *names* in Steamband data may be copyrightable expression; Brassdeep uses original IDs/text |
| Races | `p_race.txt` (~10 races) | Stat mods, skills, infravision, class limits | High | 2 | Partial (Human, Automaton) | Do not copy race prose/tables wholesale |
| Classes | `p_class.txt` (many) | Warriors, engineers, etc. with spellbooks | High | 2 | Partial (Adventurer, Engineer) | Same caution |
| Statistics | `birth.txt` | STR/INT/WIS/DEX/CON/CHR | High | 2 | **Changed** — vertical slice uses compact combat/engineering stats | Original |
| Skills | `birth.txt` | Disarm, devices, stealth, fight, shoot… | High | 2 | Partial — melee/ranged/defense/engineering points | Original |
| Powers / mutations | `mutation.c`, help | Racial powers, mutations | Medium | 4 | Missing | Reference only |
| Combat | `help/attack.txt`, `melee*.c` | To-hit, AC, blows | High | 1 | Partial — d100 accuracy, flat damage | Original formulas, not ported C |
| Firearms | Steamband gun items/classes | Guns + ammo as first-class | High | 1 | Partial — pepperbox + lead balls | Original item defs |
| Equipment | `object.txt`, ego items | Slots, bonuses, egos | High | 1–2 | Partial — weapon/armor + affix hooks | Do not copy artifact names |
| Consumables | object data | Potions/oil/food | Medium | 1 | Partial — machine oil heal | Original |
| Crafting / modification | Steamband machinist themes | Workbench recipes | High | 1 | Partial — 3 recipes | Original |
| Monsters | `monster.txt` | Large bestiary, flags | High | 1–3 | Partial — 3 behaviors | Do not copy monster names/prose |
| Procedural generation | `generate.c`, vaults | Rooms/corridors/vaults | High | 1–3 | Partial — room/corridor ruin | Original generator |
| Stores / economy | `store.c`, `shop_own.txt` | Town shops | Medium | 3 | Missing | Reference |
| Quests / objectives | sparse | Limited vs modern ARPGs | Low | 4 | Missing | — |
| Status effects | spells/melee | Blind, poison, stun… | High | 1–2 | Partial — poison | Original |
| Saving | `save.c` | Binary Angband saves | High | 1 | **Changed** — JSON schema v1 | Do not reuse binary format |
| QoL / controller | SteambandRedux controller work | Ally mappings | High | 1 | Partial — new Godot mapping | Original |

## Intentionally changed

- Presentation: isometric Godot vs ASCII/GDI/SDL prototypes.
- Stats model: simplified for a shippable slice; expand toward Steamband density later.
- Saves: versioned JSON instead of legacy binary.
