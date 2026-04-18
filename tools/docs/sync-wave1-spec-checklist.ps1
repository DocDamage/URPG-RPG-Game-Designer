param(
    [switch]$Check
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

$startMarker = "<!-- WAVE1_CHECKLIST_START -->"
$endMarker = "<!-- WAVE1_CHECKLIST_END -->"
$canonicalPath = Join-Path $repoRoot "docs\WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md"

if (-not (Test-Path $canonicalPath)) {
    throw "Canonical checklist not found: $canonicalPath"
}

$canonicalRel = "WAVE1_SUBSYSTEM_CLOSURE_CHECKLIST.md"

$specs = @(
    @{
        Name = "UI / Menu Core"
        Path = "docs/UI_MENU_CORE_NATIVE_SPEC.md"
        Specific = @(
            "Menu runtime route/command ownership is authoritative and no plugin command string owns route resolution.",
            "Menu authoring surface (structure/layout/command/preview) supports inspect/edit/validate workflows.",
            "Fallback routes and command-state migration paths are schema-defined and test-backed."
        )
    },
    @{
        Name = "Message / Text Core"
        Path = "docs/MESSAGE_TEXT_CORE_NATIVE_SPEC.md"
        Specific = @(
            "MessageScene-native renderer ownership is authoritative (compat window bridge no longer primary).",
            "Text layout + alignment behavior is deterministic across runtime and preview, including wrapped snapshot anchors.",
            "Escape/token/schema migration and diagnostics remain explicit, typed, and test-backed."
        )
    },
    @{
        Name = "Save / Data Core"
        Path = "docs/SAVE_DATA_CORE_NATIVE_SPEC.md"
        Specific = @(
            "Save catalog/serializer/recovery ownership is authoritative and separate from UI presentation concerns.",
            "Autosave, recovery tier escalation, and safe-mode behavior are policy-owned and diagnostics-backed.",
            "Compat metadata/import upgrade mappings are typed, versioned, and test-backed."
        )
    },
    @{
        Name = "Battle Core"
        Path = "docs/BATTLE_CORE_NATIVE_SPEC.md"
        Specific = @(
            "Battle flow/action/rule ownership is authoritative; HUD and overlays consume battle state only.",
            "Deterministic turn/escape/action outcomes are preserved under runtime and preview validation anchors.",
            "Battle schema + migration + diagnostics closure is complete and release-evidence-backed."
        )
    }
)

function New-ChecklistSection {
    param(
        [string]$SubsystemName,
        [string[]]$SubsystemSpecific
    )

    $lines = @()
    $lines += "## Wave 1 Closure Checklist (Canonical)"
    $lines += ""
    $lines += '_Managed by `tools/docs/sync-wave1-spec-checklist.ps1`. Do not edit manually._'
    $lines += "_Canonical source: [$canonicalRel]($canonicalRel)_"
    $lines += ""
    $lines += "### Universal closure gates"
    $lines += ""
    $lines += "- [ ] Runtime ownership is authoritative and compat behavior for this subsystem is bridge-only."
    $lines += "- [ ] Editor productization is complete (inspect/edit/preview/validate) with diagnostics surfaced."
    $lines += "- [ ] Schema contracts and migration/import paths are explicit, versioned, and test-backed."
    $lines += "- [ ] Deterministic validation exists (unit + integration + snapshot where layout/presentation applies)."
    $lines += "- [ ] Failure-path diagnostics and safe-mode/bounded fallback behavior are explicitly documented and tested."
    $lines += "- [ ] Release evidence is published in status docs and gate snapshots are recorded."
    $lines += ""
    $lines += "### $SubsystemName specific closure gates"
    $lines += ""
    foreach ($item in $SubsystemSpecific) {
        $lines += "- [ ] $item"
    }
    $lines += ""
    $lines += "### Closure sign-off artifact checklist"
    $lines += ""
    $lines += "- [ ] Runtime owner files listed (header + source)."
    $lines += "- [ ] Editor owner files listed."
    $lines += "- [ ] Schema and migration files listed."
    $lines += "- [ ] Latest deterministic test outputs recorded."
    $lines += "- [ ] `README.md`, `docs/PROGRAM_COMPLETION_STATUS.md`, and `URPG_Blueprint_v3_1_Integrated.md` updated."

    return ($lines -join "`r`n")
}

$changed = @()

foreach ($spec in $specs) {
    $path = Join-Path $repoRoot $spec.Path
    if (-not (Test-Path $path)) {
        throw "Spec not found: $($spec.Path)"
    }

    $text = Get-Content -Path $path -Raw
    $generated = New-ChecklistSection -SubsystemName $spec.Name -SubsystemSpecific $spec.Specific
    $replacementBlock = "$startMarker`r`n$generated`r`n$endMarker"

    $newText = $text
    if ($text.Contains($startMarker) -and $text.Contains($endMarker)) {
        $pattern = [regex]::Escape($startMarker) + "(?s).*?" + [regex]::Escape($endMarker)
        $newText = [regex]::Replace($text, $pattern, [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacementBlock }, 1)
    } else {
        $insert = "$replacementBlock`r`n`r`n"
        $anchor = "## Non-goals for this slice"
        if ($text.Contains($anchor)) {
            $newText = $text.Replace($anchor, $insert + $anchor)
        } else {
            $newText = $text.TrimEnd() + "`r`n`r`n" + $insert
        }
    }

    if ($newText -ne $text) {
        $changed += $spec.Path
        if (-not $Check) {
            Set-Content -Path $path -Value $newText -NoNewline
        }
    }
}

if ($Check) {
    if ($changed.Count -gt 0) {
        Write-Host "Wave 1 spec checklist drift detected:" -ForegroundColor Yellow
        foreach ($file in $changed) {
            Write-Host " - $file" -ForegroundColor Yellow
        }
        Write-Host "Run tools/docs/sync-wave1-spec-checklist.ps1 to synchronize." -ForegroundColor Yellow
        exit 1
    }
    Write-Host "Wave 1 spec checklists are synchronized." -ForegroundColor Green
    exit 0
}

if ($changed.Count -eq 0) {
    Write-Host "No checklist updates required." -ForegroundColor Green
} else {
    Write-Host "Updated Wave 1 checklists in:" -ForegroundColor Cyan
    foreach ($file in $changed) {
        Write-Host " - $file" -ForegroundColor Cyan
    }
}
