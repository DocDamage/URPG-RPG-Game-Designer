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

The runtime must not depend directly on retrieval-building libraries from this directory.
