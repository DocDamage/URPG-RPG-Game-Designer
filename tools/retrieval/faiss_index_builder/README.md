# FAISS Index Builder

This directory is the first retrieval-tooling landing zone.

Current scaffold:

- chunk manifest generation for project text and metadata
- deterministic retrieval bundle generation from chunk manifests
- adapter-driven bundle builds so a real embedding backend can replace the built-in path later
- job-runner-compatible CLI entrypoint

Later additions can layer real embeddings and FAISS index generation on top of the same manifest contract.
