# Retrieval Tooling

Offline retrieval and indexing tools live here.

Initial planned scope:

- chunk manifest generation
- FAISS-compatible index building
- local query/debug tooling
- lightweight built-in retrieval bundle generation before FAISS is installed
- pluggable embedding adapters that preserve the same manifest/bundle/query boundary
- external `command_adapter` support for local model runners behind the same bundle contract
- persistent `command_adapter` worker support with chunked batching for heavier local embedding models
- `local_ngram_projection` as the default command adapter backend, with `builtin_hashed` available for fallback/debug and `optional_sentence_transformer` available when optional tooling dependencies are installed

## Quick smoke check

Use this recipe to verify adapter activation and fallback quickly:

```powershell
$manifestPath = "C:\temp\retrieval_smoke_manifest.json"
$bundlePath = "C:\temp\retrieval_smoke_bundle.json"

@"
{
  "schema": "content/schemas/retrieval_chunk_manifest.schema.json",
  "source_root": ".",
  "chunks": [
    {
      "chunk_id": "smoke-001",
      "source_path": "tools/retrieval/README.md",
      "chunk_index": 0,
      "text": "URPG retrieval smoke chunk for local adapter activation checks."
    }
  ]
}
"@ | Set-Content -Encoding UTF8 $manifestPath

python tools/retrieval/faiss_index_builder/build_retrieval_bundle.py `
  --manifest $manifestPath `
  --output $bundlePath `
  --adapter local_ngram_projection

python tools/retrieval/faiss_index_builder/build_retrieval_bundle.py `
  --manifest $manifestPath `
  --output $bundlePath `
  --adapter optional_sentence_transformer
if ($LASTEXITCODE -ne 0) {
  Write-Host "Optional backend unavailable; falling back to builtin_hashed."
  python tools/retrieval/faiss_index_builder/build_retrieval_bundle.py `
    --manifest $manifestPath `
    --output $bundlePath `
    --adapter builtin_hashed
}
```

This keeps the smoke command self-contained and shows the intended adapter
activation flow:

- `local_ngram_projection` now resolves to the command adapter automatically.
- `optional_sentence_transformer` uses the same flow when dependencies are available.
- `builtin_hashed` remains deterministic fallback/debug mode.

## Focused unit test

Run the new adapter-resolution guard tests directly with:

```powershell
python tools/retrieval/tests/test_build_retrieval_bundle.py
```

This test is also registered in the top-level `pr` CTest lane as:

- `urpg_retrieval_adapter_resolution_test` (labels: `pr`)

The runtime must not depend directly on retrieval-building libraries from this directory.
