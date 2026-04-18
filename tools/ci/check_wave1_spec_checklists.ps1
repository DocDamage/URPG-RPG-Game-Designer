$ErrorActionPreference = "Stop"

$syncScript = Join-Path $PSScriptRoot "..\docs\sync-wave1-spec-checklist.ps1"
if (-not (Test-Path $syncScript)) {
    throw "Checklist sync script missing: $syncScript"
}

& $syncScript -Check
