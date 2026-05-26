# Finish Current Codebase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the current post-`v0.1.0` blockers so the repository can make a broader production/completion claim with fresh evidence.

**Architecture:** Treat this as a sequence of independent hardening lanes with fail-closed gates. Security and lifetime fixes land before product-scope expansion; docs and CI guards are updated in the same task as each behavior change so stale claims cannot reappear.

**Tech Stack:** C++20, CMake/Ninja, Catch2, PowerShell CI scripts, QuickJS C API, ImGui editor panels, Git LFS, JSON manifests, Python asset tooling.

---

## Current Blocker Summary

- Production-adjacent `std::system` execution remains in AI, creator-command, analytics, achievement, and non-Windows asset conversion paths.
- AI and analytics shell transports place bearer tokens in command-line arguments.
- Compat `AudioManager` stores raw role pointers to channels owned by `std::unique_ptr`.
- `ChatbotComponent` callbacks capture raw `this` and do not have cancellation semantics.
- QuickJS memory limits exist, but CPU limits do not use `JS_SetInterruptHandler`.
- Bundle protection is lightweight RLE/XOR plus keyed digest; `obfuscateScript()` is a stub helper.
- Cloud sync is local-only in-tree.
- Native import source picker is Windows-only outside injectable tests.
- The checkout tracks generated/local and retired-root paths: `build-local/`, `Testing/Temporary/`, root `third_party/`, and root `itch/loose/`.
- `git lfs ls-files --name-only` reports 32,229 LFS-tracked normalized asset paths in current `development`.

## File Map

- Create: `engine/core/platform/process_runner.h`
- Create: `engine/core/platform/process_runner.cpp`
- Modify: `CMakeLists.txt`
- Modify: `engine/core/ai/openai_compatible_chat_service.*`
- Modify: `engine/core/ai/creator_command_planner.*`
- Modify: `engine/core/analytics/analytics_uploader.*`
- Modify: `engine/core/achievement/achievement_platform_backend.*`
- Modify: `editor/assets/asset_library_model.*`
- Modify: `runtimes/compat_js/audio_manager.*`
- Modify: `engine/core/message/chatbot_component.h`
- Modify: `engine/core/message/mock_chat_service.h`
- Modify: `engine/core/ai/openai_compatible_chat_service.*`
- Modify: `runtimes/compat_js/quickjs_runtime.*`
- Modify: `engine/core/security/resource_protector.h`
- Modify: `engine/core/tools/export_packager*`
- Modify: `engine/core/export/*`
- Modify: `engine/core/social/cloud_service.h`
- Modify: `editor/assets/asset_library_panel.*`
- Modify: `tools/assets/global_asset_import.py`
- Create: `tools/ci/check_no_generated_tracked_files.ps1`
- Create: `tools/ci/check_no_production_system_calls.ps1`
- Create: `tools/ci/check_lfs_release_scope.ps1`
- Modify: `tools/ci/run_local_gates.ps1`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Test: focused Catch2 tests under `tests/unit/`
- Test: PowerShell guard tests where local patterns already exist

### Task 1: Repository Truth Guards

**Files:**
- Create: `tools/ci/check_no_generated_tracked_files.ps1`
- Create: `tools/ci/check_no_production_system_calls.ps1`
- Create: `tools/ci/check_lfs_release_scope.ps1`
- Modify: `tools/ci/run_local_gates.ps1`
- Modify: `docs/agent/QUALITY_GATES.md`

- [ ] **Step 1: Add generated-path guard**

Create `tools/ci/check_no_generated_tracked_files.ps1`:

```powershell
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
```

- [ ] **Step 2: Add production system-call guard**

Create `tools/ci/check_no_production_system_calls.ps1`:

