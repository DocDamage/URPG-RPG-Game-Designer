# Asset Import Debt Paydown Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Pay down the highest-risk technical debt in the global asset import workflow without expanding product scope.

**Architecture:** Keep Python tooling responsible for source import/extraction and C++ editor code responsible for deterministic request/snapshot contracts. Replace ad hoc shell/process behavior with explicit argv contracts, preserve fail-closed import semantics, and split tests by workflow after behavior is locked.

**Tech Stack:** C++20 editor model/panel, Catch2, Python 3 unittest tooling, CMake/Ninja, Markdown documentation.

---

## File Structure

- `tools/assets/global_asset_import.py`: owns external archive extraction, extractor command parsing from CLI/env, and import-session manifest generation.
- `tools/assets/tests/test_global_asset_import.py`: Python importer regression tests for extraction failure, config parsing, nested archives, and CLI behavior.
- `editor/assets/asset_library_model.h`: C++ model contracts for import wizard snapshots, conversion process execution, and Add Source handoff.
- `editor/assets/asset_library_model.cpp`: C++ implementation for extractor config snapshot, import handoff command generation, conversion process runner, project picker rows, and model refresh.
- `editor/assets/asset_library_panel.h`: typed panel snapshot shape for the import wizard.
- `editor/assets/asset_library_panel.cpp`: panel render snapshot mapping and native picker dispatch.
- `tests/unit/test_asset_library_panel.cpp`: current monolithic test source; split after behavior changes are protected.
- `tests/unit/test_asset_library_import_wizard.cpp`: new focused tests for wizard snapshot, picker, Add Source handoff, and extractor config.
- `tests/unit/test_asset_library_conversion.cpp`: new focused tests for conversion execution and conversion-driven promotion unblock behavior.
- `tests/unit/test_asset_library_promotion_attachment.cpp`: new focused tests for promotion, global promotion, project attachment, and picker rows.
- `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`: status/debt notes for current asset import behavior.
- `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`: canonical design status and implementation caveats.

---

### Task 1: Make Failed External Extraction Fail Closed

**Files:**
- Modify: `tools/assets/global_asset_import.py`
- Test: `tools/assets/tests/test_global_asset_import.py`
- Docs: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [ ] **Step 1: Write the failing test**

Add this test to `GlobalAssetImportTests` after `test_optional_external_archive_extractor_invocation`:

```python
    def test_failed_external_archive_extraction_does_not_catalog_partial_outputs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "pack.7z"
            archive.write_bytes(b"not really 7z")
            extractor = root / "partial_extractor.py"
            extractor.write_text(
                "import pathlib, sys\n"
                "out = pathlib.Path(sys.argv[2]) / 'img' / 'characters'\n"
                "out.mkdir(parents=True, exist_ok=True)\n"
                "(out / 'Partial.png').write_bytes(b'\\x89PNG\\r\\n\\x1a\\n' + b'0' * 32)\n"
                "raise SystemExit(2)\n",
                encoding="utf-8",
            )

            session = global_asset_import.build_session(
                source=archive,
                library_root=root / ".urpg" / "asset-library",
                session_id="import_7z_partial_failure",
                license_note="User-provided test license.",
                max_files=100,
                max_bytes=1024 * 1024,
                external_extractor_command=[sys.executable, str(extractor)],
            )

            self.assertEqual(session["sourceKind"], "external_archive")
            self.assertEqual(session["status"], "failed")
            self.assertEqual(session["diagnostics"][0]["code"], "external_extractor_failed")
            self.assertEqual(session["summary"]["filesScanned"], 0)
            self.assertEqual(session["records"], [])
```

- [ ] **Step 2: Run the red test**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
```

Expected: FAIL because the importer currently scans partial extractor output and reports one record.

- [ ] **Step 3: Implement fail-closed scanning**

In `tools/assets/global_asset_import.py`, add a helper near `build_session`:

```python
FATAL_IMPORT_DIAGNOSTICS = {
    "unsafe_archive_path",
    "archive_read_failed",
    "import_file_count_limit_exceeded",
    "import_byte_limit_exceeded",
    "external_extractor_missing",
    "external_extractor_timeout",
    "external_extractor_failed",
}


