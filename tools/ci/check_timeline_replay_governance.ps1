param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
)

$ErrorActionPreference = "Stop"

$required = @(
  "content/schemas/timeline.schema.json",
  "content/schemas/replay.schema.json",
  "content/fixtures/timeline_replay_fixture.json",
  "engine/core/timeline/timeline_document.h",
  "engine/core/timeline/timeline_player.h",
  "engine/core/events/event_macro_recorder.h",
  "engine/core/replay/replay_recorder.h",
  "engine/core/replay/replay_player.h",
  "engine/core/replay/replay_gallery.h",
  "engine/core/replay/golden_replay_lane.h",
  "tests/unit/test_timeline_player.cpp",
  "tests/unit/test_event_macro_recorder.cpp",
  "tests/unit/test_replay_recorder.cpp",
  "tests/integration/test_replay_integration.cpp",
  "tools/ci/check_golden_replays.ps1",
  "content/fixtures/golden_replays/expected.json",
  "content/fixtures/golden_replays/current.json"
)

$missing = @()
foreach ($path in $required) {
  if (-not (Test-Path (Join-Path $RepoRoot $path))) {
    $missing += $path
  }
}

if ($missing.Count -gt 0) {
  Write-Error ("Missing FFS-11 governance artifacts: " + ($missing -join ", "))
}

$cmake = Get-Content -Raw -Path (Join-Path $RepoRoot "CMakeLists.txt")
foreach ($needle in @(
  "engine/core/timeline/timeline_document.cpp",
  "engine/core/events/event_macro_recorder.cpp",
  "engine/core/replay/replay_recorder.cpp",
  "engine/core/replay/golden_replay_lane.cpp",
  "editor/timeline/timeline_panel.cpp",
  "tests/unit/test_timeline_player.cpp",
  "tests/integration/test_replay_integration.cpp",
  "urpg_golden_replay_lane"
)) {
  if ($cmake -notmatch [regex]::Escape($needle)) {
    Write-Error "CMake registration missing: $needle"
  }
}

Write-Host "FFS-11 timeline/replay governance artifacts are present."
