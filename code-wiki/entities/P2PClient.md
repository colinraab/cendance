---
title: P2PClient
created: 2026-05-14
updated: 2026-05-14
type: entity
tags: [network, p2p, preset]
source_files:
  - Source/Network/P2PClient.h
  - Source/Network/P2PClient.cpp
  - Source/Network/P2PDownloadRegistry.h
  - Source/Network/P2PDownloadRegistry.cpp
---

# P2PClient

## Purpose
P2P preset/sample/algorithm sharing client. V1 uses a local file-backed store for testing. Accepts `CENDANCE_P2P_ENDPOINT` for future HTTP/IPFS/S3 backend. All operations are async (return `std::future`).

## Key Members
| Member | Type | Description |
|--------|------|-------------|
| `endpoint_` | `std::string` | HTTP endpoint URL |
| `registry_` | `P2PDownloadRegistry` | Download tracking registry |

## Relationships
- **UsedBy:** [[TuiApp]] (sound file browser, preset sharing)
- **UsedBy:** [[McpServer]] (P2P tools via p2pFn_)
- **Uses:** [[PresetSerializer]], [[SecurityManager]], [[ContentHeader]]
- **Contains:** [[P2PDownloadRegistry]]

## Behavior
- `setEndpoint(url)` / `endpoint()` — Configure/get the P2P endpoint.
- `publishPreset(envelope)` — Publish a signed preset envelope. Returns `future<PublishResult>`.
- `publishSample(envelope)` — Publish a signed sample envelope.
- `publishAlgorithm(envelope)` — Publish a signed algorithm envelope.
- `requestPreset(id)` / `requestSample(id)` / `requestAlgorithm(id)` — Download by ID. Returns `future<string>`.
- `searchPresets()` / `searchSamples()` / `searchAlgorithms()` — Search available content. Returns `future<vector<NetworkPresetEntry>>`.
- `isConfigured()` — True if endpoint is set.
- `registry()` — Access the download registry.

## Thread Safety
- All public methods return `std::future` — they're async by design.
- Internal file-store fallback is synchronous but only used when no endpoint configured.

## Gotchas
- V1 is file-backed only. Network endpoint is optional.
- Published content is signed via [[SecurityManager]] / [[PresetSerializer]].
- `PublishResult` contains `preset_id` / `sample_id` assigned by the network (or generated locally).
- Content types: `ContentType::Preset`, `ContentType::Sample`, `ContentType::Algorithm`.
