# Presentation Docs Hub

## Purpose
This folder is the documentation home for the native presentation subsystem: contracts, validation, spatial editor tooling, schema evolution, and performance guidance.

## Core Guides
- [Validation Guide](./VALIDATION.md)
- [Spatial Editor Tools](./SPATIAL_EDITOR_TOOLS.md)
- [Performance Budgets](./performance_budgets.md)
- [Schema Changelog](./schema_changelog.md)

## Scene Contracts
- [Scene Contracts Hub](./test_matrix/README.md)
- [MapScene Contract](./test_matrix/MapScene_Contract.md)
- [BattleScene Contract](./test_matrix/BattleScene_Contract.md)
- [MenuScene Contract](./test_matrix/MenuScene_Contract.md)
- [Overlay/UI Contract](./test_matrix/OverlayUI_Contract.md)

## Recommended Reading Order
1. [Validation Guide](./VALIDATION.md)
2. [MapScene Contract](./test_matrix/MapScene_Contract.md)
3. [Spatial Editor Tools](./SPATIAL_EDITOR_TOOLS.md)
4. [Performance Budgets](./performance_budgets.md)
5. [Schema Changelog](./schema_changelog.md)

## Notes
- Use this folder as the source-of-truth hub for presentation-specific operational docs.
- When adding a new presentation document, link it here so the subsystem stays discoverable from one place.
- Link integrity for the main presentation docs surfaces can be checked with `tools/docs/check-presentation-doc-links.ps1`.
