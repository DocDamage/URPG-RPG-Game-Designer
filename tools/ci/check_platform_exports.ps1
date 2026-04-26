[CmdletBinding()]
param(
  [string]$ExportDir = "build/platform-exports",

  [ValidateSet("Windows_x64", "Linux_x64", "macOS_Universal", "Web_WASM")]
  [string]$Target,

  [string]$BuildDirectory,

  [string]$PackCliPath,

  [switch]$Json
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")

function ConvertTo-RepoPath {
  param([Parameter(Mandatory = $true)][string]$Path)

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }

  return [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Path))
}

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

function Test-ExistingExport {
  param(
    [Parameter(Mandatory = $true)][string]$ExportDir,
    [Parameter(Mandatory = $true)][string]$Target
  )

  $requirements = Get-RequirementsForTarget -Target $Target
  $missing = @()

  foreach ($req in $requirements) {
    $found = Test-Requirement -ExportDir $ExportDir -Requirement $req
    if ($req.Required -and -not $found) {
      $missing += $req.Description + " (" + $req.Pattern + ")"
    }
  }

  return @{
    target = $Target
    passed = $missing.Count -eq 0
    errors = @($missing | ForEach-Object { "Missing required file: $_" })
  }
}

function Resolve-PackCliPath {
  param(
    [string]$BuildDirectory,
    [string]$PackCliPath
  )

  if (-not [string]::IsNullOrWhiteSpace($PackCliPath)) {
    $resolved = ConvertTo-RepoPath $PackCliPath
    if (Test-Path $resolved) {
      return $resolved
    }
    throw "urpg_pack_cli was not found at '$resolved'."
  }

  $buildDir = if ([string]::IsNullOrWhiteSpace($BuildDirectory)) {
    Join-Path $repoRoot "build/dev-ninja-debug"
  } else {
    ConvertTo-RepoPath $BuildDirectory
  }

  $candidates = @(
    (Join-Path $buildDir "urpg_pack_cli.exe"),
    (Join-Path $buildDir "urpg_pack_cli")
  )

  foreach ($candidate in $candidates) {
    if (Test-Path $candidate) {
      return $candidate
    }
  }

  throw "urpg_pack_cli was not found in '$buildDir'. Build the pack CLI before running platform export validation."
}

function Invoke-PackCliJson {
  param(
    [Parameter(Mandatory = $true)][string]$PackCli,
    [Parameter(Mandatory = $true)][string]$Target,
    [Parameter(Mandatory = $true)][string]$OutputDir
  )

  $stdout = Join-Path ([System.IO.Path]::GetTempPath()) ("urpg_platform_export_{0}_{1}.json" -f $Target, ([guid]::NewGuid()))
  $stderr = Join-Path ([System.IO.Path]::GetTempPath()) ("urpg_platform_export_{0}_{1}.err" -f $Target, ([guid]::NewGuid()))

  try {
    & $PackCli --json --target $Target --output $OutputDir > $stdout 2> $stderr
    $exitCode = $LASTEXITCODE
    $stdoutText = if (Test-Path $stdout) { Get-Content -Raw $stdout } else { "" }
    $stderrText = if (Test-Path $stderr) { Get-Content -Raw $stderr } else { "" }

    $report = $null
    if (-not [string]::IsNullOrWhiteSpace($stdoutText)) {
      $report = $stdoutText | ConvertFrom-Json
    }

    return [pscustomobject]@{
      exitCode = $exitCode
      stdout = $stdoutText
      stderr = $stderrText
      report = $report
    }
  } finally {
    Remove-Item -LiteralPath $stdout, $stderr -Force -ErrorAction SilentlyContinue
  }
}

function New-SkipResult {
  param(
    [Parameter(Mandatory = $true)][string]$Target,
    [Parameter(Mandatory = $true)][string]$Reason
  )

  return [ordered]@{
    target = $Target
    status = "skipped"
    passed = $true
    reason = $Reason
    errors = @()
  }
}

function Test-CanRunTarget {
  param([string]$Target)

  $null = $Target
  return $null
}