```powershell
$ErrorActionPreference = "Stop"
$hits = rg -n 'std::system|\bsystem\s*\(' engine editor apps runtimes
if ($LASTEXITCODE -eq 0) {
  Write-Error ("Production system-call use remains:`n" + ($hits | Out-String))
}
Write-Host "No production system-call use found."
```

- [ ] **Step 3: Add LFS scope guard**

Create `tools/ci/check_lfs_release_scope.ps1`:

```powershell
$ErrorActionPreference = "Stop"
$lfs = git lfs ls-files --name-only | Where-Object { $_ -ne "" }
$releaseRequired = Get-Content content/fixtures/project_governance_fixture.json -Raw | ConvertFrom-Json
$requiredPaths = @()
foreach ($asset in $releaseRequired.releaseAssets.assets) {
  if ($asset.path) { $requiredPaths += $asset.path }
}
$releaseLfs = $lfs | Where-Object { $requiredPaths -contains $_ }
if ($releaseLfs.Count -gt 0) {
  Write-Error ("Release-required assets are still LFS-tracked:`n" + ($releaseLfs | Out-String))
}
Write-Host "Release-required assets are not LFS-tracked. Total LFS paths in checkout: $($lfs.Count)"
```

- [ ] **Step 4: Wire guards into local gates**

Add the three scripts to `tools/ci/run_local_gates.ps1` near the other hygiene checks, using the existing `Invoke-Step` or local wrapper pattern in that file.

- [ ] **Step 5: Verify guards fail on current tree**

Run:

```powershell
.\tools\ci\check_no_generated_tracked_files.ps1
.\tools\ci\check_no_production_system_calls.ps1
.\tools\ci\check_lfs_release_scope.ps1
```

Expected before cleanup: generated-path and system-call guards fail; LFS scope guard reports total LFS paths and fails only if release-required assets are LFS-tracked.

### Task 2: Remove Shell Execution From Production Code

**Files:**
- Create: `engine/core/platform/process_runner.h`
- Create: `engine/core/platform/process_runner.cpp`
- Modify: `CMakeLists.txt`
- Modify: `engine/core/achievement/achievement_platform_backend.cpp`
- Modify: `editor/assets/asset_library_model.cpp`
- Test: `tests/unit/test_process_runner.cpp`
- Test: `tests/unit/test_achievement_registry.cpp`
- Test: `tests/unit/test_asset_library_conversion.cpp`

- [ ] **Step 1: Add ProcessRunner interface**

Create `engine/core/platform/process_runner.h`:

```cpp
#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace urpg::platform {

struct ProcessCommand {
    std::filesystem::path executable;
    std::vector<std::string> arguments;
    std::filesystem::path workingDirectory;
    std::map<std::string, std::string> environment;
    std::chrono::milliseconds timeout{30000};
    bool captureStdout = true;
    bool captureStderr = true;
};

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
    bool timedOut = false;
    std::string error;
};

ProcessResult runProcess(const ProcessCommand& command);

} // namespace urpg::platform
```

- [ ] **Step 2: Implement no-shell platform process execution**

Implement Windows with `CreateProcessW` and POSIX with `posix_spawnp` or `fork` plus `execve`. The command must never call `cmd.exe`, PowerShell, `/bin/sh`, or `std::system`.

- [ ] **Step 3: Replace achievement command backend**

Change `CommandAchievementPlatformBackend::submitProgress()` to build `ProcessCommand{m_executable, argv}` and pass payload, achievement id, and platform as argv entries. Remove shell redirection.

- [ ] **Step 4: Replace asset conversion fallback**

Change `AssetLibraryModel::runConversionCommand()` to use `urpg::platform::runProcess()` on every platform.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "ProcessRunner|Achievement|AssetLibrary.*conversion" --output-on-failure
.\tools\ci\check_no_production_system_calls.ps1
```

Expected after Task 3: the guard may still fail because AI/analytics shell transports remain.

### Task 3: Replace Curl Shell Transports With Native HTTP

**Files:**
- Create: `engine/core/net/http_client.h`
- Create: `engine/core/net/http_client.cpp`
- Modify: `CMakeLists.txt`
- Modify: `engine/core/ai/openai_compatible_chat_service.*`
- Modify: `engine/core/ai/creator_command_planner.*`
- Modify: `engine/core/analytics/analytics_uploader.*`
- Test: `tests/unit/test_openai_compatible_chat_service.cpp`
- Test: `tests/unit/test_analytics_dispatcher.cpp`

- [ ] **Step 1: Add HTTP client boundary**

Create a small `HttpRequest`, `HttpResponse`, and injectable `IHttpClient` interface. Keep headers in memory, not argv or diagnostics.

- [ ] **Step 2: Implement deterministic fake client for tests**

Add a test-only fake that records redacted header names and response bodies without storing bearer token values.

- [ ] **Step 3: Update OpenAI-compatible chat**

Replace `buildOpenAiCompatibleChatCurlCommand()` execution with native `IHttpClient::postJson()`. Keep dry-run request-body diagnostics but redact `api_key` and never expose Authorization values in JSON snapshots.

