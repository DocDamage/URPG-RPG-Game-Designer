$ErrorActionPreference = "Stop"

$hits = rg -n 'std::system\s*\(|(^|[^[:alnum:]_:])system\s*\(' engine editor apps runtimes
if ($LASTEXITCODE -eq 0) {
    Write-Error ("Production system-call use remains:`n" + ($hits | Out-String))
}

Write-Host "No production system-call use found."