def has_fatal_diagnostic(diagnostics: list[dict]) -> bool:
    return any(diagnostic.get("code") in FATAL_IMPORT_DIAGNOSTICS for diagnostic in diagnostics)
```

Then change `build_session` so records are scanned only when there are no fatal diagnostics:

```python
    records = [] if has_fatal_diagnostic(diagnostics) else scan_records(
        scan_root,
        session_id,
        source.stem if source.is_file() else source.name,
        license_note,
    )
    sequence_groups = assemble_sequence_groups(records, session_id)
    status = "review_ready" if not has_fatal_diagnostic(diagnostics) else "failed"
```

Remove the local `fatal_codes` set from `build_session`.

- [ ] **Step 4: Verify green**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
```

Expected: PASS.

- [ ] **Step 5: Update docs**

In `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`, add a bullet under Phase 4:

```markdown
- failed external extractor runs are fail-closed: partial extractor output is left out of review records when the
  extractor reports a fatal error.
```

- [ ] **Step 6: Commit**

Run:

```powershell
git add tools/assets/global_asset_import.py tools/assets/tests/test_global_asset_import.py docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
git commit -m "Fail closed on partial archive extraction"
```

---

### Task 2: Unify Extractor Command Parsing Around Structured Argv

**Files:**
- Modify: `tools/assets/global_asset_import.py`
- Modify: `editor/assets/asset_library_model.cpp`
- Test: `tools/assets/tests/test_global_asset_import.py`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Docs: `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`

- [ ] **Step 1: Add Python parser tests**

Add these tests to `GlobalAssetImportTests` near `test_cli_uses_configured_external_extractor_from_environment`:

```python
    def test_configured_external_extractor_command_ignores_whitespace_only_environment(self) -> None:
        with mock.patch.dict(os.environ, {"URPG_ASSET_ARCHIVE_EXTRACTOR": "   \t  "}):
            self.assertIsNone(global_asset_import.configured_external_extractor_command(None))

    def test_configured_external_extractor_command_rejects_unclosed_quotes(self) -> None:
        with mock.patch.dict(os.environ, {"URPG_ASSET_ARCHIVE_EXTRACTOR": '"C:/Program Files/7-Zip/7z.exe'}):
            with self.assertRaises(ValueError):
                global_asset_import.configured_external_extractor_command(None)
```

- [ ] **Step 2: Add C++ parser status tests**

Add this test near the existing extractor configuration tests in `tests/unit/test_asset_library_panel.cpp`:

```cpp
TEST_CASE("AssetLibraryModel reports invalid external archive extractor configuration",
          "[assets][asset_library][editor][asset_import][wizard][archive]") {
    EnvironmentVariableGuard extractorEnv("URPG_ASSET_ARCHIVE_EXTRACTOR");
    extractorEnv.set("\"C:/Program Files/7-Zip/7z.exe");

    urpg::editor::AssetLibraryModel model;

    const auto& configuration = model.snapshot().import_wizard["extractor_configuration"];
    REQUIRE(configuration["configured"] == false);
    REQUIRE(configuration["source"] == "environment");
    REQUIRE(configuration["environment_variable"] == "URPG_ASSET_ARCHIVE_EXTRACTOR");
    REQUIRE(configuration["supports_rar_7z"] == false);
    REQUIRE(configuration["diagnostics"][0] == "external_extractor_command_parse_error");
}
```

