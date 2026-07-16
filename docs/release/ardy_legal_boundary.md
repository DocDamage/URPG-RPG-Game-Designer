# ARDy Preliminary Legal and Redistribution Boundary

Date reviewed: 2026-07-16
Status: **preliminary engineering inventory; unsigned legal approval is still required**

## Distinct materials

| Material | Observed terms | Allowed URPG custody before legal approval |
| --- | --- | --- |
| `nv-tlabs/ardy` repository code | Apache-2.0; repository license also carries an LLM2Vec MIT notice. | Link-only research reference. Do not vendor or redistribute. |
| ARDY model checkpoints | NVIDIA Open Model Agreement on the published model page; separate from repository code. | Do not download, commit, bundle, or redistribute. |
| Llama text encoder | Gated Meta Llama access and a Hugging Face token are required by the ARDy setup. | No token or model custody in URPG; separate approval required. |
| PyTorch, CUDA, TensorRT, demo and transitive dependencies | Separate upstream terms and platform requirements. | External research environment only; never a game/editor/runtime dependency. |
| Bones motion data and other datasets | Separate dataset terms. | Do not download or use until dataset/output-use review is signed. |
| Generated output | NVIDIA's current open-model terms say NVIDIA does not claim output ownership, but users remain responsible for outputs and their uses. Dataset, prompt/source, performer, trademark, publicity, and downstream asset rights still require review. | Promote only a reviewed baked artifact with complete provenance and an explicit redistribution decision. |

## Required notices and provenance if a future pilot is approved

- Preserve Apache-2.0 copyright, license, modification, and applicable NOTICE terms
  for any redistributed repository code.
- Preserve every separate-component notice and the model agreement/attribution
  required by the exact checkpoint terms in force at approval time.
- Record repository commit, checkpoint hash, dependency lock, dataset/source rights,
  prompt policy or hash, seed, converter and retarget versions, reviewer, output hash,
  and the legal decision governing the promoted artifact.
- Re-review terms before distributing tooling, a checkpoint, derivative model, or
  generated content under materially changed terms.

## Distribution boundary

URPG games, editor packages, source archives, caches, support bundles, and normal
project files must not contain Python, CUDA, PyTorch, TensorRT, Llama, Hugging Face
tokens, ARDy checkpoints, research servers, vendor SDKs, or workstation paths. The
runtime may consume only a normal reviewed baked artifact after a future approved
pilot. This document is not legal advice and does not satisfy ARDY-001's required
signed legal review.

Official references:

- https://github.com/nv-tlabs/ardy
- https://huggingface.co/nvidia/ARDY-Core-RP-20FPS-Horizon40
- https://www.nvidia.com/en-us/agreements/enterprise-software/nvidia-open-model-license/
