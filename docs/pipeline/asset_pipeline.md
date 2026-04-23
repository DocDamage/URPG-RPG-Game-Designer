# Asset Pipeline

The offline asset pipeline is responsible for preparing source assets into stable exported artifacts that the runtime can consume directly.

Current boundary rules:

- segmentation and tagging stay under `tools/vision`
- importer-specific preprocessing stays under `tools/importers`
- runtime consumes exported cutouts, manifests, and metadata only

Initial target outputs:

- cutout images
- masks
- per-asset manifests
- importer-normalized metadata
