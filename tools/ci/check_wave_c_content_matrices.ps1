$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))

function Read-Matrix([string]$relativePath, [string]$schema) {
  $path = Join-Path $repoRoot $relativePath
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave C evidence is missing: $relativePath" }
  $value = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
  $actualSchema = if ($null -ne $value.schema) { $value.schema } else { $value.schema_version }
  if ($actualSchema -ne $schema) { throw "$relativePath has unsupported schema '$actualSchema'." }
  return $value
}

$events = Read-Matrix "content/readiness/event_command_runtime_diagnostic_matrix.json" "urpg.event_command_runtime_diagnostic_matrix.v1"
if (@($events.commands).Count -ne 26) { throw "Native event evidence must contain exactly 26 command rows." }
foreach ($row in @($events.commands)) {
  if ([string]::IsNullOrWhiteSpace($row.id) -or [string]::IsNullOrWhiteSpace($row.authoring) -or
      [string]::IsNullOrWhiteSpace($row.serialization) -or [string]::IsNullOrWhiteSpace($row.runtime) -or
      @($row.diagnostics).Count -eq 0 -or $row.undo -ne $true) {
    throw "Native event row '$($row.id)' is incomplete."
  }
}

$corpus = Read-Matrix "content/fixtures/event_static_analysis_corpus.json" "urpg.event_static_analysis_corpus.v1"
$hazards = @("unsafe_parallel_mutation", "tight_loop", "infinite_loop", "localization_id_missing",
  "voice_caption_missing", "transfer_dead_end", "missing_switch_reference", "event_command_unreachable",
  "likely_autorun_softlock")
$bad = @($corpus.cases | Where-Object id -eq "all_requested_hazards")
if ($bad.Count -ne 1) { throw "Curated event corpus must have one all_requested_hazards case." }
foreach ($hazard in $hazards) {
  if ($hazard -notin @($bad[0].expectedCodes)) { throw "Curated event corpus is missing '$hazard'." }
}

$quest = Read-Matrix "content/fixtures/branching_voiced_quest_fixture.json" "urpg.branching_quest.v2"
if (@($quest.package_closure.voice_asset_ids).Count -lt 3 -or
    @($quest.package_closure.localization_keys).Count -lt 10 -or @($quest.quest_graph.links).Count -lt 2) {
  throw "Branching quest evidence lacks voice, localization, or branch closure."
}

$database = Read-Matrix "content/readiness/database_qualification_matrix.json" "urpg.database_qualification_matrix.v1"
if (@($database.recordKinds).Count -ne 13 -or $database.largeFixture.rows -lt 50000 -or
    $database.atomicInvalidBatch.requiresCompletePreview -ne $true -or
    $database.atomicInvalidBatch.requiresZeroMutation -ne $true -or @($database.seededBalanceReports).Count -eq 0) {
  throw "Database qualification matrix is incomplete."
}

$menu = Read-Matrix "content/readiness/menu_authoring_qualification_matrix.json" "urpg.menu_authoring_qualification_matrix.v1"
if (@($menu.viewports).Count -lt 5 -or @($menu.states).Count -ne 7 -or @($menu.templates).Count -ne 11) {
  throw "Menu qualification matrix is incomplete."
}

$semantic = Read-Matrix "content/readiness/semantic_editor_operation_matrix.json" "urpg.semantic_editor_operation_matrix.v1"
if (@($semantic.editors).Count -ne 5 -or "keyboard" -notin @($semantic.inputRoutes) -or
    "controller" -notin @($semantic.inputRoutes)) {
  throw "Semantic editor qualification matrix is incomplete."
}
foreach ($editor in @($semantic.editors)) {
  foreach ($operation in @("ordered_navigation", "select", "edit_properties", "focus_diagnostic")) {
    if ($operation -notin @($editor.operations)) { throw "$($editor.id) lacks semantic operation '$operation'." }
  }
  if ($editor.id -ne "tile_map" -and "create_connection" -notin @($editor.operations)) {
    throw "$($editor.id) lacks semantic connection creation."
  }
}

Write-Host "Wave C content matrices passed: eventCommands=26, databaseKinds=13, menuTemplates=11, semanticEditors=5."
