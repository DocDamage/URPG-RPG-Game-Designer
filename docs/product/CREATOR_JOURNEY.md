# Creator Journey Contract

This contract defines the reference creator journey used to measure the editor
as a product. It is deliberately expressed in creator actions rather than
editor subsystem names. The machine-readable counterpart is
`content/fixtures/creator_journey_spec.json`.

| ID | Creator action | Click budget | Time budget | Required evidence |
| --- | --- | ---: | ---: | --- |
| `launch_editor` | Launch the editor and reach a usable startup surface. | 1 | 30 s | Startup diagnostic or headless-shell report. |
| `create_project` | Create a named starter project without editing JSON. | 8 | 2 min | Valid project contract and creation diagnostic. |
| `discover_external_assets` | Search a configured external library without copying it into the project. | 4 | 30 s | Root/scan status and first result page. |
| `attach_sprite` | Review, license, promote, and attach one sprite to the project. | 8 | 2 min | Promotion and attachment manifests. |
| `paint_map` | Paint a tile or place a governed part on the starter map. | 5 | 1 min | Dirty map document and undo entry. |
| `add_event` | Add an event with a visible trigger and dialogue. | 6 | 2 min | Event data, preview, and diagnostic state. |
| `choose_spawn` | Choose the player-playtest spawn. | 3 | 30 s | Selected map/spawn target. |
| `playtest` | Start a current-map playtest. | 2 | 3 s | Session state, target, and runtime diagnostics. |
| `return_to_editor` | Return from playtest without losing unsaved work. | 1 | 10 s | Restored selection and unsaved-state indicator. |
| `save_project` | Save durable changes atomically. | 1 | 10 s | Save result and cleared dirty state. |
| `validate_project` | View focused package/map blockers. | 2 | 15 s | Validation report with focus routes. |
| `package_project` | Package the reviewed project. | 2 | 2 min | Shipping inventory and package validation. |

Automated reports may mark a step `passed`, `partial`, `failed`, or `deferred`. A deferred
step is acceptable only in the baseline while it is explicitly declared in the
fixture with a creator-readable reason and remediation milestone. It is never
evidence that the product promise is met.

A partial step names the exercised native seam and the remaining creator-flow
gap. It is not a fallback success and cannot be used as release evidence.

Manual graphical verification is required for startup layout, picker behavior,
asset-card readability, map painting feedback, playtest focus transfer,
keyboard navigation, and package-preview clarity. Headless tests prove state
and diagnostics, not visual quality or interaction feel.
