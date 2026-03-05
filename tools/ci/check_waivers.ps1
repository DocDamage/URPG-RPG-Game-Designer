Param(
    [string]$Path = "tools/ci/known_break_waivers.json"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $Path)) {
    Write-Error "Waiver file not found: $Path"
    exit 1
}

$raw = Get-Content $Path -Raw
$data = $raw | ConvertFrom-Json

if ($null -eq $data.waivers) {
    Write-Error "Invalid waiver file: missing waivers array"
    exit 1
}

$today = [DateTime]::UtcNow.Date
$hasError = $false

for ($i = 0; $i -lt $data.waivers.Count; $i++) {
    $w = $data.waivers[$i]

    if ([string]::IsNullOrWhiteSpace($w.id)) {
        Write-Host "Waiver[$i] missing id"
        $hasError = $true
    }
    if ([string]::IsNullOrWhiteSpace($w.issue_url)) {
        Write-Host "Waiver[$i] missing issue_url"
        $hasError = $true
    }
    if ([string]::IsNullOrWhiteSpace($w.scope)) {
        Write-Host "Waiver[$i] missing scope"
        $hasError = $true
    }
    if ([string]::IsNullOrWhiteSpace($w.expires_on)) {
        Write-Host "Waiver[$i] missing expires_on"
        $hasError = $true
        continue
    }

    try {
        $expiry = [DateTime]::Parse($w.expires_on).ToUniversalTime().Date
        if ($expiry -lt $today) {
            Write-Host "Waiver[$($w.id)] expired on $($w.expires_on)"
            $hasError = $true
        }
    } catch {
        Write-Host "Waiver[$($w.id)] has invalid expires_on: $($w.expires_on)"
        $hasError = $true
    }
}

if ($hasError) {
    exit 1
}

Write-Host "Waiver file validation passed."
