$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")

$requiredPhrases = @(
    @{
        Path = "docs\PROGRAM_COMPLETION_STATUS.md"
        Phrase = "No remaining 100-percent lane is optional."
    },
    @{
        Path = "docs\release\100_PERCENT_COMPLETION_INVENTORY.md"
        Phrase = "Mandatory Open Lanes"
    },
    @{
        Path = "docs\APP_RELEASE_READINESS_MATRIX.md"
        Phrase = "mandatory completion-scope lock"
    },
    @{
        Path = "docs\release\RELEASE_PACKAGING.md"
        Phrase = "mandatory content backlog"
    },
    @{
        Path = "tools\shared\job_runner\README.md"
        Phrase = "vision_segmentation"
    },
    @{
        Path = "tools\shared\job_runner\README.md"
        Phrase = "audio_processing"
    }
)

$forbiddenPhrases = @(
    @{
        Path = "docs\PROGRAM_COMPLETION_STATUS.md"
        Phrase = "optional follow-on scope"
    },
    @{
        Path = "docs\release\RELEASE_PACKAGING.md"
        Phrase = "unless a future release owner"
    },
    @{
        Path = "tools\retrieval\README.md"
        Phrase = "optional offline helper dependencies"
    },
    @{
        Path = "tools\vision\README.md"
        Phrase = "optional offline tooling dependencies"
    },
    @{
        Path = "tools\audio\README.md"
        Phrase = "Planned scope:"
    },
    @{
        Path = "tools\vision\README.md"
        Phrase = "Planned scope:"
    },
    @{
        Path = "tools\retrieval\README.md"
        Phrase = "Initial planned scope:"
    }
)

$errors = @()

foreach ($entry in $requiredPhrases) {
    $path = Join-Path $repoRoot $entry.Path
    if (-not (Test-Path $path)) {
        $errors += "Missing mandatory-scope file: $($entry.Path)"
        continue
    }
    $text = Get-Content -Raw -Path $path
    if ($text -notmatch [regex]::Escape($entry.Phrase)) {
        $errors += "$($entry.Path) is missing required mandatory-scope phrase '$($entry.Phrase)'."
    }
}

foreach ($entry in $forbiddenPhrases) {
    $path = Join-Path $repoRoot $entry.Path
    if (-not (Test-Path $path)) {
        continue
    }
    $text = Get-Content -Raw -Path $path
    if ($text -match [regex]::Escape($entry.Phrase)) {
        $errors += "$($entry.Path) still contains forbidden optional/future deferral phrase '$($entry.Phrase)'."
    }
}

if ($errors.Count -gt 0) {
    throw "Mandatory completion-scope check failed:`n$($errors -join "`n")"
}

Write-Host "Mandatory completion-scope checks passed."