- [ ] **Step 3: Run tests red**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel reports invalid external archive extractor configuration" --output-on-failure
```

Expected: Python raises `ValueError` or returns an empty list inconsistently, and C++ reports configured instead of invalid.

- [ ] **Step 4: Implement Python parser result**

Change `configured_external_extractor_command` in `tools/assets/global_asset_import.py`:

```python
def configured_external_extractor_command(cli_value: str | None) -> list[str] | None:
    command = cli_value if cli_value is not None else os.environ.get(EXTERNAL_EXTRACTOR_ENV)
    if command is None or not command.strip():
        return None
    try:
        return shlex.split(command)
    except ValueError as exc:
        raise ValueError(f"{EXTERNAL_EXTRACTOR_ENV} could not be parsed: {exc}") from exc
```

In `main`, catch this parse error and write a failed manifest with `external_extractor_config_invalid` only if the source exists:

```python
    try:
        external_extractor_command = configured_external_extractor_command(args.external_extractor_command)
    except ValueError as exc:
        raise SystemExit(str(exc))
```

- [ ] **Step 5: Implement C++ parse diagnostics**

In `editor/assets/asset_library_model.cpp`, introduce:

```cpp
struct ParsedExternalExtractorCommand {
    std::vector<std::string> arguments;
    bool valid = true;
    std::string diagnostic;
};
```

Change `splitConfiguredCommand` to return `ParsedExternalExtractorCommand`. At end of parsing, if `quote != '\0'` or `escaping` is true, return:

```cpp
return {{}, false, "external_extractor_command_parse_error"};
```

Change `configuredExternalExtractorCommand` to return `{}` when invalid. Change `externalExtractorConfigurationSnapshot` to include:

```cpp
{"diagnostics", parsed.valid ? nlohmann::json::array() : nlohmann::json::array({parsed.diagnostic})}
```

Set `configured` and `supports_rar_7z` to false when invalid.

- [ ] **Step 6: Verify green**

Run:

```powershell
python tools/assets/tests/test_global_asset_import.py
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel reports invalid external archive extractor configuration|AssetLibraryModel exposes configured archive extractor status|AssetLibraryModel applies configured external archive extractor" --output-on-failure
```

Expected: PASS.

- [ ] **Step 7: Update docs**

In `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`, add:

```markdown
Malformed extractor configuration must surface as `external_extractor_command_parse_error` in the wizard snapshot and
must not be treated as configured RAR/7z support.
```

- [ ] **Step 8: Commit**

Run:

```powershell
git add tools/assets/global_asset_import.py tools/assets/tests/test_global_asset_import.py editor/assets/asset_library_model.cpp tests/unit/test_asset_library_panel.cpp docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md
git commit -m "Validate archive extractor configuration"
```

---

### Task 3: Replace Shell-Based Conversion Execution With An Argv Process Runner

**Files:**
- Modify: `editor/assets/asset_library_model.h`
- Modify: `editor/assets/asset_library_model.cpp`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Docs: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [ ] **Step 1: Add conversion runner test for preserving process CWD**

Add this test near `AssetLibraryModel runs audio conversion handoff and unblocks promotion`:

```cpp
TEST_CASE("AssetLibraryModel conversion runner preserves process working directory",
          "[assets][asset_library][editor][asset_import][conversion]") {
    const auto original = std::filesystem::current_path();
    const auto root = uniqueTempRoot("urpg_asset_library_conversion_cwd");
    const auto sourceRoot = root / "sources" / "import_audio";
    std::filesystem::create_directories(sourceRoot / "audio" / "bgm");
    writeBinaryFile(sourceRoot / "audio" / "bgm" / "theme.ogg", "ogg");

    urpg::assets::AssetImportSession session;
    session.sessionId = "import_audio_cwd";
    session.managedSourceRoot = sourceRoot.generic_string();
    session.status = urpg::assets::AssetImportStatus::ReviewReady;
    session.records.push_back(urpg::assets::AssetImportRecord{
        "asset.theme",
        "audio/bgm/theme.ogg",
        "asset://import/audio/bgm/theme.ogg",
        ".ogg",
        "audio",
        "audio/bgm",
        "pack",
        "sha",
        3,
        0,
        0,
        0,
        false,
        "",
        false,
        false,
        false,
        false,
        {"conversion_required"},
        true,
        "converted/audio/bgm/theme.wav",
        {"python", "-c", "from pathlib import Path; Path('converted/audio/bgm').mkdir(parents=True, exist_ok=True); Path('converted/audio/bgm/theme.wav').write_bytes(b'wav')"},
        "",
        -1,
        0,
        true,
        "audio",
        ""});

    urpg::editor::AssetLibraryModel model;
    model.ingestImportSession(session);

    const auto result = model.runImportRecordConversion("import_audio_cwd", "asset.theme");

    REQUIRE(result["success"] == true);
    REQUIRE(std::filesystem::current_path() == original);
}
```

- [ ] **Step 2: Run test red if current runner leaks or remains shell-only**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel conversion runner preserves process working directory" --output-on-failure
```

