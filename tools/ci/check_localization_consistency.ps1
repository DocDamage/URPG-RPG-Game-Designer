[CmdletBinding()]
param(
    [string]$BundleRoot = "content/localization",
    [string]$SchemaPath = "content/schemas/localization_bundle.schema.json",
    [string]$ReportPath = "imports/reports/localization/localization_consistency_report.json"
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
    param([string]$PathValue)

    if ([System.IO.Path]::IsPathRooted($PathValue)) {
        return [System.IO.Path]::GetFullPath($PathValue)
    }

    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $PathValue))
}

function ConvertTo-PlainHashtable {
    param([object]$Value)

    if ($null -eq $Value) {
        return $null
    }

    if ($Value -is [System.Collections.IDictionary]) {
        $result = @{}
        foreach ($key in $Value.Keys) {
            $result[$key] = ConvertTo-PlainHashtable -Value $Value[$key]
        }

        return $result
    }

    if ($Value -is [System.Management.Automation.PSCustomObject]) {
        $result = @{}
        foreach ($property in $Value.PSObject.Properties) {
            $result[$property.Name] = ConvertTo-PlainHashtable -Value $property.Value
        }

        return $result
    }

    if ($Value -is [System.Collections.IEnumerable] -and -not ($Value -is [string])) {
        $items = New-Object System.Collections.ArrayList
        foreach ($item in $Value) {
            [void]$items.Add((ConvertTo-PlainHashtable -Value $item))
        }

        return ,$items.ToArray()
    }

    return $Value
}

function Read-JsonObject {
    param([string]$LiteralPath)

    $rawJson = Get-Content -LiteralPath $LiteralPath -Raw
    return ConvertTo-PlainHashtable -Value (ConvertFrom-Json -InputObject $rawJson)
}

function Test-BundleShape {
    param([object]$JsonValue)

    if ($null -eq $JsonValue -or -not ($JsonValue -is [System.Collections.IDictionary])) {
        return $false
    }

    if (-not $JsonValue.Contains("locale") -or -not ($JsonValue["locale"] -is [string]) -or
        [string]::IsNullOrWhiteSpace($JsonValue["locale"])) {
        return $false
    }

    if (-not $JsonValue.Contains("keys") -or -not ($JsonValue["keys"] -is [System.Collections.IDictionary])) {
        return $false
    }

    if ($JsonValue.Contains("version") -and $null -ne $JsonValue["version"] -and -not ($JsonValue["version"] -is [string])) {
        return $false
    }

    if ($JsonValue.Contains("metadata") -and $null -ne $JsonValue["metadata"] -and
        -not ($JsonValue["metadata"] -is [System.Collections.IDictionary])) {
        return $false
    }

    foreach ($value in $JsonValue["keys"].Values) {
        if (-not ($value -is [string])) {
            return $false
        }
    }

    return $true
}

function Get-AllowedTopLevelProperties {
    param([object]$SchemaJson)

    if ($null -eq $SchemaJson -or -not ($SchemaJson -is [System.Collections.IDictionary])) {
        throw "Localization schema did not parse to a JSON object."
    }

    if (-not $SchemaJson.Contains("properties") -or -not ($SchemaJson["properties"] -is [System.Collections.IDictionary])) {
        throw "Localization schema is missing a top-level 'properties' object."
    }

    return [System.Collections.Generic.HashSet[string]]::new(
        [string[]]$SchemaJson["properties"].Keys,
        [System.StringComparer]::Ordinal)
}

function Write-LocalizationReport {
    param(
        [hashtable]$Report,
        [string]$LiteralPath
    )

    $reportDirectory = Split-Path -Parent $LiteralPath
    if (-not [string]::IsNullOrWhiteSpace($reportDirectory)) {
        New-Item -ItemType Directory -Path $reportDirectory -Force | Out-Null
    }

    $Report["generatedAtUtc"] = [DateTime]::UtcNow.ToString("o")
    $jsonText = $Report | ConvertTo-Json -Depth 8
    [System.IO.File]::WriteAllText($LiteralPath, $jsonText + [Environment]::NewLine)
}

$resolvedBundleRoot = Resolve-RepoPath -PathValue $BundleRoot
$resolvedSchemaPath = Resolve-RepoPath -PathValue $SchemaPath
$resolvedReportPath = Resolve-RepoPath -PathValue $ReportPath

$report = [ordered]@{
    schemaVersion = "1.0.0"
    bundleRoot = $resolvedBundleRoot
    schemaPath = $resolvedSchemaPath
    reportPath = $resolvedReportPath
    status = "running"
    summary = [ordered]@{
        hasBundles = $false
        bundleCount = 0
        masterLocale = $null
        missingLocaleCount = 0
        missingKeyCount = 0
        extraKeyCount = 0
    }
    bundles = @()
    errors = @()
}

