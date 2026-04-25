$ErrorActionPreference = "Stop"

$required = @(
  "editor/localization/localization_workspace_model.h",
  "editor/localization/localization_workspace_panel.h",
  "engine/core/accessibility/accessibility_fix_advisor.h",
  "editor/accessibility/accessibility_assistant_panel.h",
  "engine/core/input/input_remap_profile.h",
  "editor/input/input_remap_panel.h",
  "engine/core/platform/device_profile.h",
  "editor/platform/device_profile_panel.h",
  "content/schemas/device_profile.schema.json",
  "content/fixtures/device_profile_fixture.json",
  "content/fixtures/player_experience_fixture.json",
  "tests/unit/test_player_experience_platform_suite.cpp"
)

foreach ($path in $required) {
  if (-not (Test-Path $path)) {
    throw "Missing FFS-14 governance artifact: $path"
  }
}

Write-Host "FFS-14 player experience/platform governance artifacts are present."
