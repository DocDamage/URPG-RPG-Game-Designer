$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$path = Join-Path $repoRoot "content\readiness\external_release_qualification.json"
if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave G external qualification record is missing." }
$record = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
if ($record.schema -ne "urpg.external_release_qualification.v1" -or @($record.finalGates).Count -ne 10) {
  throw "Wave G external qualification schema is incomplete."
}
$expected = @("clean_clone", "clean_machine", "package", "upgrade", "save_migration", "accessibility", "performance", "security", "license", "truth")
$ids = @($record.finalGates | ForEach-Object id)
if (@($ids | Select-Object -Unique).Count -ne 10 -or @($expected | Where-Object { $_ -notin $ids }).Count -ne 0) {
  throw "Wave G final gate authority is incomplete or duplicated."
}
if ($record.candidateFreeze.frozen -or $null -ne $record.candidateBuild -or $record.decision.signed -or
    $record.decision.kind -ne "undecided") {
  throw "Wave G evidence claims a freeze or decision before qualification."
}
Write-Host "Wave G freeze readiness passed: 10 final authorities defined; candidate and decision truthfully pending."
