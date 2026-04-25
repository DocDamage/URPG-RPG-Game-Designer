param(
  [string]$FixtureRoot = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
if (-not $FixtureRoot) {
  $FixtureRoot = Join-Path $repoRoot "content\fixtures\template_certification"
}

$requiredFiles = @(
  "positive_jrpg.json",
  "negative_missing_loop.json"
)

$errors = @()
foreach ($file in $requiredFiles) {
  $path = Join-Path $FixtureRoot $file
  if (-not (Test-Path $path)) {
    $errors += "Missing template certification fixture: $file"
    continue
  }

  try {
    $json = Get-Content -Raw -Path $path | ConvertFrom-Json
  } catch {
    $errors += "Fixture '$file' is not valid JSON: $_"
    continue
  }

  if (-not $json.templateId) {
    $errors += "Fixture '$file' is missing templateId"
  }
  if (-not ($json.loops -is [System.Array])) {
    $errors += "Fixture '$file' must declare loops as an array"
  }
}

$positive = Join-Path $FixtureRoot "positive_jrpg.json"
if (Test-Path $positive) {
  $project = Get-Content -Raw -Path $positive | ConvertFrom-Json
  foreach ($loop in @("battle_loop", "save_loop")) {
    if ($project.loops -notcontains $loop) {
      $errors += "positive_jrpg.json is missing required loop '$loop'"
    }
  }
}

$negative = Join-Path $FixtureRoot "negative_missing_loop.json"
if (Test-Path $negative) {
  $project = Get-Content -Raw -Path $negative | ConvertFrom-Json
  if ($project.loops -contains "battle_loop") {
    $errors += "negative_missing_loop.json must omit battle_loop"
  }
}

$result = @{
  passed = $errors.Count -eq 0
  fixtureRoot = (Resolve-Path $FixtureRoot).Path
  errors = $errors
}

$result | ConvertTo-Json -Depth 4
if ($errors.Count -gt 0) {
  exit 1
}