Expected: The test may pass because current code restores CWD. If it passes, continue with the process abstraction refactor under the existing conversion tests.

- [ ] **Step 3: Add process runner abstraction**

In `AssetLibraryModel` in `editor/assets/asset_library_model.h`, add:

```cpp
using ProcessCommandExecutor = std::function<ConversionCommandResult(const ConversionCommand&)>;
static ConversionCommandResult runConversionCommand(const ConversionCommand& command);
```

In `editor/assets/asset_library_model.cpp`, replace `runConversionCommandWithSystem` with `AssetLibraryModel::runConversionCommand`. On Windows, use `CreateProcessW` with `lpCurrentDirectory` set to `command.working_directory`. On non-Windows, use `std::system` as a guarded fallback but do not change `std::filesystem::current_path`.

For the Windows implementation, build a quoted command line from argv:

```cpp
std::wstring commandLine = joinWindowsCommandLine(command.arguments);
STARTUPINFOW startup{};
PROCESS_INFORMATION process{};
const BOOL launched = CreateProcessW(
    nullptr,
    commandLine.data(),
    nullptr,
    nullptr,
    FALSE,
    CREATE_NO_WINDOW,
    nullptr,
    command.working_directory.wstring().c_str(),
    &startup,
    &process);
```

Wait for the process, read the exit code, close handles, and return `{static_cast<int>(exitCode), "", ""}`.

- [ ] **Step 4: Wire model conversion through the new runner**

Change:

```cpp
const auto result = executor ? executor(command) : runConversionCommandWithSystem(command);
```

to:

```cpp
const auto result = executor ? executor(command) : AssetLibraryModel::runConversionCommand(command);
```

- [ ] **Step 5: Verify conversion tests**

Run:

```powershell
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel runs audio conversion handoff and unblocks promotion|AssetLibraryPanel dispatches selected import conversions through the wizard|AssetLibraryModel conversion runner preserves process working directory" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update docs**

In `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`, change the conversion bullet to mention argv process execution:

```markdown
- conversion-needed audio records execute through an argv process runner with an explicit working directory instead of
  mutating process-global current directory.
```

- [ ] **Step 7: Commit**

Run:

```powershell
git add editor/assets/asset_library_model.h editor/assets/asset_library_model.cpp tests/unit/test_asset_library_panel.cpp docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
git commit -m "Run asset conversions with explicit process context"
```

---

### Task 4: Resolve Import Tool Paths For Packaged Editor Contexts

**Files:**
- Modify: `editor/assets/asset_library_model.h`
- Modify: `editor/assets/asset_library_model.cpp`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Docs: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [ ] **Step 1: Add a request override test**

Add this test near `AssetLibraryModel requests add-source import command handoff`:

```cpp
TEST_CASE("AssetLibraryModel request import source supports configured importer paths",
          "[assets][asset_library][editor][asset_import][wizard]") {
    const auto root = uniqueTempRoot("urpg_asset_library_add_source_tool_paths");
    const auto source = root / "fantasy_pack.zip";
    const auto libraryRoot = root / ".urpg" / "asset-library";

    urpg::editor::AssetLibraryModel model;
    model.setImportToolCommand({"C:/Tools/Python/python.exe", "C:/URPG/tools/assets/global_asset_import.py"});

    const auto request = model.requestImportSource(
        source, libraryRoot, "import_tool_paths_001", "User-provided test license.");

    REQUIRE(request["command"][0] == "C:/Tools/Python/python.exe");
    REQUIRE(request["command"][1] == "C:/URPG/tools/assets/global_asset_import.py");
    REQUIRE(model.snapshot().import_wizard["pending_request"]["command"][0] == "C:/Tools/Python/python.exe");
}
```

- [ ] **Step 2: Run test red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel request import source supports configured importer paths" --output-on-failure
```

