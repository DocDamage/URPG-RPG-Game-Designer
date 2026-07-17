param()

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))

function Read-Matrix([string]$RelativePath, [string]$Schema) {
  $path = Join-Path $repoRoot $RelativePath
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave A matrix is missing: $RelativePath" }
  $value = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
  if ($value.schema -ne $Schema) { throw "Wave A matrix '$RelativePath' has unsupported schema '$($value.schema)'." }
  return $value
}

$reference = Read-Matrix "content\readiness\project_reference_matrix.json" "urpg.project_reference_matrix.v1"
$migration = Read-Matrix "content\readiness\project_migration_matrix.json" "urpg.project_migration_matrix.v1"
$widgets = Read-Matrix "content\readiness\editor_widget_state_matrix.json" "urpg.editor_widget_state_matrix.v1"
$scales = Read-Matrix "content\readiness\editor_scale_matrix.json" "urpg.editor_scale_matrix.v1"

$requiredReference = @("map","event","switch_variable","asset","dialogue","quest","menu","database_record","recipe","plugin","mod","package_inclusion")
$referenceTypes = @($reference.families | ForEach-Object { $_.type })
foreach ($type in $requiredReference) { if ($type -notin $referenceTypes) { throw "Reference matrix is missing '$type'." } }
foreach ($row in @($reference.families)) {
  foreach ($field in @("type","extractor","inbound","outbound","package","liveRoute","status","gap")) {
    if ($null -eq $row.PSObject.Properties[$field]) { throw "Reference matrix row '$($row.type)' is missing '$field'." }
  }
}

if (@($migration.documents).Count -ne 14) { throw "Migration matrix must cover all 14 durable document owners." }
$migrationTypes = @($migration.documents | ForEach-Object { $_.type })
if (@($migrationTypes | Select-Object -Unique).Count -ne 14) { throw "Migration matrix document types must be unique." }
$migrationCorpusPath = Join-Path $repoRoot "content\fixtures\project_document_migration_corpus.json"
$migrationCorpus = Get-Content -LiteralPath $migrationCorpusPath -Raw | ConvertFrom-Json
if ($migrationCorpus.schema -ne "urpg.project_document_migration_corpus.v1") { throw "Migration corpus schema is invalid." }
if (@($migrationCorpus.documents).Count -ne 14) { throw "Migration corpus must cover all 14 durable document owners." }
foreach ($row in @($migration.documents)) {
  if ($row.status -notin @("current_only","implemented","migration_required","forward_migration_registered")) { throw "Migration row '$($row.type)' has invalid status." }
  foreach ($field in @("type","versionField","current","oldestSupported","status","fixture")) {
    if ($null -eq $row.PSObject.Properties[$field]) { throw "Migration row '$($row.type)' is missing '$field'." }
  }
  if ($row.fixture -eq "missing" -or -not (Test-Path -LiteralPath (Join-Path $repoRoot $row.fixture) -PathType Leaf)) {
    throw "Migration row '$($row.type)' does not name an existing fixture corpus."
  }
  $fixtureRows = @($migrationCorpus.documents | Where-Object { $_.type -eq $row.type })
  if ($fixtureRows.Count -ne 1) { throw "Migration corpus must contain exactly one '$($row.type)' fixture." }
  $fixture = $fixtureRows[0]
  if ($fixture.versionField -ne $row.versionField -or $fixture.oldest -ne $row.oldestSupported -or
      $fixture.current -ne $row.current) {
    throw "Migration matrix and corpus disagree for '$($row.type)'."
  }
  if ($row.status -eq "forward_migration_registered" -and $row.oldestSupported -ne "unversioned") {
    throw "Registered forward migration '$($row.type)' must name its unversioned adoption source."
  }
}

$requiredStates = @("normal","hover","focused","pressed","disabled","loading","error")
foreach ($state in $requiredStates) { if ($state -notin @($widgets.requiredStates)) { throw "Widget matrix is missing state '$state'." } }
if (@($widgets.families).Count -ne 15) { throw "Widget matrix must cover all 15 shared widget families." }

$requiredScales = @(0.5,1.0,1.5,2.0,3.0)
foreach ($scale in $requiredScales) {
  if ($scale -notin @($scales.levels | ForEach-Object { [double]$_.scale })) { throw "Scale matrix is missing '$scale'." }
}

Write-Host "Wave A foundation matrices passed: $($reference.families.Count) reference families, $($migration.documents.Count) migration owners, $($widgets.families.Count) widget families, and $($scales.levels.Count) scale levels."