- [ ] **Step 4: Update creator command transport**

Use the same HTTP client and redaction rules for `invokeCreatorProvider()`.

- [ ] **Step 5: Update analytics upload**

Replace `AnalyticsUploader::setHttpJsonEndpoint()` curl command construction with `IHttpClient`.

- [ ] **Step 6: Verify**

Run:

```powershell
ctest --preset dev-all -R "OpenAiCompatible|AnalyticsUploader|creator" --output-on-failure
rg -n "Authorization: Bearer|api_key|bearerToken" engine tests docs
.\tools\ci\check_no_production_system_calls.ps1
```

Expected: no production `std::system`; no unredacted bearer value in snapshots or tests.

### Task 4: Fix Compat Audio Channel Lifetime

**Files:**
- Modify: `runtimes/compat_js/audio_manager.cpp`
- Modify: `runtimes/compat_js/audio_manager.h`
- Test: `tests/unit/test_audio_manager.cpp`

- [ ] **Step 1: Replace role raw pointers**

Change role members from `AudioChannel*` to `std::optional<uint32_t>`:

```cpp
std::optional<uint32_t> bgmChannelId_;
std::optional<uint32_t> bgsChannelId_;
std::optional<uint32_t> meChannelId_;
std::vector<uint32_t> seChannelIds_;
```

- [ ] **Step 2: Add resolver helper**

Add a private helper that resolves IDs through `channelIndex_` or channel map and returns a short-lived local pointer only for immediate use.

- [ ] **Step 3: Clear role IDs on destroy**

In `destroyChannel(uint32_t id)`, clear matching BGM/BGS/ME IDs and erase matching SE IDs before erasing the owning channel.

- [ ] **Step 4: Add regressions**

Add tests for destroying active BGM, BGS, ME, and SE followed by `update()`, `stop*()`, diagnostics, and repeated destroy.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "AudioManager" --output-on-failure
cmake --preset dev-ninja-debug -DURPG_SANITIZERS=address,undefined
cmake --build --preset dev-debug
ctest --preset dev-all -R "AudioManager" --output-on-failure
```

Expected: focused tests pass under sanitizer configuration.

### Task 5: Add Chatbot Cancellation And Lifetime Safety

**Files:**
- Modify: `engine/core/message/chatbot_component.h`
- Modify: `engine/core/message/mock_chat_service.h`
- Modify: `engine/core/ai/openai_compatible_chat_service.*`
- Test: `tests/unit/test_chatbot_component.cpp`
- Test: `tests/unit/test_scene_manager.cpp`

- [ ] **Step 1: Add cancellation handle**

Extend `IChatService` with a lightweight cancellation token:

```cpp
class ChatRequestHandle {
  public:
    void cancel();
    bool cancelled() const;
};
```

Return `std::shared_ptr<ChatRequestHandle>` from `requestResponse()` and `requestStream()`.

- [ ] **Step 2: Make ChatbotComponent weak-callback safe**

Construct `ChatbotComponent` through `std::shared_ptr` or add an owner token captured by callbacks. The callback must return without touching members if the component was destroyed or the request was cancelled.

- [ ] **Step 3: Update MapScene ownership**

Change `MapScene::m_activeChatbot` from `std::unique_ptr` to `std::shared_ptr` if `enable_shared_from_this` is used.

- [ ] **Step 4: Add delayed fake service**

Add a fake service that stores callbacks, destroy the component, then fire the callback. Assert no history mutation or crash occurs.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "Chatbot|MapScene.*chat" --output-on-failure
```

Expected: delayed callbacks after destruction are ignored.

### Task 6: Enforce QuickJS CPU And Async Resource Limits

**Files:**
- Modify: `runtimes/compat_js/quickjs_runtime.h`
- Modify: `runtimes/compat_js/quickjs_runtime.cpp`
- Test: `tests/unit/test_quickjs_runtime.cpp`

- [ ] **Step 1: Add deadline state**

Store per-context deadline and cancellation flag in `QuickJSContextImpl`.

- [ ] **Step 2: Install interrupt handler**

Call `JS_SetInterruptHandler(runtime, interruptHandler, impl.get())` during initialization. The handler returns nonzero when the deadline or cancellation flag is exceeded.

- [ ] **Step 3: Bound pending jobs**

Update `drainPendingJobs()` to cap total jobs per frame, emit diagnostics when the cap is hit, and preserve existing exception diagnostics.

- [ ] **Step 4: Add regressions**

