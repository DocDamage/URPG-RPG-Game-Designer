#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path

REQUIRED_LANES = [f"FRL-{index:02d}" for index in range(1, 11)]
ALLOWED_STATUSES = {"not_started", "in_progress", "ready", "blocked", "deferred"}
REQUIRED_DOC_PHRASES = {
    "FRL-01": "broader battle feedback fixture coverage remains",
    "FRL-02": "broader fixture import coverage remains",
    "FRL-03": "richer visual controls remain",
    "FRL-04": "full task-graph runtime sequencing and arbitrary scripting remain",
    "FRL-05": "broader curated payload promotion and richer generated preview assets remain",
    "FRL-06": "richer painted diff rendering remains",
    "FRL-07": "broader automatic filesystem walking remains",
    "FRL-08": "broader subsystem validator invocation remains",
    "FRL-09": "true socket-level live chunk delivery remains",
    "FRL-10": "full native signing/notarization and launched multi-platform smoke remain",
}


def fail(message: str) -> None:
    print(f"feature robustness lane check failed: {message}", file=sys.stderr)
    raise SystemExit(1)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def require_non_empty_list(lane: dict, key: str) -> None:
    value = lane.get(key)
    if not isinstance(value, list) or not value:
        fail(f"{lane.get('id', '<unknown>')} must contain non-empty list '{key}'")
    for item in value:
        if not isinstance(item, str) or not item.strip():
            fail(f"{lane.get('id', '<unknown>')} has empty item in '{key}'")


def main() -> int:
    root = repo_root()
    tracker_path = root / "content" / "readiness" / "feature_robustness_lanes.json"
    doc_path = root / "docs" / "features" / "FEATURE_ROBUSTNESS_PLAN.md"

    if not tracker_path.is_file():
        fail(f"missing tracker: {tracker_path}")
    if not doc_path.is_file():
        fail(f"missing source doc: {doc_path}")

    tracker = json.loads(tracker_path.read_text(encoding="utf-8"))
    doc_text = doc_path.read_text(encoding="utf-8").lower()

    if tracker.get("schemaVersion") != "1.0.0":
        fail("schemaVersion must be 1.0.0")
    if tracker.get("sourceDocument") != "docs/features/FEATURE_ROBUSTNESS_PLAN.md":
        fail("sourceDocument must point to docs/features/FEATURE_ROBUSTNESS_PLAN.md")
    if "not bounded v0.1.0 release blockers" not in tracker.get("releaseBoundary", ""):
        fail("releaseBoundary must preserve bounded-release distinction")

    lanes = tracker.get("lanes")
    if not isinstance(lanes, list):
        fail("lanes must be a list")
    ids = [lane.get("id") for lane in lanes if isinstance(lane, dict)]
    if ids != REQUIRED_LANES:
        fail(f"lanes must be ordered exactly as {REQUIRED_LANES}; got {ids}")

    for expected_priority, lane in enumerate(lanes, start=1):
        lane_id = lane["id"]
        if lane.get("priority") != expected_priority:
            fail(f"{lane_id} priority must be {expected_priority}")
        if lane.get("status") not in ALLOWED_STATUSES:
            fail(f"{lane_id} status must be one of {sorted(ALLOWED_STATUSES)}")
        for key in ["title", "system", "remainingDepth"]:
            if not isinstance(lane.get(key), str) or not lane[key].strip():
                fail(f"{lane_id} must contain non-empty string '{key}'")
        for key in [
            "currentEvidence",
            "nextImplementation",
            "evidenceTargets",
            "verificationCommands",
        ]:
            require_non_empty_list(lane, key)
        if len(lane["nextImplementation"]) < 3:
            fail(f"{lane_id} must have at least three nextImplementation items")
        phrase = REQUIRED_DOC_PHRASES[lane_id]
        if phrase not in doc_text:
            fail(
                f"source doc no longer contains required phrase for {lane_id}: {phrase}"
            )
        if not all(
            target.endswith((".cpp", ".h", ".py", ".ps1"))
            for target in lane["evidenceTargets"]
        ):
            fail(
                f"{lane_id} evidenceTargets must name concrete implementation/test files"
            )

    print(f"feature robustness lane check passed: {len(lanes)} lanes governed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
