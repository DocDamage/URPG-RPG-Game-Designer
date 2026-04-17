# Presentation Scene Contracts

## Purpose
This folder contains the scene-family contracts for the native presentation subsystem. These documents define the minimum behavior, fallback policy, and presentation expectations for each major scene type.

## Contracts
- [MapScene Contract](./MapScene_Contract.md)
- [BattleScene Contract](./BattleScene_Contract.md)
- [MenuScene Contract](./MenuScene_Contract.md)
- [Overlay/UI Contract](./OverlayUI_Contract.md)

## Recommended Use
- Read these contracts before changing translator behavior or presentation-runtime assumptions for a scene family.
- Use them alongside the focused presentation validation gate in [../VALIDATION.md](../VALIDATION.md).
- When a new scene-family contract is added, link it here and from the parent [presentation docs hub](../README.md).
