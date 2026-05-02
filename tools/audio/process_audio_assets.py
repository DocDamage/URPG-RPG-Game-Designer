#!/usr/bin/env python3
"""Emit offline audio processing manifests for stems and compression trials."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


DEMUCS_STEMS = ("drums", "bass", "other", "vocals")


def build_outputs(
    source_audio: Path, output_root: Path, mode: str, reviewed: bool
) -> list[dict]:
    output_root.mkdir(parents=True, exist_ok=True)
    release_eligible = reviewed and mode == "compression"
    review_status = "reviewed" if reviewed else "pending_review"
    outputs = []

    if mode == "stems":
        for label in DEMUCS_STEMS:
            path = output_root / f"{source_audio.stem}_{label}.wav"
            path.write_text(
                f"placeholder {label} stem for {source_audio.as_posix()}\n",
                encoding="utf-8",
            )
            outputs.append(
                {
                    "output_id": f"{source_audio.stem}_{label}",
                    "kind": "demucs_stem",
                    "path": path.as_posix(),
                    "source_attribution": "creator_supplied",
                    "review_status": review_status,
                    "generated_prototype": True,
                    "release_eligible": False,
                }
            )
    else:
        path = output_root / f"{source_audio.stem}_encodec.ogg"
        path.write_text(
            f"placeholder compressed audio for {source_audio.as_posix()}\n",
            encoding="utf-8",
        )
        outputs.append(
            {
                "output_id": f"{source_audio.stem}_encodec",
                "kind": "encodec_compression_experiment",
                "path": path.as_posix(),
                "source_attribution": "creator_supplied",
                "review_status": review_status,
                "generated_prototype": not reviewed,
                "release_eligible": release_eligible,
            }
        )
    return outputs


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Create an offline audio tool manifest."
    )
    parser.add_argument("--source-audio", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--output-root", required=True)
    parser.add_argument("--mode", choices=["stems", "compression"], default="stems")
    parser.add_argument("--reviewed", action="store_true")
    args = parser.parse_args()

    source_audio = Path(args.source_audio)
    manifest = {
        "schema": "content/schemas/audio_tool_manifest.schema.json",
        "tool_id": "demucs_encodec_offline_processor",
        "source_audio": source_audio.as_posix(),
        "outputs": build_outputs(
            source_audio, Path(args.output_root), args.mode, args.reviewed
        ),
    }

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote audio tool manifest: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
