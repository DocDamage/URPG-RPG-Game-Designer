# ARDy Use-Case Admission and Deferral

Date: 2026-07-16
Decision owner required: product leadership
Current decision: **defer; no ARDy implementation or product dependency**

## Candidate use case

The only plausible bounded use case is generating humanoid waypoint locomotion,
full-body transitions, or sparse hand/foot-target motion as an offline reference for
a future skeletal 3D character. No shipping URPG character, rig, mesh/skin importer,
skeletal clip, retargeter, previewer, or baked skeletal runtime path currently owns
that output.

## Admission fields

| Field | Current evidence |
| --- | --- |
| Target character | None approved. The current product is 2D/2.5D and faux-3D. |
| Target rig | None approved. ARDy's published Core and G1 skeletons are not URPG runtime profiles. |
| Required motions | Candidate only: waypoint locomotion, full-body transition, sparse hand/foot target. |
| Target output | Would need to be a reviewed, baked neutral skeletal clip or rendered 2D reference; neither lane is approved. |
| Current manual time | Not measured because there is no admitted skeletal authoring task or representative rig. |
| Expected value | Unproven. The plan requires at least 30% median creator-time reduction after bounded cleanup. |
| Why existing tools are insufficient | Not established for a shipping use case; existing sprite/timeline tools match the current product lane. |

## Gate decision

The required non-ARDy skeletal workflow, mesh/skin/skeleton pipeline, Linux/NVIDIA
workstation owner, signed legal approval, and measured baseline do not exist. ARDy
therefore remains optional and deferred. ARDY-003 through ARDY-009 must not begin
unless product leadership changes this decision with evidence for every admission
gate. Release work should stay focused on the native sprite, timeline, event, menu,
accessibility, and presentation lanes.
