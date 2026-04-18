# BattleScene Presentation Contract

## Purpose
Defines the staged spatial composition for combat encounters.

## Minimum Requirements
- **Staged Spatial Composition**: Intent for "Stage Background" vs "Combatants" layers.
- **Formation Anchoring**: Must support all formation sizes with guaranteed placement.
- **Readability Guarantees**: Effect layers (hits, spells) MUST NOT obscure tactical data or target cursors.
- **Effect Layering Priority**: Clear hierarchy for status effects vs skill animations.
- **Camera Profiles**: Pre-defined framing per battle type (Standard, Boss, Arena).
- **UI-Safe Zones**: No spatial elements allowed to bleed into reserved UI regions.

## Fallback Policy
- Degrade to flat 2D backgrounds with sprite-based depths.
- Switch to static or orthographic camera if spatial projection is unavailable.
