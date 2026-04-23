# Embedding Jobs

This directory contains external embedding adapters and batch embedding jobs.

`external_embedding_adapter.py` is the first stable `command_adapter`
implementation. It reads JSON from stdin in this shape:

```json
{
  "texts": ["query or chunk text", "another chunk"],
  "dimension": 128
}
```

and writes JSON to stdout in this shape:

```json
{
  "embeddings": [[0.0, 0.1, 0.0], [0.2, 0.0, 0.0]]
}
```

Single-text requests using `text` / `embedding` remain supported for query-time
calls. The current implementation still uses deterministic hashed embeddings,
but it runs in a separate process so local model runners can replace it later
without changing manifest, bundle, or query interfaces.

When launched with `--serve`, the adapter stays warm and switches to a JSONL
protocol:

```text
{"texts":["a","b"],"dimension":128}
{"embeddings":[[...],[...]],"adapter_id":"command_adapter"}
```

This is the preferred mode for bundle builds, because it enables session reuse
and chunked batching without changing the persisted bundle contract.

The worker also supports control/status requests:

```text
{"control":"status","include_cache_stats":true}
{"status":{"backend":{"backend_id":"builtin_hashed","backend_version":"1.0"},"health":{"ok":true,"status":"ready"},"cache_stats":{"enabled":true,"entries":12,"hits":34,"misses":8}},"adapter_id":"command_adapter"}
```

That control surface is intended to stay stable as heavier local model backends
replace the current hashed placeholder.

The default backend is now `local_ngram_projection`, a local projection backend
that combines token features and character n-grams into a
deterministic normalized embedding. `builtin_hashed` remains available as a
fallback/debug backend, but new retrieval bundle jobs should prefer the
projection backend so the worker contract is exercised with a more realistic
local embedding path.
