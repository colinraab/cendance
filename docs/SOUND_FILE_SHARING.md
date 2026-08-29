# Sound File Sharing — Implementation Documentation

**Ticket:** t_b2987186  
**Status:** ✅ Complete  
**Build:** Clean compilation, arm64 Mach-O binary verified

## Overview

Extends the P2P preset sharing system to support sound file and custom sound preset distribution. Adds sample pack contribution packages, WAV/FLAC sample serialization with base64 encoding, custom sound presets with embedded samples, 6 new MCP tools, a TUI sound file browser modal, and ToS compliance updates.

## What Was Implemented

### 1. Sample Pack Contribution Packages
- `ContributionPackage::Kind::SamplePack` — new package kind for sample packs
- `SamplePackItem` struct with: itemId, name, description, filePath, format, sampleRate, channels, duration, tags, sha256
- Full serialization/deserialization in `ContributionPackage::Library`
- Payload file copying into `Payloads/<packageId>/` on install
- `Library::findSamplePackItem()` lookup method
- Updated `kindToString()`, `kindFromString()`, `parsePackage()`, `appendPackageSummary()`, `packagesJson()`, `contributionCatalogJson()`, `previewJson()`, `exportPackageTemplate()`

### 2. Sound File Serialization for P2P
- `SampleEnvelopeMetadata` struct (name, description, format, tags)
- `VerifiedSampleResult` struct (ok, trustLevel, local_path, name, format, sha256, sender_id, error, sampleRate, channels, duration, timestamp)
- `PresetSerializer::createSampleEnvelope(audioFilePath, metadata, security, error)` — reads WAV/FLAC via JUCE AudioFormatManager, extracts metadata, base64-encodes audio, signs with libsodium
- `PresetSerializer::verifyAndLoadSample(envelope_json, security)` — verifies signature, decodes base64, writes to download sample directory
- `PresetSerializer::downloadSampleDirectory()` — separate directory for downloaded samples
- `PresetSerializer::maxSampleFileBytes()` — configurable max file size (default 50MB)
- Envelope format: `{content_type: 1, header: ContentHeader, payload: {name, format, sample_rate, channels, duration, data_base64, sha256, tags}}`

### 3. Custom Sound Preset Sharing
- `CustomSoundPresetOptions` struct (trackIndex, includeSamples)
- `PresetSerializer::createCustomSoundPresetEnvelope(appState, trackIndex, includeSamples, security, error)`
- Embedded samples: base64 audio data included in envelope alongside ProjectSnapshot
- Uses `ContentType::Preset` with `"preset_kind":"custom_sound"` subtype marker
- Validates track index against `AppState::kTrackCount`

### 4. P2P Network Extensions
- `P2PClient::publishSample(signed_envelope)` — publish sample envelope
- `P2PClient::requestSample(sample_id)` — request sample by ID
- `P2PClient::searchSamples()` — list available samples
- Extended `NetworkPresetEntry` with: fileSize, format, sampleRate, channels, contentType
- Extended `P2PDownloadEntry` with: content_type, display_name, format, sample_rate, channels, duration, sha256
- File-store helpers refactored for type-aware extensions (.preset / .sample)
- ID prefixes: `preset_...` and `sample_...`
- Backward-compatible JSON serialization for `p2p_downloads.json`

### 5. MCP Tools for Sound File Sharing
6 new tools added to `McpServer::toolSchemas()` and `handleToolsCall()`:
- `save_and_sign_sample` — `{path, name?, description?, tags?}` → `{envelope, sha256, format, sample_rate, channels, duration}`
- `share_sample_on_network` — `{sample_json}` → `{ok, sample_id, error?}`
- `search_samples` — `{query?, format?}` → sample list
- `download_sample` — `{sample_id}` → local path
- `list_downloaded_samples` — no args → registry entries
- `create_custom_sound_preset` — `{track, includeSamples=true}` → signed envelope

All tools gated by `ToSGuard::isAccepted()` (error code 4001).

### 6. TUI Integration
- Sound file browser modal in `TuiAppRender.cpp` (follows `drumSampleModalOpen` pattern)
- Columns: name, format, sample rate, channels, duration, sender, verified status
- Key bindings: Enter (import), 1-4 (select drum slot), R (refresh), Esc (close)
- Import flow: `DrumSampleLibrary::importSampleFromPath()` → `SetDrumSampleSlotSampleId` command
- Sample pack install UI: browse installed/previewed sample packs, install, import payloads
- Modal state: `soundFileBrowserOpen`, selected index, status, filter mode, cached entries
- `TuiApp` constructor extended with `P2PClient*` and `PresetSerializer*` dependencies

### 7. ToS Compliance
- Updated `ToSGuard::tosText()` with sample sharing terms:
  - P2P features include presets, samples, sample packs, and embedded custom sound presets
  - Copyright/license language for uploaded audio files, loops, one-shots, and sample packs
  - Sample files may be copied to peers' devices and retained in their local downloads
- All new MCP tools and P2P operations check `ToSGuard::isAccepted()`

## Files Modified (17)

| File | Changes |
|------|---------|
| `Source/App/ContributionPackage.h` | SamplePackItem struct, SamplePack Kind, findSamplePackItem() |
| `Source/App/ContributionPackage.cpp` | Full SamplePack parsing, serialization, install/remove |
| `Source/Config/ToSGuard.cpp` | Updated tosText() with sample sharing terms |
| `Source/Mcp/P2PToolHandler.h/.cpp` | Extended with 6 new sample/sound P2P tool handlers (extracted from Main.cpp) |
| `Source/Mcp/McpTools.cpp` | 6 new tool schemas + handlers |
| `Source/Network/P2PClient.h` | publishSample(), requestSample(), searchSamples(), extended NetworkPresetEntry |
| `Source/Network/P2PClient.cpp` | Implementation of sample P2P operations |
| `Source/Network/P2PDownloadRegistry.h` | Extended P2PDownloadEntry with sample metadata |
| `Source/Network/P2PDownloadRegistry.cpp` | Extended JSON serialization/deserialization |
| `Source/Security/PresetSerializer.h` | New methods: createSampleEnvelope, verifyAndLoadSample, createCustomSoundPresetEnvelope, downloadSampleDirectory, maxSampleFileBytes |
| `Source/Security/PresetSerializer.cpp` | Full implementation of sample + custom preset serialization |
| `Source/UI/TuiApp.h` | Sound file browser modal state variables |
| `Source/UI/TuiApp.cpp` | Constructor extension, import helper |
| `Source/UI/TuiAppDrumCommands.cpp` | Sound file browser open/close, import logic |
| `Source/UI/TuiAppInput.cpp` | Sound file browser input handling |
| `Source/UI/TuiAppRender.cpp` | Sound file browser modal rendering |

## Build Verification

```bash
cd build
cmake --build . --target cendance -j8
# Result: 13 compilation units, clean link, arm64 Mach-O binary
```

## Architecture Notes

- All crypto operations use `std::async` (off audio thread)
- JUCE `AudioFormatManager` for WAV/FLAC reading and metadata extraction
- JUCE `Base64` for audio data encoding/decoding
- File-store fallback for P2P (local directory with .sample/.preset extensions)
- HTTP endpoint architecture ready for future swap (set via `CENDANCE_P2P_ENDPOINT`)
- Trust levels: Verified (signature OK), Untrusted (no signature), Tampered (hash mismatch)