Expected: compile fail because `setImportToolCommand` does not exist.

- [ ] **Step 3: Add model configuration API**

In `editor/assets/asset_library_model.h`, add:

```cpp
void setImportToolCommand(std::vector<std::string> command_prefix);
```

Add a private member:

```cpp
std::vector<std::string> import_tool_command_ = {"python", "tools/assets/global_asset_import.py"};
```

In `editor/assets/asset_library_model.cpp`, implement:

```cpp
void AssetLibraryModel::setImportToolCommand(std::vector<std::string> command_prefix) {
    if (command_prefix.size() >= 2) {
        import_tool_command_ = std::move(command_prefix);
    }
    refreshSnapshot();
}
```

Change `requestImportSource` command construction to start from `import_tool_command_`:

```cpp
nlohmann::json command = nlohmann::json::array();
for (const auto& part : import_tool_command_) {
    command.push_back(part);
}
command.push_back("--source");
```

- [ ] **Step 4: Verify handoff tests**

Run:

```powershell
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryModel request import source supports configured importer paths|AssetLibraryModel requests add-source import command handoff|AssetLibraryModel requests add-source with external archive extractor handoff" --output-on-failure
```

Expected: PASS.

- [ ] **Step 5: Document tool command configuration**

In `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`, add:

```markdown
The editor model keeps the Add Source importer command prefix configurable so packaged builds can resolve the Python
runtime and importer script without depending on the process working directory.
```

- [ ] **Step 6: Commit**

Run:

```powershell
git add editor/assets/asset_library_model.h editor/assets/asset_library_model.cpp tests/unit/test_asset_library_panel.cpp docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
git commit -m "Configure asset import tool command path"
```

---

### Task 5: Make Non-Windows Native Picker Unsupported State Explicit

**Files:**
- Modify: `editor/assets/asset_library_panel.cpp`
- Modify: `editor/assets/asset_library_panel.h`
- Test: `tests/unit/test_asset_library_panel.cpp`
- Docs: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`

- [ ] **Step 1: Add a platform-independent picker capability test**

Add this test near picker tests:

```cpp
TEST_CASE("AssetLibraryPanel exposes native import picker availability",
          "[assets][asset_library][editor][asset_import][wizard]") {
    const auto availability = urpg::editor::AssetLibraryPanel::nativeImportSourcePickerAvailability();

#ifdef _WIN32
    REQUIRE(availability.available == true);
    REQUIRE(availability.code == "native_import_source_picker_available");
#else
    REQUIRE(availability.available == false);
    REQUIRE(availability.code == "native_import_source_picker_unsupported");
#endif
}
```

- [ ] **Step 2: Run red**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryPanel exposes native import picker availability" --output-on-failure
```

Expected: compile fail because the API does not exist.

- [ ] **Step 3: Add availability API**

In `editor/assets/asset_library_panel.h`, add:

```cpp
struct ImportSourcePickerAvailability {
    bool available = false;
    std::string code;
    std::string message;
};

static ImportSourcePickerAvailability nativeImportSourcePickerAvailability();
```

In `editor/assets/asset_library_panel.cpp`, implement:

