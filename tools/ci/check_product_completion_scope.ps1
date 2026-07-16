param([switch]$AllowOpenP0)

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$scopePath = Join-Path $repoRoot "content\readiness\product_completion_scope.json"
$defectPath = Join-Path $repoRoot "content\readiness\product_completion_defects.json"

foreach ($path in @($scopePath, $defectPath)) {
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    throw "Product-completion policy input is missing: $path"
  }
}

$scope = Get-Content -LiteralPath $scopePath -Raw | ConvertFrom-Json
$defects = Get-Content -LiteralPath $defectPath -Raw | ConvertFrom-Json
if ($scope.schema -ne "urpg.product_completion_scope.v1") {
  throw "Unsupported product-completion scope schema: $($scope.schema)"
}
if ($defects.schema -ne "urpg.product_completion_defects.v1") {
  throw "Unsupported product-completion defect schema: $($defects.schema)"
}

$allowedReasons = @($scope.admissionRule.allowedReasons)
foreach ($required in @("fix_p0_or_p1", "remove_recorded_debt", "improve_required_evidence", "separately_approved")) {
  if ($required -notin $allowedReasons) { throw "Scope admission rule is missing '$required'." }
}
if ($scope.admissionRule.newFeatureDefault -ne "deferred") {
  throw "Unapproved new feature work must default to deferred."
}

$entryIds = @{}
foreach ($entry in @($scope.entries)) {
  foreach ($field in @("id", "title", "priority", "owners", "dependencies", "acceptance", "deferralReason")) {
    if ($null -eq $entry.PSObject.Properties[$field]) { throw "Scope entry is missing '$field'." }
  }
  if ($entryIds.ContainsKey($entry.id)) { throw "Duplicate scope entry '$($entry.id)'." }
  $entryIds[$entry.id] = $true
  if ($entry.priority -notin @("P0", "P1", "RESEARCH", "DEFERRED")) {
    throw "Scope entry '$($entry.id)' has unsupported priority '$($entry.priority)'."
  }
  if (@($entry.owners).Count -eq 0 -or [string]::IsNullOrWhiteSpace($entry.acceptance) -or
      [string]::IsNullOrWhiteSpace($entry.deferralReason)) {
    throw "Scope entry '$($entry.id)' has incomplete owner, acceptance, or deferral data."
  }
}

$categories = @($defects.p0Categories)
$requiredCategories = @("data_loss", "crash", "unfinishable_golden_path", "inaccessible_required_action",
                        "packaging_corruption", "security_or_privacy_leak", "false_readiness_claim")
foreach ($category in $requiredCategories) {
  if ($category -notin $categories) { throw "P0 defect bar is missing category '$category'." }
}

$defectIds = @{}
$openP0 = @()
foreach ($defect in @($defects.defects)) {
  foreach ($field in @("id", "severity", "category", "status", "accepted", "owner", "summary", "disposition")) {
    if ($null -eq $defect.PSObject.Properties[$field]) { throw "Defect entry is missing '$field'." }
  }
  if ($defectIds.ContainsKey($defect.id)) { throw "Duplicate defect '$($defect.id)'." }
  $defectIds[$defect.id] = $true
  if ($defect.severity -eq "P0" -and $defect.category -notin $categories) {
    throw "P0 defect '$($defect.id)' has unknown category '$($defect.category)'."
  }
  if ($defect.status -notin @("open", "closed", "deferred")) {
    throw "Defect '$($defect.id)' has unsupported status '$($defect.status)'."
  }
  if ($defect.severity -eq "P0" -and $defect.accepted -eq $true -and $defect.status -eq "open") {
    $openP0 += $defect
  }
}

Write-Host "Product-completion scope policy is structurally valid: $(@($scope.entries).Count) scope entries, $(@($defects.defects).Count) defects."
if (-not $AllowOpenP0 -and $openP0.Count -gt 0) {
  $ids = @($openP0 | ForEach-Object { $_.id }) -join ", "
  throw "Release candidate creation is blocked by $($openP0.Count) accepted open P0 defect(s): $ids"
}
if ($AllowOpenP0 -and $openP0.Count -gt 0) {
  Write-Warning "Implementation validation allows $($openP0.Count) accepted open P0 defect(s); release-candidate validation will fail."
}