try {
    if (-not (Test-Path -LiteralPath $resolvedSchemaPath -PathType Leaf)) {
        throw "Localization bundle schema not found: $resolvedSchemaPath"
    }

    $schemaJson = Read-JsonObject -LiteralPath $resolvedSchemaPath
    $allowedProperties = Get-AllowedTopLevelProperties -SchemaJson $schemaJson

    if (-not (Test-Path -LiteralPath $resolvedBundleRoot -PathType Container)) {
        $report["status"] = "no_bundles"
        Write-LocalizationReport -Report $report -LiteralPath $resolvedReportPath
        Write-Host "Localization consistency check passed: no localization bundle root present at $resolvedBundleRoot; nothing to validate."
        exit 0
    }

    $bundleFiles = @(Get-ChildItem -LiteralPath $resolvedBundleRoot -Recurse -File -Filter *.json | Sort-Object FullName)
    if ($bundleFiles.Count -eq 0) {
        $report["status"] = "no_bundles"
        Write-LocalizationReport -Report $report -LiteralPath $resolvedReportPath
        Write-Host "Localization consistency check passed: no localization bundles found under $resolvedBundleRoot; nothing to validate."
        exit 0
    }

    $bundles = New-Object System.Collections.ArrayList
    $localeMap = @{}

    foreach ($file in $bundleFiles) {
        $json = Read-JsonObject -LiteralPath $file.FullName
        if (-not (Test-BundleShape -JsonValue $json)) {
            throw "Localization bundle $($file.FullName) does not match the expected bundle shape."
        }

        $unexpectedProperties = @($json.Keys | Where-Object { -not $allowedProperties.Contains([string]$_) })
        if ($unexpectedProperties.Count -gt 0) {
            throw "Localization bundle $($file.FullName) contains unexpected top-level properties: $($unexpectedProperties -join ', ')"
        }

        $localeCode = [string]$json["locale"]
        if ($localeMap.ContainsKey($localeCode)) {
            throw "Localization bundles $($localeMap[$localeCode]) and $($file.FullName) both declare locale '$localeCode'."
        }

        $localeMap[$localeCode] = $file.FullName
        [void]$bundles.Add([ordered]@{
                path = $file.FullName
                locale = $localeCode
                keyCount = @($json["keys"].Keys).Count
                missingKeys = @()
                extraKeys = @()
            })
    }

    $report["bundles"] = $bundles
    $report["summary"]["hasBundles"] = $true
    $report["summary"]["bundleCount"] = $bundles.Count

    $masterBundle = $bundles | Where-Object { $_["locale"] -eq "en" } | Select-Object -First 1
    if ($null -eq $masterBundle) {
        $masterBundle = $bundles | Sort-Object { $_["path"] } | Select-Object -First 1
    }

    $report["summary"]["masterLocale"] = $masterBundle["locale"]

    $masterJson = Read-JsonObject -LiteralPath $masterBundle["path"]
    $masterKeys = [System.Collections.Generic.HashSet[string]]::new([string[]]$masterJson["keys"].Keys, [System.StringComparer]::Ordinal)

    foreach ($bundle in $bundles) {
        if ($bundle["locale"] -eq $masterBundle["locale"]) {
            continue
        }

        $bundleJson = Read-JsonObject -LiteralPath $bundle["path"]
        $bundleKeys = [System.Collections.Generic.HashSet[string]]::new([string[]]$bundleJson["keys"].Keys, [System.StringComparer]::Ordinal)
        $missingKeys = New-Object System.Collections.ArrayList
        $extraKeys = New-Object System.Collections.ArrayList

        foreach ($key in $masterKeys) {
            if (-not $bundleKeys.Contains($key)) {
                [void]$missingKeys.Add($key)
            }
        }

        foreach ($key in $bundleKeys) {
            if (-not $masterKeys.Contains($key)) {
                [void]$extraKeys.Add($key)
            }
        }

        $bundle["missingKeys"] = [string[]]($missingKeys | Sort-Object)
        $bundle["extraKeys"] = [string[]]($extraKeys | Sort-Object)
        if ($bundle["missingKeys"].Count -gt 0) {
            $report["summary"]["missingLocaleCount"] += 1
            $report["summary"]["missingKeyCount"] += $bundle["missingKeys"].Count
        }
        if ($bundle["extraKeys"].Count -gt 0) {
            $report["summary"]["extraKeyCount"] += $bundle["extraKeys"].Count
        }
    }

    if ($report["summary"]["missingKeyCount"] -gt 0) {
        $report["status"] = "missing_keys"
        Write-LocalizationReport -Report $report -LiteralPath $resolvedReportPath
        $missingLocales = @($bundles | Where-Object { @($_["missingKeys"]).Count -gt 0 } | ForEach-Object { $_["locale"] })
        throw "Localization consistency check failed: $($report["summary"]["missingKeyCount"]) missing keys across locale(s): $($missingLocales -join ', ')."
    }

    $report["status"] = if ($report["summary"]["extraKeyCount"] -gt 0) { "passed_with_extra_keys" } else { "passed" }
    Write-LocalizationReport -Report $report -LiteralPath $resolvedReportPath
    Write-Host "Localization consistency check passed for $($bundles.Count) bundle(s) under $resolvedBundleRoot using master locale '$($masterBundle["locale"])'."
} catch {
    $report["status"] = "failed"
    $report["errors"] = @($_.Exception.Message)
    Write-LocalizationReport -Report $report -LiteralPath $resolvedReportPath
    throw
}
