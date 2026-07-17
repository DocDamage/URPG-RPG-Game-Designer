#!/usr/bin/env python3
"""Materialize deterministic PCQ-700 project fixtures from the governed plan."""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import json
import os
import re
import shutil
from pathlib import Path
from typing import Any


PLAN_SCHEMA = "urpg.product_benchmark_plan.v1"
FIXTURE_SCALES = ("tiny", "medium", "large")
HARDWARE_CLASSES = ("minimum_desktop", "recommended_desktop", "high_end_desktop")
METRICS = (
    "startup",
    "project_open",
    "save",
    "search",
    "map_edit",
    "playtest_launch",
    "frame_pacing",
    "package",
    "memory",
)
SAFE_ID = re.compile(r"^[a-z0-9][a-z0-9_-]{0,63}$")


class PlanError(ValueError):
    pass


def _positive_int(value: Any, field: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
        raise PlanError(f"{field} must be a positive integer")
    return value


def load_plan(path: Path) -> dict[str, Any]:
    try:
        plan = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise PlanError(f"unable to read benchmark plan: {error}") from error
    if not isinstance(plan, dict) or plan.get("schema") != PLAN_SCHEMA:
        raise PlanError(f"benchmark plan schema must be {PLAN_SCHEMA}")
    version = plan.get("baseline_version")
    if not isinstance(version, str) or not re.fullmatch(r"pcq700\.v[1-9][0-9]*", version):
        raise PlanError("baseline_version must match pcq700.v<positive integer>")

    fixtures = plan.get("fixtures")
    if not isinstance(fixtures, list) or len(fixtures) != len(FIXTURE_SCALES):
        raise PlanError("fixtures must contain exactly tiny, medium, and large rows")
    seen_ids: set[str] = set()
    seen_scales: set[str] = set()
    for fixture in fixtures:
        if not isinstance(fixture, dict):
            raise PlanError("every fixture must be an object")
        fixture_id = fixture.get("id")
        scale = fixture.get("scale")
        if not isinstance(fixture_id, str) or not SAFE_ID.fullmatch(fixture_id) or fixture_id in seen_ids:
            raise PlanError(f"invalid or duplicate fixture id: {fixture_id}")
        if scale not in FIXTURE_SCALES or scale in seen_scales:
            raise PlanError(f"invalid or duplicate fixture scale: {scale}")
        seen_ids.add(fixture_id)
        seen_scales.add(scale)
        for field in ("maps", "events", "assets", "database_records", "content_bytes"):
            _positive_int(fixture.get(field), f"fixture {fixture_id}.{field}")
    if seen_scales != set(FIXTURE_SCALES):
        raise PlanError("fixtures must cover tiny, medium, and large scales")

    hardware = plan.get("hardware")
    if not isinstance(hardware, list) or len(hardware) != len(HARDWARE_CLASSES):
        raise PlanError("hardware must contain exactly three target classes")
    hardware_ids: set[str] = set()
    hardware_classes: set[str] = set()
    for profile in hardware:
        if not isinstance(profile, dict):
            raise PlanError("every hardware profile must be an object")
        profile_id = profile.get("id")
        profile_class = profile.get("class")
        if not isinstance(profile_id, str) or not SAFE_ID.fullmatch(profile_id) or profile_id in hardware_ids:
            raise PlanError(f"invalid or duplicate hardware id: {profile_id}")
        if profile_class not in HARDWARE_CLASSES or profile_class in hardware_classes:
            raise PlanError(f"invalid or duplicate hardware class: {profile_class}")
        hardware_ids.add(profile_id)
        hardware_classes.add(profile_class)
        _positive_int(profile.get("logical_cores"), f"hardware {profile_id}.logical_cores")
        _positive_int(profile.get("memory_bytes"), f"hardware {profile_id}.memory_bytes")
        if not isinstance(profile.get("graphics_class"), str) or not profile["graphics_class"]:
            raise PlanError(f"hardware {profile_id}.graphics_class must be non-empty")

    thresholds = plan.get("thresholds")
    if not isinstance(thresholds, dict) or set(thresholds) != hardware_ids:
        raise PlanError("thresholds must contain exactly one row for every hardware id")
    for hardware_id, metric_rows in thresholds.items():
        if not isinstance(metric_rows, dict) or set(metric_rows) != set(METRICS):
            raise PlanError(f"thresholds.{hardware_id} must cover all required metrics")
        for metric, scale_rows in metric_rows.items():
            if not isinstance(scale_rows, dict) or set(scale_rows) != set(FIXTURE_SCALES):
                raise PlanError(f"thresholds.{hardware_id}.{metric} must cover all fixture scales")
            for scale, threshold in scale_rows.items():
                _positive_int(threshold, f"thresholds.{hardware_id}.{metric}.{scale}")
    return plan


def _write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8", newline="\n")


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _write_sparse_payload(path: Path, logical_bytes: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as stream:
        if os.name == "nt":
            import msvcrt

            fsctl_set_sparse = 0x000900C4
            bytes_returned = ctypes.c_ulong()
            handle = msvcrt.get_osfhandle(stream.fileno())
            kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
            succeeded = kernel32.DeviceIoControl(
                ctypes.c_void_p(handle),
                fsctl_set_sparse,
                None,
                0,
                None,
                0,
                ctypes.byref(bytes_returned),
                None,
            )
            if not succeeded:
                raise PlanError(f"unable to mark benchmark payload sparse: Windows error {ctypes.get_last_error()}")
        stream.seek(logical_bytes - 1)
        stream.write(b"\0")


def materialize_fixture(fixture: dict[str, Any], output_root: Path, force: bool = False) -> Path:
    fixture_id = fixture["id"]
    target = output_root.resolve() / fixture_id
    if target.exists():
        if not force:
            raise PlanError(f"fixture already exists: {target}; pass --force to replace it")
        if target.parent != output_root.resolve():
            raise PlanError(f"refusing to replace fixture outside output root: {target}")
        shutil.rmtree(target)
    target.mkdir(parents=True)

    _write_json(
        target / "project.json",
        {
            "_engine_version_min": "0.1.0",
            "_urpg_format_version": "1.0",
            "benchmark": {"plan": PLAN_SCHEMA, "scale": fixture["scale"]},
            "project_id": fixture_id,
            "project_name": f"PCQ-700 {fixture['scale'].title()} Benchmark",
            "schema_version": "urpg.project.v1",
            "startup": {"headless_safe": True, "map": "map_0000"},
        },
    )

    event_count = fixture["events"]
    map_count = fixture["maps"]
    event_offset = 0
    map_paths: list[Path] = []
    for map_index in range(map_count):
        count = event_count // map_count + (1 if map_index < event_count % map_count else 0)
        events = [
            {
                "id": f"event_{event_offset + event_index:06d}",
                "name": f"Benchmark Event {event_offset + event_index:06d}",
                "trigger": "confirm_interact",
                "x": (event_offset + event_index) % 64,
                "y": ((event_offset + event_index) // 64) % 64,
            }
            for event_index in range(count)
        ]
        event_offset += count
        path = target / "content" / "maps" / f"map_{map_index:04d}.json"
        _write_json(
            path,
            {
                "events": events,
                "height": 64,
                "id": f"map_{map_index:04d}",
                "schema": "urpg.map.v1",
                "spawn": {"x": 1, "y": 1},
                "width": 64,
            },
        )
        map_paths.append(path)

    asset_path = target / "content" / "assets" / "benchmark_catalog.json"
    _write_json(
        asset_path,
        {
            "assets": [
                {
                    "id": f"benchmark_asset_{index:06d}",
                    "kind": ("image", "audio", "data")[index % 3],
                    "project_path": f"content/benchmark_payload/asset_{index:06d}.bin",
                }
                for index in range(fixture["assets"])
            ],
            "schema": "urpg.benchmark_asset_catalog.v1",
        },
    )
    database_path = target / "content" / "database.json"
    _write_json(
        database_path,
        {
            "items": [
                {"id": f"benchmark_record_{index:06d}", "name": f"Benchmark Record {index:06d}", "price": index}
                for index in range(fixture["database_records"])
            ],
            "schema": "urpg.database.v1",
        },
    )

    payload_path = target / "content" / "benchmark_payload.bin"
    _write_sparse_payload(payload_path, fixture["content_bytes"])

    inventory_path = target / "benchmark_inventory.json"
    _write_json(
        inventory_path,
        {
            "assets": fixture["assets"],
            "content_bytes": payload_path.stat().st_size,
            "database_records": fixture["database_records"],
            "events": event_offset,
            "files": {
                "asset_catalog_sha256": _sha256(asset_path),
                "database_sha256": _sha256(database_path),
                "map_manifest_sha256": hashlib.sha256(
                    "".join(_sha256(path) for path in map_paths).encode("ascii")
                ).hexdigest(),
            },
            "fixture_id": fixture_id,
            "maps": map_count,
            "scale": fixture["scale"],
            "schema": "urpg.product_benchmark_fixture_inventory.v1",
        },
    )
    return target


def generate(plan_path: Path, output_root: Path, force: bool = False) -> list[Path]:
    plan = load_plan(plan_path)
    output_root.mkdir(parents=True, exist_ok=True)
    return [materialize_fixture(fixture, output_root, force=force) for fixture in plan["fixtures"]]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--plan",
        type=Path,
        default=Path("content/benchmarks/product_benchmark_plan_v1.json"),
        help="Governed PCQ-700 plan JSON.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("build/benchmarks/pcq700"),
        help="Generated fixture root.",
    )
    parser.add_argument("--force", action="store_true", help="Replace existing generated fixture directories.")
    arguments = parser.parse_args()
    try:
        generated = generate(arguments.plan, arguments.output, force=arguments.force)
    except PlanError as error:
        parser.error(str(error))
    print(json.dumps({"generated": [path.as_posix() for path in generated], "plan": str(arguments.plan)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
