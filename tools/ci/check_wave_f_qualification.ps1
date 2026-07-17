$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$evaluationPath = Join-Path $repoRoot "content\readiness\ardy_evaluation.json"
$externalPath = Join-Path $repoRoot "content\readiness\external_release_qualification.json"
$environmentPath = Join-Path $repoRoot "tools\research\ardy\environment_manifest.json"
foreach ($path in @($evaluationPath, $externalPath, $environmentPath)) {
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Wave F evidence is missing: $path" }
}
$ardy = Get-Content -LiteralPath $evaluationPath -Raw | ConvertFrom-Json
$external = Get-Content -LiteralPath $externalPath -Raw | ConvertFrom-Json
$environment = Get-Content -LiteralPath $environmentPath -Raw | ConvertFrom-Json
if ($ardy.schema -ne "urpg.ardy_evaluation.v1" -or $ardy.decision -ne "defer" -or
    @($ardy.admissionGates).Count -ne 7 -or @($ardy.packetDisposition.PSObject.Properties).Count -ne 10 -or
    $ardy.runtimeDependency -ne $false -or $ardy.packagedDependency -ne $false) {
  throw "ARDY defer evidence is incomplete or unsafe."
}
if ($environment.schema -ne "urpg.ardy_research_environment.v1" -or
    $environment.status -ne "not_provisioned_admission_deferred" -or @($environment.forbiddenInRepository).Count -lt 6) {
  throw "ARDY environment boundary is incomplete."
}
if ($external.schema -ne "urpg.external_release_qualification.v1" -or
    $external.status -ne "pending_external_execution" -or @($external.journeySessions).Count -ne 0 -or
    @($external.finalGates).Count -ne 10 -or $external.decision.signed -ne $false) {
  throw "External research/beta evidence must remain truthfully pending until executed."
}
Write-Host "Wave F qualification passed: ARDY decision=defer, conditional packets closed=8, external research/beta pending."
