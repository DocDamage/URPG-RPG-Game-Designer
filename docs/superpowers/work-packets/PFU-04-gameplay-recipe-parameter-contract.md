# PFU-04 Work Packet: typed gameplay recipe parameters

**Status:** Implemented in worktree; verification deferred by user instruction

**Date:** 2026-07-15

## Scope

Extend the bounded F06 native recipe owner with explicit, typed template
parameters. A parameter may bind only to a named field on a known
`GameplayWysiwygRule`; it is never an arbitrary JSON pointer, project
mutation, MZ-plugin setting, or script expression. The first kinds are bounded
strings and signed 32-bit integers with optional inclusive integer bounds.

## Traceability and owners

- **Source outcomes:** F06 Gameplay Recipe Gallery; I07 typed configuration;
  F04 native/compatibility boundary.
- **Authoritative owner:** `engine/core/gameplay/gameplay_recipe_document.*`.
- **Editor projection:** `editor/gameplay/gameplay_recipe_panel.*` and the
  existing Ability-workspace gallery in `apps/editor/main.cpp`.
- **Schema impact:** Add parameter and binding metadata to
  `urpg.gameplay_recipe.v1`. The containing project document is persisted by
  the later owned project-persistence packet.

## Contract

1. Every parameter has a stable key, display label, kind, and a valid default.
   Integer parameters may declare an inclusive range.
2. Every binding names one existing parameter, one existing rule ID, and one
   compatible named field. Bindings may only target rule `target`, `effect`,
   `value`, `duration`, an existing variable-write value, or an existing
   resource-delta value.
3. Instantiation checks parameter keys, kinds, ranges, template validation,
   and binding targets before yielding a concrete recipe. Invalid input leaves
   the selected concrete recipe unchanged.
4. Apply and revert continue to use the existing receipt/target guard. A
   changed parameter set for an already-applied stable recipe is diagnosed as
   a conflict until the owned target is safely reverted.
5. The editor exposes only the parameter kinds declared by the selected native
   template. It does not imply general project persistence or MZ support.

## Acceptance and verification

- Defaults instantiate to the existing built-in recipe behavior.
- String and integer bindings affect only their declared rule fields.
- Unknown, missing, out-of-range, and incompatible bindings fail closed.
- Parameter changes after apply cannot overwrite the receipt-owned target.

```powershell
.\build\dev-ninja-debug\urpg_tests.exe "[gameplay][recipe]" --reporter compact
git diff --check
```

## Rollback and limits

The parameter descriptors and editor controls are isolated to the native
recipe owner. Arbitrary document editing, custom parameter expressions, plugin
configuration, and cross-domain target composition remain separate work.

## Implementation evidence

2026-07-15: `GameplayRecipe` now serializes typed string/integer parameter
descriptors and named bindings. Parameterization validates every key, default,
range, rule ID, map key, kind, field, and binding before changing a concrete
recipe; malformed configuration stays diagnostic-only. The starter quest
choice template exposes its quest-status variable write and reputation delta
as bounded native inputs. The Ability-workspace gallery renders descriptors
from the panel snapshot and routes edits through the parameterization owner;
an already-applied parameter set remains receipt/target guarded until safely
reverted. No build, test, or formatting command was run after this increment
because the user explicitly directed implementation to continue without
further test activity.
