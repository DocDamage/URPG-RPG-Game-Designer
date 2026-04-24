param(
    [string]$PolicyPath = "",
    [string]$GoldenRoot = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
if ([string]::IsNullOrWhiteSpace($PolicyPath)) {
    $PolicyPath = Join-Path $repoRoot "content\fixtures\visual_golden_governance.json"
}
if ([string]::IsNullOrWhiteSpace($GoldenRoot)) {
    $GoldenRoot = Join-Path $repoRoot "tests\snapshot\goldens"
}

if (-not (Test-Path $PolicyPath)) {
    throw "Missing visual golden governance policy: $PolicyPath"
}
if (-not (Test-Path $GoldenRoot)) {
    throw "Missing visual golden root: $GoldenRoot"
}

$policy = Get-Content -Raw -Path $PolicyPath | ConvertFrom-Json
if ($policy.version -ne 1) {
    throw "Unsupported visual golden governance policy version '$($policy.version)'."
}
if (-not $policy.maxUngovernedGoldenBytes -or $policy.maxUngovernedGoldenBytes -lt 1) {
    throw "visual_golden_governance.json must define maxUngovernedGoldenBytes."
}

$repoRootPath = (Resolve-Path $repoRoot).Path
$gitattributesPath = Join-Path $repoRoot ".gitattributes"
$gitattributes = Get-Content -Raw -Path $gitattributesPath
if ($gitattributes -notmatch '(?m)^\*\.json\s+text\s+eol=lf') {
    throw ".gitattributes must keep JSON goldens as LF-normalized text while the current policy keeps them in JSON."
}
if ($gitattributes -notmatch '(?m)^\*\.png\s+filter=lfs\s+diff=lfs\s+merge=lfs\s+-text') {
    throw ".gitattributes must keep PNG visual baselines covered by Git LFS for future compact golden storage."
}

$governed = @{}
foreach ($entry in $policy.oversizedGoldens) {
    if ([string]::IsNullOrWhiteSpace($entry.path)) {
        throw "Every oversizedGoldens entry must define path."
    }
    if ([string]::IsNullOrWhiteSpace($entry.reason)) {
        throw "Oversized golden '$($entry.path)' is missing reason."
    }
    if ([string]::IsNullOrWhiteSpace($entry.reviewStrategy)) {
        throw "Oversized golden '$($entry.path)' is missing reviewStrategy."
    }
    if ([string]::IsNullOrWhiteSpace($entry.humanReviewArtifact)) {
        throw "Oversized golden '$($entry.path)' is missing humanReviewArtifact."
    }
    if (-not $entry.maxBytes -or $entry.maxBytes -lt $policy.maxUngovernedGoldenBytes) {
        throw "Oversized golden '$($entry.path)' must define maxBytes >= maxUngovernedGoldenBytes."
    }

    $normalized = ($entry.path -replace '\\', '/')
    $absolute = Join-Path $repoRootPath ($normalized -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    if (-not (Test-Path $absolute)) {
        throw "Governed oversized golden does not exist: $($entry.path)"
    }
    $governed[$normalized.ToLowerInvariant()] = $entry
}

$oversizedCount = 0
$files = Get-ChildItem -Path $GoldenRoot -Recurse -File -Filter "*.golden.json"
foreach ($file in $files) {
    $relative = ($file.FullName.Substring($repoRootPath.Length + 1) -replace '\\', '/')
    $key = $relative.ToLowerInvariant()
    if ($file.Length -gt $policy.maxUngovernedGoldenBytes) {
        $oversizedCount += 1
        if (-not $governed.ContainsKey($key)) {
            throw "Golden '$relative' is $($file.Length) bytes and exceeds maxUngovernedGoldenBytes=$($policy.maxUngovernedGoldenBytes), but is not governed."
        }
        $entry = $governed[$key]
        if ($file.Length -gt $entry.maxBytes) {
            throw "Golden '$relative' is $($file.Length) bytes, above governed maxBytes=$($entry.maxBytes)."
        }
    }
}

foreach ($key in $governed.Keys) {
    $absolute = Join-Path $repoRootPath ($governed[$key].path -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    $file = Get-Item -Path $absolute
    if ($file.Length -le $policy.maxUngovernedGoldenBytes) {
        throw "Governed golden '$key' is no longer oversized; remove it from visual_golden_governance.json."
    }
}

Write-Host "Visual golden governance passed: $oversizedCount oversized goldens are explicitly governed."
