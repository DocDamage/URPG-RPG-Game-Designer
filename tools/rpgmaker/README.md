# RPG Maker Helpers

## Steam DLC sync

Copies RPG Maker MZ Steam DLC into repo vendor directories and stages plugin drop-ins.

```powershell
.\tools\rpgmaker\sync-steam-mz-dlc.ps1
```

By default, sync now also runs curated drop-in generation and curated validation.
Use `-SkipCurate` and/or `-SkipCuratedValidation` only when you intentionally want to bypass that pipeline.

Reports:
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_inventory.json`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_packs.csv`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_plugins.csv`

## Plugin drop-in validation

Validates plugin JS headers and detects duplicate stem/key conflicts across release drop-ins. With no
`-PluginRoot`, the validator uses the curated release tree when present and falls back to raw staged
DLC only before curation has been generated. Use `-PluginRoot imports\raw\third_party_assets\rpgmaker-mz\steam-dlc\plugin-dropins\js\plugins`
when auditing raw vendor intake collisions.

```powershell
.\tools\rpgmaker\validate-plugin-dropins.ps1
.\tools\rpgmaker\validate-plugin-dropins.ps1 -FailOnError
```

Reports:
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_summary.json`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_files.csv`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_issues.csv`

## Curated safe drop-ins

Builds a conflict-free curated plugin set (one canonical JS per plugin key) and exports selection/conflict reports.

```powershell
.\tools\rpgmaker\curate-plugin-dropins.ps1 -CleanOutput
.\tools\rpgmaker\validate-plugin-dropins.ps1 -PluginRoot imports\raw\third_party_assets\rpgmaker-mz\steam-dlc\plugin-dropins-curated\js\plugins -ReportPrefix plugin_dropins_curated_validation -FailOnError
```

Curated output:
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/plugin-dropins-curated/js/plugins/*.js`

Reports:
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_summary.json`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_manifest.csv`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_conflicts.csv`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_release_manifest.json`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_validation_summary.json`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_validation_files.csv`
- `imports/raw/third_party_assets/rpgmaker-mz/steam-dlc/reports/plugin_dropins_curated_validation_issues.csv`