Add tests for `while (true) {}`, recursive promise/job scheduling, timer flood if timers are enabled, and memory over-limit behavior.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "QuickJS" --output-on-failure
ctest --preset dev-all -L weekly --output-on-failure
```

Expected: infinite-loop tests fail closed with CPU-budget diagnostics instead of hanging.

### Task 7: Make Bundle And Script Protection Truthful

**Files:**
- Modify: `engine/core/security/resource_protector.h`
- Modify: `engine/core/tools/export_packager_bundle_writer.cpp`
- Modify: `engine/core/tools/export_packager_payload_builder.cpp`
- Modify: `engine/core/export/export_bundle_contract.cpp`
- Test: `tests/unit/test_export_packager_bundles.cpp`
- Test: `tests/unit/test_runtime_bundle_loader.cpp`

- [ ] **Step 1: Rename lightweight mode**

Keep `rle_xor` only as `lightweight_obfuscation`, or preserve the serialized value while every user-facing diagnostic says "lightweight, not encrypted".

- [ ] **Step 2: Fail closed for script obfuscation**

Remove or make private `obfuscateScript()` until a real transform exists. Export requests with `obfuscateScripts=true` must keep returning a hard failure explaining that no script transform is implemented.

- [ ] **Step 3: Add stronger bundle authenticity path**

Implement HMAC-SHA256 at minimum for keyed authenticity. If public verification is required for distribution, implement Ed25519 in a separate task and document key ownership.

- [ ] **Step 4: Add tamper tests**

Test manifest tamper, payload tamper, nonce/key-id tamper if present, signature mismatch, and replay of payload under a different manifest scope.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-export --output-on-failure
rg -n "Script Logic Obfuscator \\(Stub\\)|shipping-hardened|encrypted" engine docs
```

Expected: docs and diagnostics no longer imply unsupported protection.

### Task 8: Close Cloud Sync Or Keep It Hidden With Tests

**Files:**
- Modify: `engine/core/social/cloud_service.h`
- Modify: `engine/core/message/ai_sync_coordinator.*`
- Modify: `editor/*`
- Test: `tests/unit/test_ai_sync_coordinator.cpp`

- [ ] **Step 1: Decide release scope**

Choose one path for this branch: keep cloud sync hidden/local-only, or implement a real persistent provider plus fake remote integration.

- [ ] **Step 2A: Hidden/local-only path**

If keeping hidden, add release UI tests that scan editor release surfaces and assert no cloud/cross-device sync controls are visible unless `releaseVisibility().release_visible && remote_transport`.

- [ ] **Step 2B: Real provider path**

If implementing a provider, add `LocalFilesystemCloudService` and `FakeRemoteCloudService` with credentials, encrypted cache, sync journal, tombstones, conflict detection, retries, quota/auth diagnostics, and two-client merge tests.

- [ ] **Step 3: Verify**

Run:

```powershell
ctest --preset dev-all -R "cloud|AISync|releaseVisibility|editor app panels" --output-on-failure
rg -n "cloud sync|cross-device|LocalInMemoryCloudService" README.md docs engine editor
```

Expected: release docs and UI agree with the implemented provider scope.

### Task 9: Finish Cross-Platform Native Import Picker

**Files:**
- Modify: `editor/assets/asset_library_panel.cpp`
- Modify: `editor/assets/asset_library_panel.h`
- Test: `tests/unit/test_asset_library_import_wizard.cpp`

- [ ] **Step 1: Keep injectable picker as test boundary**

Do not remove the existing injectable picker; tests should continue driving picker outcomes without platform UI.

- [ ] **Step 2: Add macOS picker**

Implement an Objective-C++ bridge file for `NSOpenPanel` and compile it only on Apple platforms.

- [ ] **Step 3: Add Linux picker**

Implement `xdg-desktop-portal` first. Add GTK/KDialog only as fallback when the portal is unavailable.

- [ ] **Step 4: Add platform availability diagnostics**

Expose diagnostic codes for `native_import_source_picker_available`, `native_import_source_picker_portal_missing`, and `native_import_source_picker_unsupported`.

- [ ] **Step 5: Verify**

Run:

```powershell
ctest --preset dev-all -R "AssetLibrary.*Import|native_import_source_picker" --output-on-failure
```

Expected: Windows remains covered, unsupported platforms report precise diagnostics, and injectable tests cover every branch.

### Task 10: Reconcile Assets, LFS, And Retired Root Folders

