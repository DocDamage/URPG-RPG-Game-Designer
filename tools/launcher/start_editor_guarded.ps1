param(
    [string]$BuildRoot = "build/dev-ninja-debug",
    [string]$ProjectRoot = ".",
    [switch]$Visible,
    [switch]$ResetStartupGuard,
    [int]$VisibleFrames = 0
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$buildPath = Resolve-Path (Join-Path $repoRoot $BuildRoot)
$editorExe = Join-Path $buildPath "urpg_editor.exe"
if (!(Test-Path $editorExe)) {
    throw "Missing editor executable: $editorExe. Build with: cmake --build --preset dev-debug --target urpg_editor"
}

$resolvedProjectRoot = Resolve-Path (Join-Path $repoRoot $ProjectRoot)

if ($ResetStartupGuard) {
    & $editorExe --project-root $resolvedProjectRoot --reset-startup-guard
    exit $LASTEXITCODE
}

& $editorExe --project-root $resolvedProjectRoot --probe-platform
if ($LASTEXITCODE -ne 0) {
    throw "Editor platform probe failed. Visible startup was not attempted."
}

& $editorExe --project-root $resolvedProjectRoot --probe-render
if ($LASTEXITCODE -ne 0) {
    throw "Editor hidden render probe failed. Visible startup was not attempted."
}

& $editorExe --project-root $resolvedProjectRoot --probe-editor-frame
if ($LASTEXITCODE -ne 0) {
    throw "Editor hidden frame probe failed. Visible startup was not attempted."
}

if ($Visible) {
    if ($VisibleFrames -gt 0) {
        & $editorExe --project-root $resolvedProjectRoot --frames $VisibleFrames
    } else {
        & $editorExe --project-root $resolvedProjectRoot
    }
    exit $LASTEXITCODE
}

& $editorExe --project-root $resolvedProjectRoot --safe-mode --list-panels
exit $LASTEXITCODE
