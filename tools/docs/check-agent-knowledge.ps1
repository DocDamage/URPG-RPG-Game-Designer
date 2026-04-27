param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path,
  [string]$BuildDirectory = ""
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
  param([string]$Path)
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $RepoRoot $Path))
}

function Get-RepoRelativePath {
  param([string]$Path)

  $fullPath = [System.IO.Path]::GetFullPath($Path)
  $rootPath = [System.IO.Path]::GetFullPath($RepoRoot)
  if (-not $rootPath.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $rootPath = $rootPath + [System.IO.Path]::DirectorySeparatorChar
  }

  $rootUri = New-Object System.Uri($rootPath)
  $pathUri = New-Object System.Uri($fullPath)
  return [System.Uri]::UnescapeDataString($rootUri.MakeRelativeUri($pathUri).ToString()).Replace('/', [System.IO.Path]::DirectorySeparatorChar).Replace('\', '/')
}

function Test-CtestRegexMatches {
  param(
    [string]$BuildRoot,
    [string]$Pattern,
    [string]$Context
  )

  $output = & ctest --test-dir $BuildRoot -N -R $Pattern 2>&1
  if ($LASTEXITCODE -ne 0) {
    throw "CTest discovery failed while checking $Context '$Pattern'."
  }

  $joined = ($output -join "`n")
  if ($joined -match "Total Tests:\s+0") {
    throw "$Context matches zero discovered tests: $Pattern. CTest -R is case-sensitive; use exact discovered test-name casing."
  }
}

function Split-TopLevelAlternation {
  param([string]$Pattern)

  $parts = New-Object System.Collections.Generic.List[string]
  $depth = 0
  $escaped = $false
  $start = 0
  for ($i = 0; $i -lt $Pattern.Length; $i++) {
    $ch = $Pattern[$i]
    if ($escaped) {
      $escaped = $false
      continue
    }
    if ($ch -eq '\') {
      $escaped = $true
      continue
    }
    if ($ch -eq '(') {
      $depth++
      continue
    }
    if ($ch -eq ')' -and $depth -gt 0) {
      $depth--
      continue
    }
    if ($ch -eq '|' -and $depth -eq 0) {
      $parts.Add($Pattern.Substring($start, $i - $start))
      $start = $i + 1
    }
  }
  $parts.Add($Pattern.Substring($start))
  return @($parts)
}

function Expand-GroupedAlternation {
  param([string]$Pattern)

  $depth = 0
  $escaped = $false
  $groupStart = -1
  for ($i = 0; $i -lt $Pattern.Length; $i++) {
    $ch = $Pattern[$i]
    if ($escaped) {
      $escaped = $false
      continue
    }
    if ($ch -eq '\') {
      $escaped = $true
      continue
    }
    if ($ch -eq '(') {
      if ($depth -eq 0) {
        $groupStart = $i
      }
      $depth++
      continue
    }
    if ($ch -eq ')' -and $depth -gt 0) {
      $depth--
      if ($depth -eq 0 -and $groupStart -ge 0) {
        $inside = $Pattern.Substring($groupStart + 1, $i - $groupStart - 1)
        $alternatives = Split-TopLevelAlternation $inside
        if ($alternatives.Count -gt 1) {
          $expanded = New-Object System.Collections.Generic.List[string]
          $prefix = $Pattern.Substring(0, $groupStart)
          $suffix = $Pattern.Substring($i + 1)
          foreach ($alternative in $alternatives) {
            foreach ($candidate in (Expand-GroupedAlternation ($prefix + $alternative + $suffix))) {
              $expanded.Add($candidate)
            }
          }
          return @($expanded)
        }
      }
    }
  }

  return @($Pattern)
}

function Get-CtestRegexBranches {
  param([string]$Pattern)

  $branches = New-Object System.Collections.Generic.List[string]
  foreach ($part in (Split-TopLevelAlternation $Pattern)) {
    $trimmed = $part.Trim()
    if ([string]::IsNullOrWhiteSpace($trimmed)) {
      continue
    }
    foreach ($expanded in (Expand-GroupedAlternation $trimmed)) {
      $expandedTrimmed = $expanded.Trim()
      if (-not [string]::IsNullOrWhiteSpace($expandedTrimmed)) {
        $branches.Add($expandedTrimmed)
      }
    }
  }
  return @($branches)
}

function Get-ActiveMarkdownFiles {
  $roots = @(
    "AGENTS.md",
    "docs"
  )

  $files = New-Object System.Collections.Generic.List[string]
  foreach ($root in $roots) {
    $path = Resolve-RepoPath $root
    if (Test-Path -LiteralPath $path -PathType Leaf) {
      $files.Add($path)
      continue
    }
    if (Test-Path -LiteralPath $path -PathType Container) {
      Get-ChildItem -LiteralPath $path -Recurse -File -Filter "*.md" |
        Where-Object {
          $relative = Get-RepoRelativePath $_.FullName
          $relative -notmatch '^docs/(archive|audits|superpowers)/'
        } |
        ForEach-Object { $files.Add($_.FullName) }
    }
  }
  return @($files)
}

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$requiredDocs = @(
  "AGENTS.md",
  "docs/agent/INDEX.md",
  "docs/agent/ARCHITECTURE_MAP.md",
  "docs/agent/QUALITY_GATES.md",
  "docs/agent/EXECUTION_WORKFLOW.md",
  "docs/agent/KNOWN_DEBT.md",
  "docs/release/AAA_RELEASE_EXECUTION_PLAN.md"
)

foreach ($relative in $requiredDocs) {
  $path = Resolve-RepoPath $relative
  if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
    throw "Missing agent knowledge file: $relative"
  }
}