function Clear-MatrixOutput {
  param(
    [Parameter(Mandatory = $true)][string]$ExportRoot,
    [Parameter(Mandatory = $true)][string]$Target
  )

  $resolvedRoot = [System.IO.Path]::GetFullPath($ExportRoot)
  New-Item -ItemType Directory -Path $resolvedRoot -Force | Out-Null
  $targetDir = [System.IO.Path]::GetFullPath((Join-Path $resolvedRoot $Target))

  $rootWithSeparator = $resolvedRoot.TrimEnd([System.IO.Path]::DirectorySeparatorChar, [System.IO.Path]::AltDirectorySeparatorChar) +
    [System.IO.Path]::DirectorySeparatorChar
  if (-not $targetDir.StartsWith($rootWithSeparator, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to clear export output outside export root: $targetDir"
  }

  if (Test-Path $targetDir) {
    Remove-Item -LiteralPath $targetDir -Recurse -Force
  }
  New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
  return $targetDir
}

function Invoke-PlatformExportMatrix {
  param(
    [Parameter(Mandatory = $true)][string]$ExportRoot,
    [Parameter(Mandatory = $true)][string]$PackCli,
    [string]$SingleTarget
  )

  $targets = if ([string]::IsNullOrWhiteSpace($SingleTarget)) {
    @("Windows_x64", "Linux_x64", "macOS_Universal", "Web_WASM")
  } else {
    @($SingleTarget)
  }

  $results = @()

  foreach ($targetName in $targets) {
    $skipReason = Test-CanRunTarget -Target $targetName
    if ($null -ne $skipReason) {
      $results += New-SkipResult -Target $targetName -Reason $skipReason
      continue
    }

    $targetDir = Clear-MatrixOutput -ExportRoot $ExportRoot -Target $targetName
    $cliResult = Invoke-PackCliJson -PackCli $PackCli -Target $targetName -OutputDir $targetDir
    $cliPassed = $cliResult.exitCode -eq 0
    $errors = @()

    if ($null -ne $cliResult.report -and $null -ne $cliResult.report.postExportValidation) {
      $errors += @($cliResult.report.postExportValidation.errors)
    }
    if (-not $cliPassed -and $errors.Count -eq 0) {
      if (-not [string]::IsNullOrWhiteSpace($cliResult.stderr)) {
        $errors += $cliResult.stderr.Trim()
      } else {
        $errors += "urpg_pack_cli failed with exit code $($cliResult.exitCode)."
      }
    }

    $results += [ordered]@{
      target = $targetName
      status = if ($cliPassed) { "passed" } else { "failed" }
      passed = $cliPassed
      outputDir = $targetDir
      errors = @($errors)
      report = $cliResult.report
    }
  }

  $failed = @($results | Where-Object { $_.status -eq "failed" })
  return [ordered]@{
    tool = "check_platform_exports"
    phase = "platform_export_matrix"
    success = $failed.Count -eq 0
    exportRoot = $ExportRoot
    packCli = $PackCli
    results = @($results)
  }
}

$resolvedExportDir = ConvertTo-RepoPath $ExportDir

if (-not [string]::IsNullOrWhiteSpace($Target) -and
    [string]::IsNullOrWhiteSpace($BuildDirectory) -and
    [string]::IsNullOrWhiteSpace($PackCliPath)) {
  $singleReport = Test-ExistingExport -ExportDir $resolvedExportDir -Target $Target
  if ($Json) {
    $singleReport | ConvertTo-Json -Compress
    if (-not $singleReport.passed) {
      exit 1
    }
    exit 0
  }

  if (-not $singleReport.passed) {
    throw "Platform export validation failed for $Target. $($singleReport.errors -join '; ')"
  }
  Write-Host "Platform export validation passed for $Target"
  exit 0
}

$resolvedPackCli = Resolve-PackCliPath -BuildDirectory $BuildDirectory -PackCliPath $PackCliPath
$matrixReport = Invoke-PlatformExportMatrix -ExportRoot $resolvedExportDir -PackCli $resolvedPackCli -SingleTarget $Target

if ($Json) {
  $matrixReport | ConvertTo-Json -Depth 64
} else {
  foreach ($result in $matrixReport.results) {
    if ($result.status -eq "skipped") {
      Write-Host "Platform export skipped for $($result.target): $($result.reason)" -ForegroundColor Yellow
    } elseif ($result.passed) {
      Write-Host "Platform export passed for $($result.target): $($result.outputDir)" -ForegroundColor Green
    } else {
      Write-Host "Platform export failed for $($result.target): $($result.errors -join '; ')" -ForegroundColor Red
    }
  }
}

if (-not $matrixReport.success) {
  exit 1
}
