param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$requiredPaths = @(
  "engine/core/export/runtime_bundle_loader.h",
  "engine/core/export/runtime_bundle_loader.cpp",
  "engine/core/export/export_artifact_compare.h",
  "engine/core/export/export_artifact_compare.cpp",
  "engine/core/export/patch_manifest.h",
  "engine/core/export/patch_manifest.cpp",
  "engine/core/export/creator_package_manifest.h",
  "engine/core/export/creator_package_manifest.cpp",
  "content/schemas/patch_manifest.schema.json",
  "content/schemas/creator_package_manifest.schema.json",
  "content/fixtures/export_packaging_fixture.json",
  "docs/RELEASE_PACKAGING.md",
  "tools/ci/package_release_artifacts.ps1",
  "tests/unit/test_runtime_bundle_loader.cpp",
  "tests/unit/test_export_artifact_compare.cpp",
  "tests/unit/test_patch_manifest.cpp",
  "tests/unit/test_creator_package_manifest.cpp"
)

$missing = @()
foreach ($path in $requiredPaths) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

if ($missing.Count -gt 0) {
  throw "Missing FFS-07 export packaging files: $($missing -join ', ')"
}

$cmake = Get-Content (Join-Path $RepoRoot "CMakeLists.txt") -Raw
foreach ($needle in @(
  "engine/core/export/runtime_bundle_loader.cpp",
  "engine/core/export/export_artifact_compare.cpp",
  "engine/core/export/patch_manifest.cpp",
  "engine/core/export/creator_package_manifest.cpp",
  "tests/unit/test_runtime_bundle_loader.cpp",
  "tests/unit/test_export_artifact_compare.cpp",
  "tests/unit/test_patch_manifest.cpp",
  "tests/unit/test_creator_package_manifest.cpp"
)) {
  if (-not $cmake.Contains($needle)) {
    throw "CMakeLists.txt does not register $needle"
  }
}

$plan = Get-Content (Join-Path $RepoRoot "docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md") -Raw
if (-not $plan.Contains("FFS-07 - Export, Patch, And Packaging Hardening")) {
  throw "FFS-07 plan section is missing."
}
if ($plan.Contains("- [ ] Runtime loader rejects tampered bundles before loading content.")) {
  throw "FFS-07 runtime tamper checklist remains unchecked."
}

Write-Host "FFS-07 export packaging governance checks passed."
