# Quality Gates

Use the narrowest command that covers the changed surface. If a release plan specifies a command, run that command exactly.

`ctest -R` matching is case-sensitive. Use the casing from discovered CTest names (`ctest --test-dir <build-dir> -N`) and avoid lowercase or underscore aliases unless those aliases are present in the discovered test list.

## Command Map

| Change Area | Command |
| --- | --- |
| General PR-level changes | `ctest --preset dev-all -L pr --output-on-failure` or `ctest -L pr --output-on-failure` from the build dir |
| Runtime startup/settings/input | `ctest --preset dev-all -R "startup|settings|input" --output-on-failure` |
| Map scene/render assets | `ctest --preset dev-all -R "MapScene|AssetLoader|Runtime map asset" --output-on-failure` |
| Battle assets/authoring | `ctest --preset dev-all -R "battle.*assets|battle.*authoring" --output-on-failure` |
| Compat JS / WindowCompat / plugin fixtures | `ctest -L weekly --output-on-failure` |
| Export packager/validator | `ctest --preset dev-all -R "ExportPackager|urpg_pack_cli|export_validator" --output-on-failure` |
| Presentation/spatial/rendering | `.\tools\ci\run_presentation_gate.ps1` |
| WYSIWYG readiness/done-rule changes | `ctest --preset dev-all -R "WYSIWYG|readiness_status|truth" --output-on-failure` |
| Native package layout | `.\tools\ci\check_package_smoke.ps1 -BuildDirectory build/dev-ninja-release -PackageRoot build/package-smoke` |
| Full local gate | `.\tools\ci\run_local_gates.ps1` |

## Knowledge Health

Run this when editing `AGENTS.md`, `docs/agent/`, or release execution plans:

```powershell
.\tools\docs\check-agent-knowledge.ps1 -BuildDirectory build/dev-ninja-debug
```

The check enforces that `AGENTS.md` stays small, required agent index docs exist, and runnable `ctest -R` commands in active markdown docs match at least one discovered test when a build directory is available. For alternations, each top-level or grouped branch must independently match a discovered test.
