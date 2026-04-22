$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$errors = @()

$requiredFiles = @(
    "content\schemas\input_bindings.schema.json",
    "content\schemas\controller_bindings.schema.json",
    "engine\core\input\input_core.h",
    "engine\core\input\input_remap_store.h",
    "engine\core\input\input_remap_store.cpp",
    "engine\core\action\controller_binding_runtime.h",
    "engine\core\action\controller_binding_runtime.cpp",
    "editor\action\controller_binding_panel.h",
    "editor\action\controller_binding_panel.cpp",
    "tests\unit\test_input_remap_store.cpp",
    "tests\unit\test_controller_binding_runtime.cpp",
    "tests\unit\test_controller_binding_panel.cpp",
    "tools\ci\check_input_governance.ps1",
    "content\fixtures\input_bindings_fixture.json",
    "content\fixtures\controller_bindings_fixture.json"
)

foreach ($relPath in $requiredFiles) {
    $fullPath = Join-Path $repoRoot $relPath
    if (-not (Test-Path $fullPath)) {
        $errors += "Missing required input artifact: $relPath"
    }
}

$schemaPath = Join-Path $repoRoot "content\schemas\input_bindings.schema.json"
if (Test-Path $schemaPath) {
    try {
        $null = Get-Content -Raw -Path $schemaPath | ConvertFrom-Json
    } catch {
        $errors += "Input bindings schema is not valid JSON: $_"
    }
}

$controllerSchemaPath = Join-Path $repoRoot "content\schemas\controller_bindings.schema.json"
if (Test-Path $controllerSchemaPath) {
    try {
        $null = Get-Content -Raw -Path $controllerSchemaPath | ConvertFrom-Json
    } catch {
        $errors += "Controller bindings schema is not valid JSON: $_"
    }
}

$fixturePath = Join-Path $repoRoot "content\fixtures\input_bindings_fixture.json"
if (Test-Path $fixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $fixturePath | ConvertFrom-Json
        if (-not $fixture.version) {
            $errors += "Input bindings fixture is missing version"
        }
        if (-not $fixture.bindings) {
            $errors += "Input bindings fixture is missing bindings"
        } elseif (-not ($fixture.bindings -is [System.Array])) {
            $errors += "Input bindings fixture bindings must be an array"
        } elseif ($fixture.bindings.Count -eq 0) {
            $errors += "Input bindings fixture must declare at least one binding"
        }

        foreach ($binding in @($fixture.bindings)) {
            if ($null -eq $binding.keyCode) {
                $errors += "Input bindings fixture contains a binding without keyCode"
                break
            }
            if (-not $binding.action) {
                $errors += "Input bindings fixture contains a binding without action"
                break
            }
        }
    } catch {
        $errors += "Input bindings fixture is not valid JSON: $_"
    }
}

$controllerFixturePath = Join-Path $repoRoot "content\fixtures\controller_bindings_fixture.json"
if (Test-Path $controllerFixturePath) {
    try {
        $fixture = Get-Content -Raw -Path $controllerFixturePath | ConvertFrom-Json
        if (-not $fixture.version) {
            $errors += "Controller bindings fixture is missing version"
        }
        if (-not $fixture.bindings) {
            $errors += "Controller bindings fixture is missing bindings"
        } elseif (-not ($fixture.bindings -is [System.Array])) {
            $errors += "Controller bindings fixture bindings must be an array"
        } elseif ($fixture.bindings.Count -eq 0) {
            $errors += "Controller bindings fixture must declare at least one binding"
        }

        foreach ($binding in @($fixture.bindings)) {
            if (-not $binding.button) {
                $errors += "Controller bindings fixture contains a binding without button"
                break
            }
            if (-not $binding.action) {
                $errors += "Controller bindings fixture contains a binding without action"
                break
            }
        }
    } catch {
        $errors += "Controller bindings fixture is not valid JSON: $_"
    }
}

$result = @{
    passed = $errors.Count -eq 0
    errors = $errors
}

$result | ConvertTo-Json -Depth 4

if ($errors.Count -gt 0) {
    exit 1
}
