# Mandatory 100 Percent Scope Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make every remaining 100-percent lane mandatory in docs, tooling, and gates.

**Architecture:** Keep release truth in canonical docs, enforce it with a small CI script, and route all approved offline tool lanes through the shared job runner. Runtime/editor code continues to consume exported artifacts only.

**Tech Stack:** PowerShell governance scripts, Python offline tooling, CMake/CTest Python tool registrations, Markdown release docs.

---

### Task 1: Mandatory Scope Governance

**Files:**
- Create: `tools/ci/check_mandatory_completion_scope.ps1`
- Modify: `tools/ci/run_local_gates.ps1`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/release/100_PERCENT_COMPLETION_INVENTORY.md`
- Modify: `docs/APP_RELEASE_READINESS_MATRIX.md`
- Modify: `docs/release/RELEASE_PACKAGING.md`

- [ ] Add a gate that rejects optional/future deferral phrasing in canonical 100-percent docs.
- [ ] Add mandatory open rows for WYSIWYG roadmap completion, offline tooling adoption, asset/content curation, and final release tagging.
- [ ] Wire the gate into `run_local_gates.ps1`.
- [ ] Run `powershell -ExecutionPolicy Bypass -File .\tools\ci\check_mandatory_completion_scope.ps1`.

### Task 2: Mandatory Offline Job Runner Coverage

**Files:**
- Modify: `tools/shared/job_runner/run_offline_job.py`
- Create: `tools/shared/job_runner/example_vision_segmentation_job.json`
- Create: `tools/shared/job_runner/example_audio_processing_job.json`
- Create: `tools/shared/job_runner/tests/test_offline_job_runner.py`
- Modify: `tools/shared/job_runner/README.md`
- Modify: `tools/retrieval/README.md`
- Modify: `tools/vision/README.md`
- Modify: `tools/audio/README.md`
- Modify: `tools/retrieval/requirements.txt`
- Modify: `tools/vision/requirements.txt`
- Modify: `CMakeLists.txt`

- [ ] Add `vision_segmentation` and `audio_processing` job types.
- [ ] Add focused tests proving retrieval, vision, and audio jobs all execute through the shared runner.
- [ ] Register the job-runner test in the Python `pr;tools;python` CTest lane.
- [ ] Run the Python tool tests.

### Task 3: Verification And Commit

**Files:**
- All files above.

- [ ] Run `python -m py_compile` for changed Python files.
- [ ] Run `ctest --preset dev-all -R "offline|retrieval|vision|audio|job_runner|tools" --output-on-failure`.
- [ ] Run `powershell -ExecutionPolicy Bypass -File .\tools\ci\check_mandatory_completion_scope.ps1`.
- [ ] Run `git diff --check`.
- [ ] Commit and push when checks pass.
