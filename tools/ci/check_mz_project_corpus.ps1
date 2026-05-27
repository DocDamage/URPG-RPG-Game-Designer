$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$schemaPath = Join-Path $repoRoot "content\compat\mz_project_corpus.schema.json"
$corpusDir = Join-Path $repoRoot "imports\fixtures\compat\mz_projects"
$allowedLegalUse = @("repo_owned", "permissive_sample", "owner_provided_private")
$errors = New-Object System.Collections.Generic.List[string]

if (-not (Test-Path $schemaPath)) {
    $errors.Add("Missing MZ project corpus schema: content/compat/mz_project_corpus.schema.json")
}
else {
    try {
        $schema = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
        if ($schema.'$id' -ne "https://urpg.dev/schemas/mz_project_corpus.schema.json") {
            $errors.Add("MZ project corpus schema has unexpected `$id.")
        }
    }
    catch {
        $errors.Add("MZ project corpus schema is not valid JSON: $_")
    }
}

if (-not (Test-Path $corpusDir)) {
    $errors.Add("Missing MZ project corpus directory: imports/fixtures/compat/mz_projects")
}
else {
    $descriptorFiles = Get-ChildItem -Path $corpusDir -Filter "*.json" -File
    if ($descriptorFiles.Count -eq 0) {
        $errors.Add("MZ project corpus has no descriptor JSON files.")
    }

    foreach ($file in $descriptorFiles) {
        try {
            $descriptor = Get-Content -Raw -Path $file.FullName | ConvertFrom-Json
        }
        catch {
            $errors.Add("Descriptor '$($file.Name)' is not valid JSON: $_")
            continue
        }

        foreach ($field in @(
            "schemaVersion",
            "projectId",
            "sourceLicense",
            "legalUse",
            "fixtureScope",
            "maps",
            "events",
            "plugins",
            "saves",
            "assets",
            "expectedCoverage"
        )) {
            if (-not $descriptor.PSObject.Properties[$field]) {
                $errors.Add("Descriptor '$($file.Name)' is missing required field '$field'.")
            }
        }

        if ($descriptor.schemaVersion -ne "1.0.0") {
            $errors.Add("Descriptor '$($file.Name)' has unsupported schemaVersion '$($descriptor.schemaVersion)'.")
        }
        if ($allowedLegalUse -notcontains $descriptor.legalUse) {
            $errors.Add("Descriptor '$($file.Name)' has unsupported legalUse '$($descriptor.legalUse)'.")
        }
        if ([string]::IsNullOrWhiteSpace($descriptor.sourceLicense)) {
            $errors.Add("Descriptor '$($file.Name)' must name a sourceLicense.")
        }
        if ($descriptor.fixtureScope.copyrightedRpgMakerPayloadsIncluded -ne $false) {
            $errors.Add("Descriptor '$($file.Name)' must not include copyrighted RPG Maker payloads.")
        }
        if ($descriptor.plugins.fixtureManifestDirectory -and
            ($descriptor.plugins.fixtureManifestDirectory -match '^\.\.' -or
             [System.IO.Path]::IsPathRooted($descriptor.plugins.fixtureManifestDirectory))) {
            $errors.Add("Descriptor '$($file.Name)' plugin fixture path must be repo-relative and non-rooted.")
        }
        if ($descriptor.expectedCoverage.runtimeParityClaim -eq $true) {
            $errors.Add("Descriptor '$($file.Name)' cannot claim runtime parity in the corpus descriptor lane.")
        }
        if ($descriptor.expectedCoverage.visualParityClaim -eq $true) {
            $errors.Add("Descriptor '$($file.Name)' cannot claim visual parity in the corpus descriptor lane.")
        }
    }
}

if ($errors.Count -gt 0) {
    foreach ($errorMessage in $errors) {
        Write-Error $errorMessage
    }
    exit 1
}

Write-Host "MZ project corpus descriptors passed."
