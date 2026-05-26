$ErrorActionPreference = "Stop"

$forbidden = @(
    '^build-local/',
    '^Testing/Temporary/',
    '^third_party/',
    '^itch/loose/',
    '^\.urpg/',
    '^more assets/',
    '^more assets to ingest/'
)

$tracked = git ls-files
$hits = @()
foreach ($path in $tracked) {
    foreach ($pattern in $forbidden) {
        if ($path -match $pattern) {
            $hits += $path
            break
        }
    }
}

if ($hits.Count -gt 0) {
    Write-Error ("Forbidden tracked generated/local paths:`n" + ($hits | Select-Object -First 200 | Out-String))
}

Write-Host "No forbidden generated/local paths are tracked."
