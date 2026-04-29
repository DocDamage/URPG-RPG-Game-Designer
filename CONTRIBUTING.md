# Contributing

This repo contains source code and a large volume of binary game assets.
Please follow these rules so we keep merges safe and the repo healthy.

## 1) Local setup

```powershell
git lfs install
python -m pip install pre-commit
pre-commit install
```

The C++ pre-commit hooks require `clang-format` and `clang-tidy` on `PATH`.
Run `cmake --preset dev-ninja-debug` before the full hook suite so clang-tidy can read `build/dev-ninja-debug/compile_commands.json`.
The C++ hooks enforce changed and untracked files; this avoids turning legacy formatting debt into unrelated release churn while still rejecting new regressions.
clang-tidy analyzes a deterministic bounded subset by default for fast local feedback; set `URPG_CLANG_TIDY_MAX_FILES=0` when you need full changed-file static coverage.
On Windows MinGW builds, the local clang-tidy hook skips analysis unless `URPG_REQUIRE_CLANG_TIDY=1` is set because Windows LLVM can mix MSVC headers into MinGW SDL translation units; Linux CI enforces the hook strictly.

## 2) Before opening a PR

```powershell
.\tools\ci\run_local_gates.ps1
cmake --preset dev-ninja-debug
pre-commit run --all-files
```

If your change touches presentation runtime, spatial editor tooling, or presentation-facing rendering behavior, also run:

```powershell
.\tools\ci\run_presentation_gate.ps1
```

If you are touching RPG Maker MZ DLC content, also run:

```powershell
.\tools\rpgmaker\validate-plugin-dropins.ps1
```

If you are importing or reorganizing assets, also run:

```powershell
python .\tools\assets\asset_hygiene.py --write-reports
```

Use the PR checklist in `.github/PULL_REQUEST_TEMPLATE.md` when writing your verification summary.
Presentation documentation links are validated automatically by `.\tools\ci\run_local_gates.ps1`.

## 3) Binary and asset policy

- Large binary formats must be tracked in Git LFS (`.gitattributes` is the source of truth).
- Do not commit temporary or OS-noise files (`__MACOSX`, `.DS_Store`, `Thumbs.db`, `._*`).
- Do not commit partial downloads.
- Prefer preserving vendor/source folder names for traceability.

## 4) Repository organization

- Keep imports, quarantined source drops, and third-party/vendor asset packs under `imports/`.
- Put generated reports under `imports/reports/` or tool-specific `reports/` folders.
- Avoid mixing executable tools and content assets in the same directory.

## 5) Commit hygiene

- Keep commits focused (one concern per commit).
- Include a short verification note in the commit message body when useful.
- Never rewrite or delete user-authored changes you did not make unless explicitly requested.
