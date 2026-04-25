$ErrorActionPreference = "Stop"

$required = @(
  "engine/core/capture/capture_session.h",
  "engine/core/capture/capture_session.cpp",
  "engine/core/capture/trailer_capture_manifest.h",
  "engine/core/capture/trailer_capture_manifest.cpp",
  "engine/core/presentation/photo_mode_state.h",
  "engine/core/presentation/photo_mode_state.cpp",
  "engine/core/ui/theme_registry.h",
  "engine/core/ui/theme_registry.cpp",
  "engine/core/ui/theme_validator.h",
  "engine/core/ui/theme_validator.cpp",
  "editor/capture/capture_panel.h",
  "editor/capture/capture_panel.cpp",
  "editor/presentation/photo_mode_panel.h",
  "editor/presentation/photo_mode_panel.cpp",
  "editor/ui/theme_builder_panel.h",
  "editor/ui/theme_builder_panel.cpp",
  "content/schemas/ui_theme.schema.json",
  "content/fixtures/ui_theme_fixture.json",
  "content/fixtures/capture_theme_presentation_fixture.json",
  "tests/unit/test_capture_theme_presentation_suite.cpp"
)

foreach ($path in $required) {
  if (-not (Test-Path $path)) {
    throw "Missing FFS-15 governance artifact: $path"
  }
}

Write-Host "FFS-15 capture/theme/presentation governance artifacts are present."
