# Presentation Scene Contracts

## Purpose
This folder contains the scene-family contracts for the native presentation subsystem. These documents define the minimum behavior, fallback policy, and presentation expectations for each major scene type.

These contracts are reference material for translator/runtime behavior. They are not, by themselves, proof that every referenced scene-family surface is fully productized, compiled, or release-ready.

## Contracts
- [MapScene Contract](./MapScene_Contract.md)
- [BattleScene Contract](./BattleScene_Contract.md)
- [MenuScene Contract](./MenuScene_Contract.md)
- [Overlay/UI Contract](./OverlayUI_Contract.md)

## Recommended Use
- Read these contracts before changing translator behavior or presentation-runtime assumptions for a scene family.
- Use them alongside the focused presentation validation gate in [../VALIDATION.md](../VALIDATION.md).
- Keep conservative status explicit where the canonical docs still describe a scene-family surface or authoring path as partial or not yet release-ready.
- When a new scene-family contract is added, link it here and from the parent [presentation docs hub](../README.md).