```cpp
AssetLibraryPanel::ImportSourcePickerAvailability AssetLibraryPanel::nativeImportSourcePickerAvailability() {
#ifdef _WIN32
    return {true, "native_import_source_picker_available", "Native Windows import source picker is available."};
#else
    return {false, "native_import_source_picker_unsupported", "Native import source picker is not implemented on this platform."};
#endif
}
```

- [ ] **Step 4: Include picker availability in cancellation result**

In `requestImportSourceFromPicker`, when no source is selected and no injected picker exists, add:

```cpp
{"picker_availability", {
    {"available", nativeImportSourcePickerAvailability().available},
    {"code", nativeImportSourcePickerAvailability().code},
    {"message", nativeImportSourcePickerAvailability().message},
}},
```

- [ ] **Step 5: Verify picker tests**

Run:

```powershell
ctest --test-dir build/dev-ninja-debug -R "AssetLibraryPanel exposes native import picker availability|AssetLibraryPanel requests add-source through an import source picker" --output-on-failure
```

Expected: PASS.

- [ ] **Step 6: Update docs**

In `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`, add:

```markdown
Native source picker availability is explicit in panel diagnostics; Windows has `IFileOpenDialog`, while other
platforms report `native_import_source_picker_unsupported` until a platform picker is implemented.
```

- [ ] **Step 7: Commit**

Run:

```powershell
git add editor/assets/asset_library_panel.h editor/assets/asset_library_panel.cpp tests/unit/test_asset_library_panel.cpp docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md
git commit -m "Expose native import picker availability"
```

---

### Task 6: Split Asset Library Tests By Workflow

**Files:**
- Modify: `tests/unit/test_asset_library_panel.cpp`
- Create: `tests/unit/test_asset_library_import_wizard.cpp`
- Create: `tests/unit/test_asset_library_conversion.cpp`
- Create: `tests/unit/test_asset_library_promotion_attachment.cpp`
- Modify: `CMakeLists.txt` or relevant test source list if unit tests are explicitly enumerated.

- [ ] **Step 1: Inspect current test registration**

Run:

```powershell
rg -n "test_asset_library_panel.cpp|urpg_tests|tests/unit" CMakeLists.txt cmake tests
```

Expected: identify whether new test files are picked up by glob or must be added explicitly.

- [ ] **Step 2: Move shared helpers to a local test helper header**

Create `tests/unit/asset_library_test_helpers.h`:

```cpp
#pragma once

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

inline std::filesystem::path uniqueAssetLibraryTempRoot(const std::string& prefix) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (prefix + "_" + std::to_string(tick));
}

inline void writeAssetLibraryBinaryFile(const std::filesystem::path& path, std::string_view payload) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << payload;
}

class AssetLibraryEnvironmentVariableGuard {
  public:
    explicit AssetLibraryEnvironmentVariableGuard(const char* name) : name_(name) {
        if (const char* value = std::getenv(name_)) {
            original_ = value;
        }
    }

    ~AssetLibraryEnvironmentVariableGuard() {
        if (original_) {
            _putenv_s(name_, original_->c_str());
        } else {
            _putenv_s(name_, "");
        }
    }

    void set(const std::string& value) const { _putenv_s(name_, value.c_str()); }

  private:
    const char* name_;
    std::optional<std::string> original_;
};
```

- [ ] **Step 3: Move wizard tests**

Create `tests/unit/test_asset_library_import_wizard.cpp` and move tests covering:

- empty panel snapshot
- import sessions/review queues
- wizard steps/actions
- wizard action dispatch
- picker request/cancel
- Add Source command handoff
- external extractor handoff/configuration
- native picker availability

Include:

```cpp
#include "editor/assets/asset_library_panel.h"
#include "asset_library_test_helpers.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
```

Rename helper uses from `uniqueTempRoot` to `uniqueAssetLibraryTempRoot` and `writeBinaryFile` to `writeAssetLibraryBinaryFile`.

