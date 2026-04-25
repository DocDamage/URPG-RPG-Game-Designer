# Template Certification

Status Date: 2026-04-25

Template certification checks only the minimum authored loop evidence for a template family. It is
advisory and not a release gate. A passing certification report means the fixture contains the
required loop IDs for that template, not that the game is complete.

The default suites cover:

- `jrpg`
- `visual_novel`
- `turn_based_rpg`
- `tactics_rpg`
- `arpg`
- `monster_collector_rpg`
- `cozy_life_rpg`
- `metroidvania_lite`
- `2_5d_rpg`

Unsupported scope includes asset license clearance, full balance validation, platform-holder
certification, and automatic readiness promotion. Residual gaps must stay visible in readiness
docs until human review closes them.
