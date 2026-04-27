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
            if ($trimmed -match ':(?<path>[^:<>]+\.(?:c|cc|cxx|cpp|h|hpp))>$') {
                $trimmed = $matches["path"]
            }
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
$profileArenaFiles = Get-TargetFiles -content $cmakeContent -targetName "urpg_profile_arena"

$allTestExeFiles = $testFiles + $integrationFiles + $snapshotFiles + $compatFiles + $migrateFiles + $auditFiles + $presValFiles
$standaloneToolFiles = $profileArenaFiles
$allCMakeFiles = $coreFiles + $allTestExeFiles + $standaloneToolFiles

# Standalone tools that are intentionally not in a library/test target.
# TD-01 stale runtime/editor seam exemptions have been burned down; anything left
# here should represent a real standalone tool instead of hidden production debt.
$knownDebt = @()

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