- [ ] **Step 4: Move conversion tests**

Create `tests/unit/test_asset_library_conversion.cpp` and move:

- conversion-needed promotion blocking assertions
- `AssetLibraryModel runs audio conversion handoff and unblocks promotion`
- `AssetLibraryPanel dispatches selected import conversions through the wizard`
- conversion CWD/process runner test from Task 3

Use the same helper header.

- [ ] **Step 5: Move promotion/attachment tests**

Create `tests/unit/test_asset_library_promotion_attachment.cpp` and move:

- promote selected records
- bulk promotion
- global promotion
- attach promoted assets
- reload promoted/global/project attachment manifests
- project asset picker row assertions

Use the same helper header.

- [ ] **Step 6: Keep panel/report tests in original file**

Leave these in `tests/unit/test_asset_library_panel.cpp`:

- cleanup preview summary
- load error snapshot remediation
- canonical report directory shape
- optional local promotion catalog
- filters and used-by reference counts
- promote/archive action state
- governed promotion manifest action rows

Remove duplicate helper definitions from the original file and include `asset_library_test_helpers.h`.

- [ ] **Step 7: Update CMake if needed**

If Step 1 shows explicit test source listing, add:

```cmake
tests/unit/test_asset_library_import_wizard.cpp
tests/unit/test_asset_library_conversion.cpp
tests/unit/test_asset_library_promotion_attachment.cpp
```

to the `urpg_tests` source list.

- [ ] **Step 8: Verify all moved tests still run**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibrary|asset_library|asset_attachment|AssetImportSession" --output-on-failure
```

Expected: PASS with the same behavioral coverage under more focused source files.

- [ ] **Step 9: Commit**

Run:

```powershell
git add tests/unit/asset_library_test_helpers.h tests/unit/test_asset_library_panel.cpp tests/unit/test_asset_library_import_wizard.cpp tests/unit/test_asset_library_conversion.cpp tests/unit/test_asset_library_promotion_attachment.cpp CMakeLists.txt
git commit -m "Split asset library workflow tests"
```

---

### Task 7: Final Validation And Documentation Closure

**Files:**
- Modify: `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`
- Modify: `docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md`

- [ ] **Step 1: Add technical debt closure notes**

In `docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md`, add a short section:

```markdown
## Asset Import Debt Paydown

The post-Phase 5 debt pass tightened fail-closed external extraction, extractor configuration diagnostics, explicit
process context for conversion commands, configurable importer command paths, native picker availability diagnostics,
and split workflow tests by responsibility.
```

- [ ] **Step 2: Run full focused validation**

Run:

```powershell
cmake --build --preset dev-debug --target urpg_tests
ctest --test-dir build/dev-ninja-debug -R "AssetLibrary|asset_library|asset_attachment|AssetImportSession|global_asset_import|ExportPackager.*asset" --output-on-failure
python tools/assets/tests/test_global_asset_import.py
git diff --check
```

Expected: PASS.

- [ ] **Step 3: Inspect worktree**

Run:

```powershell
git status --short --branch
```

Expected: only intentional docs/test/source changes are present.

- [ ] **Step 4: Commit and push**

Run:

```powershell
git add docs/asset_intake/ASSET_LIBRARY_AND_MORE_ASSETS_INTAKE.md docs/superpowers/specs/2026-04-30-global-asset-library-import-design.md
git commit -m "Document asset import debt paydown"
git push
```

Expected: branch pushes cleanly.

---

## Review Checklist

- Fail-closed extraction is covered by Python tests.
- Malformed extractor configuration is visible in both importer behavior and wizard snapshot diagnostics.
- Conversion execution no longer mutates process-global working directory on Windows.
- Add Source handoff no longer assumes packaged editor runs from the repository root.
- Native picker support is explicit on unsupported platforms.
- Asset library tests are split by workflow and remain discoverable by CTest.
- Docs describe the current behavior without overstating product scope.
