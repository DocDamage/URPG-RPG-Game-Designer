param()

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$filesToCheck = @(
    "docs/presentation/README.md",
    "docs/presentation/VALIDATION.md",
    "docs/presentation/test_matrix/README.md"
)

$markdownLinkPattern = '\[([^\]]+)\]\(([^)]+)\)'
$missingLinks = @()

foreach ($relativeFile in $filesToCheck) {
    $fullPath = Join-Path $repoRoot $relativeFile
    if (-not (Test-Path $fullPath)) {
        throw "Presentation doc not found: $relativeFile"
    }

    $content = Get-Content -Path $fullPath -Raw
    $matches = [regex]::Matches($content, $markdownLinkPattern)
    $baseDirectory = Split-Path -Path $fullPath -Parent

    foreach ($match in $matches) {
        $target = $match.Groups[2].Value.Trim()
        if ([string]::IsNullOrWhiteSpace($target)) {
            continue
        }

        if ($target.StartsWith("http://") -or $target.StartsWith("https://")) {
            continue
        }

        $pathOnly = $target.Split("#")[0]
        if ([string]::IsNullOrWhiteSpace($pathOnly)) {
            continue
        }

        $resolvedTarget = [System.IO.Path]::GetFullPath((Join-Path $baseDirectory $pathOnly))
        if (-not (Test-Path $resolvedTarget)) {
            $missingLinks += [pscustomobject]@{
                Source = $relativeFile
                Target = $target
            }
        }
    }
}

if ($missingLinks.Count -gt 0) {
    Write-Host "Missing presentation doc links detected:" -ForegroundColor Yellow
    foreach ($item in $missingLinks) {
        Write-Host " - $($item.Source) -> $($item.Target)" -ForegroundColor Yellow
    }
    exit 1
}

Write-Host "Presentation doc links are valid." -ForegroundColor Green