**Files:**
- Modify: `.gitattributes`
- Modify: `.gitignore`
- Modify: `imports/manifests/asset_bundles/*.json`
- Modify: `imports/reports/asset_intake/*`
- Modify: `docs/asset_intake/*`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Inventory tracked drift**

Run:

```powershell
git ls-files build-local Testing/Temporary third_party "itch/loose" > imports/reports/asset_intake/tracked_path_drift_2026-05-26.txt
git lfs ls-files --name-only > imports/reports/asset_intake/lfs_paths_2026-05-26.txt
```

- [ ] **Step 2: Remove generated/local tracked files**

Run a reviewed removal:

```powershell
git rm -r --cached build-local Testing/Temporary
```

For `third_party/` and `itch/loose/`, either move governed payloads to documented import paths or remove them from tracking after confirming no release/test fixtures reference them.

- [ ] **Step 3: Reconcile LFS policy**

For each LFS-tracked normalized path, choose one status: release-required normal Git blob, release-eligible LFS payload with hydration gate, non-release optional payload, or remove from tracking. Update bundle manifests and docs accordingly.

- [ ] **Step 4: Verify**

Run:

```powershell
.\tools\ci\check_no_generated_tracked_files.ps1
.\tools\ci\check_lfs_release_scope.ps1
.\tools\ci\check_asset_library_governance.ps1
.\tools\ci\check_release_required_assets.ps1
```

Expected: no forbidden generated/local paths remain tracked; release-required assets are not unresolved LFS pointers.

### Task 11: Harden External Archive Extraction

**Files:**
- Modify: `tools/assets/global_asset_import.py`
- Modify: `tools/assets/tests/test_global_asset_import.py`

- [ ] **Step 1: Add extractor allowlist**

Allow only configured executable basenames such as `7z`, `7zz`, `bsdtar`, or repo-test fake extractors under the temporary test directory.

- [ ] **Step 2: Add post-extract containment audit**

Reject symlinks, hardlinks where detectable, devices, absolute paths, and any resolved output outside the target root.

- [ ] **Step 3: Add tests**

Create tests for a malicious extractor that writes outside the destination, creates a symlink, creates too many files, and exits nonzero after partial output.

- [ ] **Step 4: Verify**

Run:

```powershell
python -m unittest tools.assets.tests.test_global_asset_import
```

Expected: malicious external extractor behavior is rejected before catalog records are created.

### Task 12: Final Recertification And Documentation Pass

**Files:**
- Modify: `README.md`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/release/AAA_RELEASE_READINESS_REPORT.md`
- Modify: `docs/release/RELEASE_READINESS_MATRIX.md`

- [ ] **Step 1: Update docs from evidence only**

Update `READY`, `VERIFIED`, `OPEN`, and `PARTIAL` rows only after running the matching command in the same change set.

- [ ] **Step 2: Run source hygiene**

Run:

```powershell
.\tools\ci\check_no_generated_tracked_files.ps1
.\tools\ci\check_no_production_system_calls.ps1
.\tools\ci\check_lfs_release_scope.ps1
git diff --check
```

- [ ] **Step 3: Run security/lifetime tests**

Run:

```powershell
ctest --preset dev-all -R "ProcessRunner|OpenAiCompatible|AnalyticsUploader|Achievement|AudioManager|Chatbot|QuickJS|Export" --output-on-failure
```

- [ ] **Step 4: Run release gates**

Run:

```powershell
.\tools\ci\check_release_required_assets.ps1
.\tools\ci\check_install_smoke.ps1 -BuildDirectory build/dev-ninja-release -InstallPrefix build/install-smoke
.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke
.\tools\ci\run_local_gates.ps1
```

- [ ] **Step 5: Record final state**

Update status docs with exact dates, commands, exit results, and remaining human/external constraints. Do not publish a broader release claim until all open blockers above are closed or explicitly descoped by release owner.

## Self-Review

- Spec coverage: covers documentation truth, process execution, credential redaction, audio lifetime, chatbot lifetime, QuickJS limits, asset/script protection, cloud sync, native picker portability, asset/LFS hygiene, archive extraction, and final gates.
- Red-flag scan: no vague implementation markers are intentionally left in this plan.
- Type consistency: all new APIs use the names declared in the file map: `ProcessCommand`, `ProcessResult`, `runProcess`, `IHttpClient`, `ChatRequestHandle`, and existing URPG subsystem names.
