# Offline ML Environment

This directory holds helper environment definitions for offline retrieval, vision, and audio tooling.

Rules:

- do not reference this environment from the shipped runtime build
- keep heavyweight dependencies out of CMake/native player paths
- use this environment only for offline jobs under `tools/`
