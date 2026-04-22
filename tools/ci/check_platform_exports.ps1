[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ExportDir,

    [Parameter(Mandatory = $true)]
    [ValidateSet("Windows_x64", "Linux_x64", "macOS_Universal", "Web_WASM")]
    [string]$Target,

    [switch]$Json
)

$ErrorActionPreference = "Stop"

function Get-RequirementsForTarget {
    param([string]$Target)

    switch ($Target) {
        "Windows_x64" {
            return @(
                @{ Pattern = "*.exe"; Required = $true; Description = "Windows executable" },
                @{ Pattern = "data.pck"; Required = $true; Description = "Asset package" },
                @{ Pattern = "*.dll"; Required = $false; Description = "Dynamic libraries" }
            )
        }
        "Linux_x64" {
            return @(
                @{ Pattern = "executable_without_extension"; Required = $true; Description = "Linux executable" },
                @{ Pattern = "data.pck"; Required = $true; Description = "Asset package" },
                @{ Pattern = "*.so"; Required = $false; Description = "Shared libraries" }
            )
        }
        "macOS_Universal" {
            return @(
                @{ Pattern = "*.app"; Required = $true; Description = "macOS application bundle" },
                @{ Pattern = "data.pck"; Required = $true; Description = "Asset package" }
            )
        }
        "Web_WASM" {
            return @(
                @{ Pattern = "index.html"; Required = $true; Description = "HTML entry point" },
                @{ Pattern = "*.wasm"; Required = $true; Description = "WebAssembly binary" },
                @{ Pattern = "*.js"; Required = $true; Description = "JavaScript loader" },
                @{ Pattern = "data.pck"; Required = $true; Description = "Asset package" }
            )
        }
    }
}

function Test-Requirement {
    param(
        [string]$ExportDir,
        [hashtable]$Requirement
    )

    $pattern = $Requirement.Pattern

    if ($pattern -eq "*.app") {
        return (Get-ChildItem -Path $ExportDir -Directory -Filter "*.app" | Measure-Object).Count -gt 0
    }

    if ($pattern -eq "executable_without_extension") {
        return (Get-ChildItem -Path $ExportDir -File | Where-Object { $_.Name -notlike "*.*" } | Measure-Object).Count -gt 0
    }

    if ($pattern -like "*.*") {
        return (Get-ChildItem -Path $ExportDir -File -Filter $pattern | Measure-Object).Count -gt 0
    }

    return Test-Path -Path (Join-Path $ExportDir $pattern)
}

$requirements = Get-RequirementsForTarget -Target $Target
$missing = @()

foreach ($req in $requirements) {
    $found = Test-Requirement -ExportDir $ExportDir -Requirement $req
    if ($req.Required -and -not $found) {
        $missing += $req.Description + " (" + $req.Pattern + ")"
    }
}

$report = @{
    target = $Target
    passed = $missing.Count -eq 0
    errors = @($missing | ForEach-Object { "Missing required file: $_" })
}

if ($Json) {
    $report | ConvertTo-Json -Compress
    if (-not $report.passed) {
        exit 1
    }
} else {
    if ($missing.Count -gt 0) {
        throw "Platform export validation failed for $Target. Missing required files: $($missing -join ', ')"
    }
    Write-Host "Platform export validation passed for $Target"
}
