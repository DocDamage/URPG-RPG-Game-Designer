param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$requiredPaths = @(
  "engine/core/save/save_debugger.h",
  "engine/core/save/save_debugger.cpp",
  "engine/core/save/save_corruption_lab.h",
  "engine/core/save/save_corruption_lab.cpp",
  "engine/core/save/save_compatibility_preview.h",
  "engine/core/save/save_compatibility_preview.cpp",
  "engine/core/project/project_snapshot_store.h",
  "engine/core/project/project_snapshot_store.cpp",
  "editor/save/save_debugger_panel.h",
  "editor/save/save_debugger_panel.cpp",
  "editor/save/save_migration_preview_panel.h",
  "editor/save/save_migration_preview_panel.cpp",
  "content/fixtures/save_debugging_fixture.json",
  "tests/unit/test_save_debugger.cpp",
  "tests/unit/test_save_corruption_lab.cpp",
  "tests/unit/test_save_compatibility_preview.cpp",
  "tests/unit/test_project_snapshot_store.cpp"
)

$missing = @()
foreach ($path in $requiredPaths) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

if ($missing.Count -gt 0) {
  throw "Missing FFS-09 save debugging files: $($missing -join ', ')"
}

$cmake = Get-Content (Join-Path $RepoRoot "CMakeLists.txt") -Raw
foreach ($needle in @(
  "engine/core/save/save_debugger.cpp",
  "engine/core/save/save_corruption_lab.cpp",
  "engine/core/save/save_compatibility_preview.cpp",
  "engine/core/project/project_snapshot_store.cpp",
  "editor/save/save_debugger_panel.cpp",
  "editor/save/save_migration_preview_panel.cpp",
  "tests/unit/test_save_debugger.cpp",
  "tests/unit/test_save_corruption_lab.cpp",
  "tests/unit/test_save_compatibility_preview.cpp",
  "tests/unit/test_project_snapshot_store.cpp"
)) {
  if (-not $cmake.Contains($needle)) {
    throw "CMakeLists.txt does not register $needle"
  }
}

$plan = Get-Content (Join-Path $RepoRoot "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md") -Raw
if ($plan.Contains("- [ ] Inspect slots, metadata, recovery tier, migration notes, and subsystem state.")) {
  throw "FFS-09 save debugger checklist remains unchecked."
}

Write-Host "FFS-09 save debugging governance checks passed."
