# Hugging Face Vendor Area

Curated third-party samples and manifest-only references sourced from selected Hugging Face datasets.

## Layout

- `tiled-map-editor-tmx/`
  - Vendored BSD-licensed TMX fixtures.
- `visual-novel-maker-data/`
  - Vendored Apache-licensed VN Maker data fragments.
- `godot-gdscript-dataset/`
  - Vendored Apache-licensed script-corpus samples.
- `rpg-maker-mv-data/`
  - Manifest-only source; raw files are not vendored here.
- `rpg-maker-xp-data/`
  - Manifest-only source; raw files are not vendored here.

## Refresh

```powershell
.\tools\assets\ingest_huggingface_curated.ps1
.\tools\assets\ingest_huggingface_curated.ps1 -Refresh
```

See also:
- `tools/assets/huggingface_curated_manifest.json`
- `imports/reports/huggingface_curated_inventory.json`
- `docs/HUGGINGFACE_CURATED_IMPORTS.md`
