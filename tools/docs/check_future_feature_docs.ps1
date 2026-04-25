$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$docsRoot = Join-Path $repoRoot "docs\certification"
$changelogPath = Join-Path $repoRoot "docs\SCHEMA_CHANGELOG.md"
$errors = @()

foreach ($relPath in @(
    "docs\certification\README.md",
    "docs\certification\template_certification.md",
    "docs\certification\feature_governance.md",
    "docs\certification\schema_changelog.md"
  )) {
  if (-not (Test-Path (Join-Path $repoRoot $relPath))) {
    $errors += "Missing future feature certification doc: $relPath"
  }
}

if (Test-Path $docsRoot) {
  $text = (Get-ChildItem -Path $docsRoot -Filter "*.md" -File | ForEach-Object {
      Get-Content -Raw -Path $_.FullName
    }) -join "`n"

  foreach ($phrase in @(
      "advisory",
      "not a release gate",
      "unsupported scope",
      "residual gaps",
      "disabled optional features"
    )) {
    if ($text -notmatch [regex]::Escape($phrase)) {
      $errors += "Certification docs are missing required phrase '$phrase'"
    }
  }
}

if (-not (Test-Path $changelogPath)) {
  $errors += "Missing schema changelog: docs/SCHEMA_CHANGELOG.md"
} else {
  $changelog = Get-Content -Raw -Path $changelogPath
  foreach ($schemaId in @("template_certification", "project_completeness_score", "feature_governance_manifest")) {
    if ($changelog -notmatch [regex]::Escape($schemaId)) {
      $errors += "Schema changelog is missing entry '$schemaId'"
    }
  }
}

$result = @{
  passed = $errors.Count -eq 0
  errors = $errors
}

$result | ConvertTo-Json -Depth 4
if ($errors.Count -gt 0) {
  exit 1
}
