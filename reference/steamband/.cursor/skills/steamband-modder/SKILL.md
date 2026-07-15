---
name: steamband-modder
description: Designs and implements moddability and extensibility. Focus on JSON configs for content (monsters, items, levels), data-driven systems, future NPC AI/memorypalace support. Use when working on mod system, content addition, scripting interfaces, or extensibility features. Ensure easy for players to add content.
---

# Steamband Modder Skill

## Goals
- Make game **easily extendible** with mods or configuration files.
- Support adding content (monsters, items, levels, meta data) without code changes.
- Prepare for future NPC AI conversations with memory system (memorypalace-like).
- Data-driven where possible to lower barrier for community contributions.

## Implementation Priorities
1. **JSON-based configs**: For monsters, items (ray guns, Victorian steampunk theme), levels, spells.
2. **Mod loading system**: Scan mod directories, merge with core data.
3. **Scripting hooks**: Plan for Lua or simple event system for advanced behaviors.
4. **Memory/NPC system**: Design data structures for NPC memory, dialogue trees that can be extended.
5. **First-person integration**: Mods should affect renderer (custom sprites/textures), gameplay logic.

## Best Practices
- Backward compatible with existing save files.
- Comprehensive tests for mod loading/parsing.
- Clear documentation and examples in mods/ directory.
- Licensing guidance for mod creators (permissive).
- Controller/UI compatibility for modded content.

Follow steamband-core.mdc for TDD and extensibility sections. Coordinate with playtester and graphics skills. Update HANDOFF.md and README.md as system evolves.