$agentLineCount = (Get-Content -LiteralPath (Resolve-RepoPath "AGENTS.md") | Measure-Object -Line).Lines
if ($agentLineCount -gt 120) {
  throw "AGENTS.md has $agentLineCount lines. Keep it as a compact table of contents (120 lines max)."
}

$agentText = Get-Content -Raw -LiteralPath (Resolve-RepoPath "AGENTS.md")
foreach ($requiredLink in @(
  "docs/agent/INDEX.md",
  "docs/agent/ARCHITECTURE_MAP.md",
  "docs/agent/QUALITY_GATES.md",
  "docs/agent/EXECUTION_WORKFLOW.md",
  "docs/agent/KNOWN_DEBT.md"
)) {
  if ($agentText -notmatch [regex]::Escape($requiredLink)) {
    throw "AGENTS.md must link to $requiredLink"
  }
}

if (-not [string]::IsNullOrWhiteSpace($BuildDirectory)) {
  $buildRoot = Resolve-RepoPath $BuildDirectory
  if (-not (Test-Path -LiteralPath (Join-Path $buildRoot "CMakeCache.txt") -PathType Leaf)) {
    throw "BuildDirectory must point to a configured CMake build tree: $buildRoot"
  }

  foreach ($docPath in (Get-ActiveMarkdownFiles)) {
    $relativeDocPath = Get-RepoRelativePath $docPath
    $docText = Get-Content -Raw -LiteralPath $docPath
    $matches = [regex]::Matches($docText, 'ctest[^\r\n`]*-R\s+"([^"]+)"[^\r\n`]*')
    foreach ($match in $matches) {
      $pattern = $match.Groups[1].Value
      $context = "$relativeDocPath ctest -R regex"
      Test-CtestRegexMatches -BuildRoot $buildRoot -Pattern $pattern -Context $context

      $branches = Get-CtestRegexBranches $pattern
      if ($branches.Count -gt 1) {
        foreach ($branch in $branches) {
          Test-CtestRegexMatches -BuildRoot $buildRoot -Pattern $branch -Context "$relativeDocPath ctest -R regex branch from '$pattern'"
        }
      }
    }
  }
}

Write-Host "Agent knowledge checks passed."
