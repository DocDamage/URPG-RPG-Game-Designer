param(
    [string]$Preset = "dev-ninja-debug",
    [switch]$Fresh
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildDir = Join-Path $repoRoot "build\$Preset"
$cachePath = Join-Path $buildDir "CMakeCache.txt"

function Get-CacheValue {
    param(
        [string[]]$Lines,
        [string]$Name
    )

    foreach ($line in $Lines) {
        if ($line -match "^$([regex]::Escape($Name)):[^=]*=(.*)$") {
            return $Matches[1]
        }
    }
    return ""
}

function Test-IsExpectedGnuCompiler {
    param(
        [string]$CompilerPath,
        [string[]]$Names
    )

    if ([string]::IsNullOrWhiteSpace($CompilerPath)) {
        return $false
    }

    $leaf = [System.IO.Path]::GetFileName($CompilerPath).ToLowerInvariant()
    return $Names -contains $leaf
}

$needsFreshConfigure = [bool]$Fresh
$reasons = @()

if (Test-Path $cachePath) {
    $cacheLines = Get-Content $cachePath
    $cCompiler = Get-CacheValue -Lines $cacheLines -Name "CMAKE_C_COMPILER"
    $cxxCompiler = Get-CacheValue -Lines $cacheLines -Name "CMAKE_CXX_COMPILER"
    $cFlagsDebug = Get-CacheValue -Lines $cacheLines -Name "CMAKE_C_FLAGS_DEBUG"
    $cxxFlagsDebug = Get-CacheValue -Lines $cacheLines -Name "CMAKE_CXX_FLAGS_DEBUG"

    if (-not (Test-IsExpectedGnuCompiler -CompilerPath $cCompiler -Names @("gcc.exe", "gcc"))) {
        $needsFreshConfigure = $true
        $reasons += "CMAKE_C_COMPILER is '$cCompiler' instead of gcc"
    }
    if (-not (Test-IsExpectedGnuCompiler -CompilerPath $cxxCompiler -Names @("g++.exe", "g++"))) {
        $needsFreshConfigure = $true
        $reasons += "CMAKE_CXX_COMPILER is '$cxxCompiler' instead of g++"
    }
    if ($cFlagsDebug -match "(^|\s)-Xclang(\s|$)|(^|\s)-gcodeview(\s|$)") {
        $needsFreshConfigure = $true
        $reasons += "CMAKE_C_FLAGS_DEBUG contains Clang-only debug flags"
    }
    if ($cxxFlagsDebug -match "(^|\s)-Xclang(\s|$)|(^|\s)-gcodeview(\s|$)") {
        $needsFreshConfigure = $true
        $reasons += "CMAKE_CXX_FLAGS_DEBUG contains Clang-only debug flags"
    }
}

if ($needsFreshConfigure) {
    if ($reasons.Count -gt 0) {
        Write-Host "URPG: repairing stale CMake configure cache for preset '$Preset':"
        foreach ($reason in $reasons) {
            Write-Host "  - $reason"
        }
    } else {
        Write-Host "URPG: running fresh CMake configure for preset '$Preset'."
    }
    cmake --fresh --preset $Preset
} else {
    cmake --preset $Preset
}
