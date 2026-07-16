# Steamband Feature Matrix

Evidence base: `reference/steamband/lib/help/*.txt`, `reference/steamband/lib/edit/*.txt`, and prior handoff notes.  
**This is not a claim of complete parity.** Brassdeep reimplements selected behaviors in Godot.

| Feature | Evidence | Understanding | Priority | Phase | Status | Licensing / provenance |
|---------|----------|---------------|----------|-------|--------|------------------------|
| Character creation | `help/birth.txt`, `edit/p_race.txt`, `edit/p_class.txt` | Sex/race/class birth with stat/skill bases | High | 2 | **Partial** — 6×6 race/class, compare UI, no sex/history | Original IDs/text; do not copy Steamband prose |
| Races | `p_race.txt` (~10 races) | Stat mods, skills, resistances | High | 2 | **Partial** — Human, Automaton, Dwarf, Elf, Gnome, Beastfolk | Original |
| Classes | `p_class.txt` | Role kits with skills | High | 2 | **Partial** — Adventurer, Engineer, Gunslinger, Occultist, Officer, Scoundrel | Original |
| Statistics | `birth.txt` | STR/INT/WIS/DEX/CON/CHR | High | 2 | **Changed** — compact combat/craft stats + skill ranks | Original |
| Skills | `birth.txt` | Fight progression families | High | 2 | **Partial** — 8 families, ranks, costs, affinities, 14 abilities | Original |
| Powers / mutations | `mutation.c` | Racial powers | Medium | 4 | Missing | Reference only |
| Combat | `help/attack.txt` | To-hit, AC, blows | High | 2 | Partial — d100, crits, statuses | Original formulas |
| Firearms | gun items/classes | Guns + ammo | High | 2 | Partial — 5 firearms, 4 ammo types | Original |
| Equipment | `object.txt` | Slots, egos | High | 2 | Partial — 9 slots, weight, affixes | Original |
| Consumables | object data | Oils/meds | Medium | 2 | Partial — 6+ consumables | Original |
| Crafting | machinist themes | Stations/recipes | High | 2 | Partial — workbench/forge/alchemy, 20+ recipes | Original |
| Monsters | `monster.txt` | Bestiary + AI | High | 2 | Partial — 13 types incl. boss | Original names |
| Procedural generation | `generate.c` | Multi-level dungeons | High | 2 | Partial — town + 5 depths, 2 themes | Original |
| Stores / economy | `store.c` | Town shops | High | 2 | Partial — 3 merchants + healer | Original |
| Quests / objectives | sparse | Limited | Low | 4 | Missing | — |
| Status effects | spells/melee | Poison/fear/steam | High | 2 | Partial — poison, fear, steam, conceal, stim | Original |
| Saving | `save.c` | Persist runs | High | 2 | **Changed** — JSON schema v2 + v1 migration | Do not reuse binary |
| QoL / controller | SteambandRedux controller | Ally mappings | High | 2 | Partial — pad path + E2E/focus audit; Ally hardware pending | Original |

## Intentionally changed

- Presentation: isometric Godot with event-driven animation sequencing.
- Stats/skills: data-driven ranks rather than full Steamband skill tables.
- Saves: versioned JSON with hub/expedition state.
- Progression: save-reload hub with character death ending that operative (ADR-007).
