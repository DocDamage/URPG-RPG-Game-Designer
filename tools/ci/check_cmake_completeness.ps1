$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$cmakePath = Join-Path $repoRoot "CMakeLists.txt"

Write-Host "Checking CMakeLists.txt completeness..."

if (-not (Test-Path $cmakePath)) {
    throw "CMakeLists.txt not found at $cmakePath"
}

$cmakeContent = Get-Content -Raw -Path $cmakePath

# Normalize path separators to forward slashes for consistent comparison
function Normalize-Path {
    param([string]$path)
    return $path.Replace('\', '/').Trim()
}

# Extract files listed under a target block (add_library or add_executable)
function Get-TargetFiles {
    param(
        [string]$content,
        [string]$targetName
    )
    $pattern = '(?s)add_(?:library|executable)\(\s*' + [regex]::Escape($targetName) + '\b\s+((?:[^)]+\n?)+)\)'
    $match = [regex]::Match($content, $pattern)
    if (-not $match.Success) {
        return @()
    }
    $block = $match.Groups[1].Value
    $lines = $block -split "`r?`n"
    $files = @()
    foreach ($line in $lines) {
        $trimmed = $line.Trim()
        # Skip empty lines and CMake comments
        if ([string]::IsNullOrWhiteSpace($trimmed) -or $trimmed.StartsWith('#')) {
            continue
        }
        # Remove trailing comments
        $commentIndex = $trimmed.IndexOf('#')
        if ($commentIndex -ge 0) {
            $trimmed = $trimmed.Substring(0, $commentIndex).Trim()
        }
        if (-not [string]::IsNullOrWhiteSpace($trimmed)) {
            $files += (Normalize-Path $trimmed)
        }
    }
    return $files
}

$coreFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_core"
$testFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_tests"
$integrationFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_integration_tests"
$snapshotFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_snapshot_tests"
$compatFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_compat_tests"
$migrateFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_migrate"
$auditFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_project_audit"
$presValFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_presentation_release_validation"

$allTestExeFiles = $testFiles + $integrationFiles + $snapshotFiles + $compatFiles + $migrateFiles + $auditFiles + $presValFiles
$allCMakeFiles = $coreFiles + $allTestExeFiles

# Standalone tools and known debt that are intentionally not in a library/test target
# TODO: Remediate pre-existing orphaned files from earlier agent swarms.
$knownDebt = @(
    # Standalone profiling tool (has main(), not part of urpg_core)
    "engine/core/presentation/profile_arena.cpp"
    # Pre-existing engine/editor sources with unresolved compile errors (missing imgui.h, missing includes, etc.)
    "engine/core/editor/doc_generator.cpp"
    "engine/core/editor/plugin_host.cpp"
    "engine/core/editor/script_bridge.cpp"
    "engine/core/editor/panels/scripting_console.cpp"
    "engine/core/ui/battle_tactics_window.cpp"
    # Pre-existing test files not yet registered in test executables
    "tests/test_ability_inspector.cpp"
    "tests/test_ability_pattern_integration.cpp"
    "tests/test_ability_state_machine.cpp"
    "tests/test_ability_tasks.cpp"
    "tests/test_effect_modifiers.cpp"
    "tests/test_pattern_field_editor.cpp"
    "tests/test_pattern_serialization.cpp"
    # Header companions of the known-debt .cpp files above
    "engine/core/editor/doc_generator.h"
    "engine/core/editor/plugin_host.h"
    "engine/core/editor/script_bridge.h"
    "engine/core/editor/panels/scripting_console.h"
    "engine/core/ui/battle_tactics_window.h"
)

$orphans = @()

# 1. Validate engine/ and editor/ .cpp files are in urpg_core
$sourceDirs = @("engine", "editor")
foreach ($dir in $sourceDirs) {
    $searchPath = Join-Path $repoRoot $dir
    if (-not (Test-Path $searchPath)) {
        continue
    }
    $cppFiles = Get-ChildItem -Path $searchPath -Recurse -Filter "*.cpp" -File
    foreach ($file in $cppFiles) {
        $relativePath = Normalize-Path $file.FullName.Substring($repoRoot.Path.Length + 1)
        if ($knownDebt -contains $relativePath) {
            continue
        }
        if ($allCMakeFiles -notcontains $relativePath) {
            $orphans += $relativePath
        }
    }
}

# 2. Validate tests/ test_*.cpp files are in appropriate test executable
$testsPath = Join-Path $repoRoot "tests"
if (Test-Path $testsPath) {
    $testCppFiles = Get-ChildItem -Path $testsPath -Recurse -Filter "test_*.cpp" -File
    foreach ($file in $testCppFiles) {
        $relativePath = Normalize-Path $file.FullName.Substring($repoRoot.Path.Length + 1)
        if ($knownDebt -contains $relativePath) {
            continue
        }
        if ($allTestExeFiles -notcontains $relativePath) {
            $orphans += $relativePath
        }
    }
}

# 3. Validate .h files with same-base-name .cpp siblings
foreach ($dir in $sourceDirs) {
    $searchPath = Join-Path $repoRoot $dir
    if (-not (Test-Path $searchPath)) {
        continue
    }
    $hFiles = Get-ChildItem -Path $searchPath -Recurse -Filter "*.h" -File
    foreach ($hFile in $hFiles) {
        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($hFile.Name)
        $parentDir = $hFile.DirectoryName
        $cppName = $baseName + ".cpp"
        $cppPath = Join-Path $parentDir $cppName
        if (Test-Path $cppPath) {
            $hRelativePath = Normalize-Path $hFile.FullName.Substring($repoRoot.Path.Length + 1)
            $cppRelativePath = Normalize-Path $cppPath.Substring($repoRoot.Path.Length + 1)
            if ($knownDebt -contains $hRelativePath -or $knownDebt -contains $cppRelativePath) {
                continue
            }
            # Covered if either the .h or its .cpp sibling is in CMakeLists.txt
            if (($allCMakeFiles -notcontains $hRelativePath) -and ($allCMakeFiles -notcontains $cppRelativePath)) {
                $orphans += $hRelativePath
            }
        }
    }
}

# Deduplicate while preserving order
$orphans = $orphans | Select-Object -Unique

if ($orphans.Count -gt 0) {
    Write-Host "FAIL: The following files are orphaned (not referenced in CMakeLists.txt):" -ForegroundColor Red
    foreach ($orphan in $orphans) {
        Write-Host "  - $orphan"
    }
    throw "CMake completeness check failed. $($orphans.Count) orphaned file(s) found."
}

Write-Host "PASS: CMakeLists.txt completeness check passed."
