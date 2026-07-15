param(
  [string]$ExpectedRoot = "",
  [bool]$RequireClean = $true
)

$ErrorActionPreference = "Stop"

function Invoke-Git {
  param([Parameter(Mandatory = $true)][string[]]$Arguments)
  $output = & git @Arguments 2>&1
  if ($LASTEXITCODE -ne 0) {
    throw "git $($Arguments -join ' ') failed: $($output -join [Environment]::NewLine)"
  }
  return @($output)
}

function Normalize-RepositoryIdentity {
  param([Parameter(Mandatory = $true)][string]$Origin)
  $value = $Origin.Trim().TrimEnd('/')
  if ($value.EndsWith('.git', [System.StringComparison]::OrdinalIgnoreCase)) {
    $value = $value.Substring(0, $value.Length - 4)
  }
  $match = [regex]::Match($value, '(?i)(?:github\.com[/:])(?<owner>[^/]+)/(?<repository>[^/]+)$')
  if (-not $match.Success) { return "" }
  return "$($match.Groups['owner'].Value)/$($match.Groups['repository'].Value)".ToLowerInvariant()
}

$rootOutput = @(& git rev-parse --show-toplevel 2>&1)
if ($LASTEXITCODE -ne 0 -or $rootOutput.Count -eq 0) {
  throw "Wrong repository: the current directory is not a Git worktree with the native URPG markers."
}
$repoRoot = ([string]$rootOutput[0]).Trim()
$repoRoot = [System.IO.Path]::GetFullPath($repoRoot)
if (-not [string]::IsNullOrWhiteSpace($ExpectedRoot)) {
  $expected = [System.IO.Path]::GetFullPath($ExpectedRoot)
  if (-not [string]::Equals($repoRoot, $expected, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Wrong repository root. Expected '$expected', found '$repoRoot'. Do not implement URPG work here."
  }
}

$requiredMarkers = @('AGENTS.md', 'CMakeLists.txt', 'apps/editor', 'engine/core')
$missingMarkers = @($requiredMarkers | Where-Object { -not (Test-Path -LiteralPath (Join-Path $repoRoot $_)) })
if ($missingMarkers.Count -gt 0) {
  throw "Repository root '$repoRoot' is missing required URPG marker(s): $($missingMarkers -join ', ')."
}

$origin = (Invoke-Git -Arguments @('-C', $repoRoot, 'remote', 'get-url', 'origin') | Select-Object -First 1).Trim()
$identity = Normalize-RepositoryIdentity -Origin $origin
if ($identity -ne 'docdamage/urpg-rpg-game-designer') {
  throw "Wrong origin '$origin' (normalized '$identity'). Expected DocDamage/URPG-RPG-Game-Designer."
}

$branch = (Invoke-Git -Arguments @('-C', $repoRoot, 'branch', '--show-current') | Select-Object -First 1).Trim()
$head = (Invoke-Git -Arguments @('-C', $repoRoot, 'rev-parse', 'HEAD') | Select-Object -First 1).Trim()
$upstreamOutput = @(& git -C $repoRoot rev-parse --abbrev-ref '@{upstream}' 2>$null)
if ($LASTEXITCODE -eq 0 -and $upstreamOutput.Count -gt 0) {
  $upstream = ([string]$upstreamOutput[0]).Trim()
} else {
  $upstream = 'none'
}
$originHeadOutput = @(& git -C $repoRoot symbolic-ref --quiet --short refs/remotes/origin/HEAD 2>$null)
if ($LASTEXITCODE -eq 0 -and $originHeadOutput.Count -gt 0) {
  $originHead = ([string]$originHeadOutput[0]).Trim()
} else {
  $originHead = 'unknown/offline'
}

$porcelain = @(Invoke-Git -Arguments @('-C', $repoRoot, 'status', '--porcelain=v1'))
$staged = @($porcelain | Where-Object { $_.Length -ge 2 -and $_.Substring(0, 1) -ne ' ' -and $_.Substring(0, 1) -ne '?' }).Count
$unstaged = @($porcelain | Where-Object { $_.Length -ge 2 -and $_.Substring(1, 1) -ne ' ' -and $_.Substring(0, 1) -ne '?' }).Count
$untracked = @($porcelain | Where-Object { $_.StartsWith('??') }).Count

$openPr = 'unknown/offline'
if (Get-Command gh -ErrorAction SilentlyContinue) {
  try {
    $openPrOutput = & gh pr list --repo DocDamage/URPG-RPG-Game-Designer --head $branch --state open --json number,url 2>$null
    if ($LASTEXITCODE -eq 0) {
      $openPr = if ([string]::IsNullOrWhiteSpace($openPrOutput)) { 'none' } else { $openPrOutput.Trim() }
    }
  } catch {
    $openPr = 'unknown/offline'
  }
}

Write-Host "URPG workspace identity"
Write-Host "  root: $repoRoot"
Write-Host "  origin: $origin"
Write-Host "  repository: $identity"
Write-Host "  branch: $branch"
Write-Host "  HEAD: $head"
Write-Host "  upstream: $upstream"
Write-Host "  integration base: $originHead"
Write-Host "  changes: staged=$staged unstaged=$unstaged untracked=$untracked"
Write-Host "  open PR: $openPr"

if ($RequireClean -and ($staged + $unstaged + $untracked -gt 0)) {
  throw "Workspace is not clean. Commit or remove staged, unstaged, and untracked changes before feature implementation."
}

Write-Host "Workspace identity check passed."
