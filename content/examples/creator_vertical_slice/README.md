# Lantern of the Willow

`Lantern of the Willow` is URPG's bounded governed vertical-slice scenario. It
is intentionally small: a first playthrough must fit within fifteen minutes.
The scenario begins in Willow Village, crosses to the Moonwell Shrine, and
ends after the player returns the lantern to the village elder.

## Creator workflow

1. Create or open a JRPG project from the startup shell.
2. Attach only entries listed in `acceptance.json` through the Assets workflow.
3. Use the Map workspace to author Willow Village and Moonwell Shrine, their
   events, the shrine gate prop state, and the two NPCs.
4. Bind the two abilities, vendor stock, encounter, dialogue, quest, and save
   point through their contextual routes.
5. Run the vertical-slice gate before packaging. It rejects raw/external paths,
   local indexes, recovery snapshots, and playtest overlays.

## Asset contract

The example deliberately uses the existing release-governed proof-asset lanes
recorded in `content/fixtures/project_governance_fixture.json`; it does not
import, attach, or package any asset from `Building Game Template.zip` or any
other raw external source. The acceptance manifest identifies each selected
asset by its governed bundle and license evidence. A package remains blocked
until its exact promoted payload and attachment manifest are hydrated.

## Narrative and completion

The elder asks the player to restore the Moonwell Lantern. The player speaks
to the elder, buys a tonic, defeats the shrine encounter using either of two
abilities, receives the lantern, saves, reloads, returns to the elder, and
reaches the ending state. The scenario is an integration contract, not a claim
of final art direction or platform-release qualification.
