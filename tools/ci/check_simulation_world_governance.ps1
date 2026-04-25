$ErrorActionPreference = "Stop"

$required = @(
  "content/schemas/world_map_graph.schema.json",
  "content/schemas/crafting_registry.schema.json",
  "content/schemas/bestiary_registry.schema.json",
  "content/schemas/calendar_runtime.schema.json",
  "content/schemas/npc_schedule.schema.json",
  "content/schemas/puzzle_registry.schema.json",
  "content/schemas/runtime_hint_system.schema.json",
  "content/fixtures/simulation_world_fixture.json"
)

foreach ($path in $required) {
  if (-not (Test-Path $path)) {
    throw "Missing FFS-13 governance artifact: $path"
  }
}

Write-Host "FFS-13 simulation/world governance artifacts are present."
