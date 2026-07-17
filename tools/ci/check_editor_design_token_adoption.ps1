param()

$ErrorActionPreference = "Stop"
$repoRoot = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot "..\.."))
$primaryRoutes = @(
  "apps/editor/main.cpp",
  "editor/project/main_menu_panel.cpp",
  "editor/project/creator_checklist_panel.cpp",
  "editor/diagnostics/editor_error_card.cpp",
  "editor/assets/asset_library_panel.cpp",
  "editor/diagnostics/diagnostics_workspace.cpp",
  "editor/spatial/map_authoring_workspace.cpp",
  "editor/ability/ability_inspector_panel.cpp",
  "editor/character/character_creator_panel.cpp"
)

$violations = [System.Collections.Generic.List[string]]::new()
foreach ($relative in $primaryRoutes) {
  $path = Join-Path $repoRoot $relative
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Primary editor route is missing: $relative" }
  $lineNumber = 0
  foreach ($line in Get-Content -LiteralPath $path) {
    $lineNumber++
    if ($line -match 'TextColored\s*\(\s*ImVec4\s*\(' -or
        $line -match 'PushStyleColor\s*\([^,]+,\s*ImVec4\s*\(') {
      $violations.Add("${relative}:${lineNumber}: unexplained raw color literal")
    }
    if ($line -match 'style\.Colors\s*\[ImGuiCol_' -and $line -notmatch 'tokens\.colors\.at') {
      $violations.Add("${relative}:${lineNumber}: ImGui style color bypasses URPG design tokens")
    }
  }
}
if ($violations.Count -gt 0) { throw ($violations -join [Environment]::NewLine) }
Write-Host "Editor design-token adoption passed for $($primaryRoutes.Count) primary route owners."
