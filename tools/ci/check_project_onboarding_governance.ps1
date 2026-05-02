param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$requiredPaths = @(
  "engine/core/project/project_template_generator.h",
  "engine/core/project/project_template_generator.cpp",
  "engine/core/project/template_runtime_profile.h",
  "engine/core/project/template_runtime_profile.cpp",
  "engine/core/project/dev_room_generator.h",
  "engine/core/project/dev_room_generator.cpp",
  "engine/core/tutorial/tutorial_lesson.h",
  "engine/core/tutorial/tutorial_lesson.cpp",
  "editor/project/new_project_wizard_model.h",
  "editor/project/new_project_wizard_model.cpp",
  "editor/project/new_project_wizard_panel.h",
  "editor/project/new_project_wizard_panel.cpp",
  "content/fixtures/dev_room_fixture.json",
  "tests/unit/test_project_template_generator.cpp",
  "tests/unit/test_dev_room_generator.cpp",
  "tests/unit/test_tutorial_lesson.cpp"
)

$missing = @()
foreach ($path in $requiredPaths) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

$readiness = Get-Content (Join-Path $RepoRoot "content/readiness/readiness_status.json") -Raw | ConvertFrom-Json
foreach ($template in $readiness.templates) {
  $starterPath = "content/templates/$($template.id)_starter.json"
  if (-not (Test-Path (Join-Path $RepoRoot $starterPath))) {
    $missing += $starterPath
  }
}

if ($missing.Count -gt 0) {
  throw "Missing FFS-08 onboarding files: $($missing -join ', ')"
}

$cmake = Get-Content (Join-Path $RepoRoot "CMakeLists.txt") -Raw
foreach ($needle in @(
  "engine/core/project/project_template_generator.cpp",
  "engine/core/project/template_runtime_profile.cpp",
  "engine/core/project/dev_room_generator.cpp",
  "engine/core/tutorial/tutorial_lesson.cpp",
  "editor/project/new_project_wizard_model.cpp",
  "editor/project/new_project_wizard_panel.cpp",
  "tests/unit/test_project_template_generator.cpp",
  "tests/unit/test_dev_room_generator.cpp",
  "tests/unit/test_tutorial_lesson.cpp"
)) {
  if (-not $cmake.Contains($needle)) {
    throw "CMakeLists.txt does not register $needle"
  }
}

$planPath = Join-Path $RepoRoot "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md"
if (-not (Test-Path $planPath)) {
  $planPath = Join-Path $RepoRoot "docs/archive/planning/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md"
}
$plan = Get-Content $planPath -Raw
if ($plan.Contains("- [ ] Generate project from template with maps, menu, message, battle, save, localization, input, and export profile.")) {
  throw "FFS-08 project-template checklist remains unchecked."
}

Write-Host "FFS-08 project onboarding governance checks passed."
