# RPG Maker Helpers

## Steam DLC sync

Copies RPG Maker MZ Steam DLC into repo vendor directories and stages plugin drop-ins.

```powershell
.\tools\rpgmaker\sync-steam-mz-dlc.ps1
```

Reports:
- `third_party/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_inventory.json`
- `third_party/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_packs.csv`
- `third_party/rpgmaker-mz/steam-dlc/reports/steam_mz_dlc_plugins.csv`

## Plugin drop-in validation

Validates plugin JS headers and detects duplicate stem/key conflicts across staged drop-ins.

```powershell
.\tools\rpgmaker\validate-plugin-dropins.ps1
.\tools\rpgmaker\validate-plugin-dropins.ps1 -FailOnError
```

Reports:
- `third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_summary.json`
- `third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_files.csv`
- `third_party/rpgmaker-mz/steam-dlc/reports/plugin_dropins_validation_issues.csv`
