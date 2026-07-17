$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))

function Read-Evidence([string]$relativePath, [string]$schema) {
  $path = Join-Path $repoRoot $relativePath
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave D evidence is missing: $relativePath" }
  $value = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
  if ($value.schema -ne $schema) { throw "$relativePath has unsupported schema '$($value.schema)'." }
  return $value
}

$reload = Read-Evidence "content/readiness/playtest_hot_reload_capability_matrix.json" "urpg.playtest_reload_capabilities.v1"
if (@($reload.capabilities).Count -ne 9) { throw "Hot-reload evidence must contain nine resource classes." }
foreach ($row in @($reload.capabilities)) {
  if ($row.behavior -notin @("hot_reloadable", "restart_required", "rejected") -or
      [string]::IsNullOrWhiteSpace($row.failureRecovery)) { throw "Hot-reload row '$($row.resource_class)' is incomplete." }
}

$recovery = Read-Evidence "content/fixtures/playtest_recovery_fault_corpus.json" "urpg.playtest_recovery_fault_corpus.v1"
if (@($recovery.primaryDocuments).Count -ne 13 -or @($recovery.otherClasses).Count -ne 5) {
  throw "Recovery corpus must cover 13 primary documents and five other classes."
}

$redaction = Read-Evidence "content/fixtures/support_redaction_corpus.json" "urpg.support_redaction_corpus.v1"
if (@($redaction.values).Count -lt 6) { throw "Support redaction corpus is incomplete." }

$presentation = Read-Evidence "content/readiness/runtime_presentation_showcase_matrix.json" "urpg.runtime_presentation_showcase_matrix.v1"
if (@($presentation.components).Count -ne 7 -or @($presentation.variants).Count -ne 4) {
  throw "Presentation showcase matrix is incomplete."
}

$input = Read-Evidence "content/readiness/input_device_hotplug_matrix.json" "urpg.input_device_hotplug_matrix.v1"
if (@($input.devices).Count -ne 5 -or @($input.scenarios).Count -ne 12 -or @($input.requiredContexts).Count -ne 6) {
  throw "Input device matrix is incomplete."
}

$locale = Read-Evidence "content/readiness/wave_d_locale_accessibility_corpus.json" "urpg.wave_d_locale_accessibility_corpus.v1"
if (@($locale.locales).Count -ne 4 -or @($locale.accessibilityFailures).Count -ne 7 -or
    "rtl" -notin @($locale.locales.direction) -or "ime_entry" -notin @($locale.localizationCases)) {
  throw "Locale/accessibility corpus is incomplete."
}

Write-Host "Wave D qualification matrices passed: reload=9, recovery=18, devices=5, locales=4, accessibilityFailures=7."
