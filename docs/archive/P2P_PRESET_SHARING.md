# P2P Preset Sharing — Implementation Summary

> Archived on 2026-09-02. This milestone summary is retained for history and may
> describe behavior that has since changed. See
> [../SHARING_ARCHITECTURE.md](../SHARING_ARCHITECTURE.md).

**Ticket:** t_d17983e6
**Status:** Complete — all 5 phases implemented and building cleanly

## What Was Built

### Phase 1: Legal Onboarding (ToSGuard)
- **Files:** `Source/Config/ToSGuard.h`, `Source/Config/ToSGuard.cpp`
- Checks for `config.json` in `~/Library/Application Support/cendance/config.json`
- On first run (no config or `tos_accepted=false`), blocks with a full-screen FTXUI modal before the TUI renders
- Modal requires typing "I AGREE" explicitly; Escape exits the app
- Stores `tos_accepted=true` + ISO 8601 timestamp in config.json
- ToS text covers: warranty disclaimer (no hardware/hearing damage liability), copyright indemnification, DMCA policy link, P2P network use

### Phase 2: Identity & Crypto (SecurityManager + ContentHeader)
- **Files:** `Source/Security/ContentHeader.h/.cpp`, `Source/Security/SecurityManager.h/.cpp`
- Uses **libsodium** for Ed25519 digital signatures
- `ContentHeader`: sender_id, timestamp (unix millis), SHA-256 content hash, Ed25519 signature (64 bytes), content_type enum
- `SecurityManager::generateKeyPair()` — creates/persists Ed25519 keypair on first use (stored as hex in `~/Library/Application Support/cendance/identity.json`)
- `SecurityManager::sign()` — hashes data, signs with local keypair (async via `std::async`)
- `SecurityManager::verify()` — re-hashes, verifies against sender's public key (async)
- Secret key memory is securely wiped on destruction via `sodium_memzero`

### Phase 3: Preset Serialization (PresetSerializer)
- **Files:** `Source/Security/PresetSerializer.h/.cpp`
- Reuses `ProjectIO::ProjectSnapshot` (all 4-track DSP state: algorithm, synthPreset, macros, FX slots, drum samples, BPM, key, progression, arrangement sections, master FX)
- `createEnvelope()` — snapshots state, serializes via existing `ProjectIO::saveProjectFile()`, and wraps in transport envelope: `{content_type, header: ContentHeader, payload: snapshot JSON}`
- `verifyAndLoad()` — parses envelope, re-hashes payload, verifies signature → `{ok, trustLevel: Verified|Untrusted|Tampered, payload_json}`
- On verified success, payload can be fed into existing `ProjectIO::applySnapshotToCommandQueue()`
- Download folder: `~/Library/Application Support/cendance/downloads/`

### Phase 4: P2P Network Operations (P2PClient + P2PDownloadRegistry)
- **Files:** `Source/Network/P2PClient.h/.cpp`, `Source/Network/P2PDownloadRegistry.h/.cpp`
- Standalone P2P layer (Pilot Protocol SDK not available)
- v1 uses file-based local store at `~/Library/Application Support/cendance/downloads/store/`
- Reads `CENDANCE_P2P_ENDPOINT` when configured; otherwise uses the local file store
- Architecture supports swapping in HTTP endpoint, IPFS, or S3 later
- `publishPreset(signed_envelope)` → stores and returns unique preset_id
- `requestPreset(preset_id)` → retrieves signed envelope by ID
- `searchPresets()` → lists available presets with sender info
- `P2PDownloadRegistry` — thread-safe tracking of downloaded presets with JSON persistence

### Phase 5: MCP Tools (6 new tools)
- **Modified:** `Source/Mcp/McpTools.cpp`
- Added to `toolSchemas()` JSON array and `handleToolsCall()` dispatcher:
  - `get_tos_status` → `{accepted, timestamp}`
  - `save_and_sign_preset` → `{state_json}` → signed envelope
  - `share_on_network` → `{preset_json}` → publish result
  - `search_network` → available preset list
  - `verify_incoming_preset` → `{preset_json}` → verification result
  - `list_downloaded_presets` → registry contents
- P2P tools route through the injected MCP P2P handler, with a legacy execute-function fallback
- Error before ToS acceptance: code 4001

### TUI Integration
- **Modified:** `Source/UI/TuiApp.h`, `TuiApp.cpp`, `TuiAppRender.cpp`, `TuiAppInput.cpp`
- ToS modal state: `tosModalOpen`, `tosInput`, `tosStatus`, `tosAccepted`
- `TuiApp::run()` checks `ToSGuard::isAccepted()` and opens modal on first run
- Modal renders as full-screen FTXUI window with ToS text, input field, and status
- Input handling: type "I AGREE" + Enter to accept, Escape to decline and exit
- Modal blocks all other input while open

### Build System
- **Modified:** `CMakeLists.txt`
- Added 6 new `.cpp` files to `target_sources`
- Finds and links libsodium via CMake `find_path()` / `find_library()`
- **Build verified clean** — all compile steps and CTest suite pass

## File Inventory

### New Files (14)
```
Source/Config/ToSGuard.h
Source/Config/ToSGuard.cpp
Source/Security/ContentHeader.h
Source/Security/ContentHeader.cpp
Source/Security/SecurityManager.h
Source/Security/SecurityManager.cpp
Source/Security/PresetSerializer.h
Source/Security/PresetSerializer.cpp
Source/Network/P2PClient.h
Source/Network/P2PClient.cpp
Source/Network/P2PDownloadRegistry.h
Source/Network/P2PDownloadRegistry.cpp
```

### Modified Files (10)
```
CMakeLists.txt
Source/Main.cpp                  # Thin orchestrator — P2P setup delegated to P2PToolHandler
Source/Mcp/McpServer.h
Source/Mcp/McpServer.cpp
Source/Mcp/McpTools.cpp
Source/Mcp/P2PToolHandler.h/.cpp # P2P tool dispatch (extracted from Main.cpp lambda)
Source/UI/TuiApp.h
Source/UI/TuiApp.cpp
Source/UI/TuiAppInput.cpp
Source/UI/TuiAppRender.cpp
```

## Architecture Notes

- All crypto operations are off-audio-thread via `std::async`
- libsodium is required and can be installed with the platform package manager, e.g. `brew install libsodium`
- The P2P layer is designed to be swappable: file store → HTTP endpoint → IPFS/S3
- ToS acceptance gates MCP P2P tools that perform sharing/signing (error 4001) and the TUI modal blocks first-run app use
- The `sender_id` is the full Ed25519 public key hex so incoming signatures can be verified
- Transport envelope format: `{content_type: int, header: JSON string, payload: JSON string}`
