# Retrieval Pipeline

The offline retrieval pipeline indexes project text and imported metadata for authoring-time search and debugging.

Current boundary rules:

- indexing and embedding jobs stay under `tools/retrieval`
- runtime receives exported bundles only when there is a validated need
- manifests must preserve source paths and chunk IDs

Initial target sources:

- lore
- dialogue
- quests
- item descriptions
- imported project metadata
