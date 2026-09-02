# MIDI Algorithm Custom Presets

> Archived on 2026-09-02. This implementation record is retained for historical
> context. Current package and sharing boundaries are documented in
> [../SHARING_ARCHITECTURE.md](../SHARING_ARCHITECTURE.md).

## Overview

This feature adds user-authored MIDI/generative algorithm presets to cendance. Users can create, edit, save, share, and use custom generative algorithms alongside the 20 built-in algorithms per track.

## Architecture

### Custom Algorithm ID Scheme

- **Built-in algorithms**: IDs `0..19` (contiguous, per track)
- **Custom algorithms**: IDs `2048..` (sparse, per track), defined by `AlgorithmPresetRegistry::kCustomAlgorithmIdBase = 2048`
- The sparse ID scheme prevents collisions if built-in algorithm count grows beyond 20
- `uint16_t` is used throughout for algorithm IDs (widened from `uint8_t`)

### Data Flow

```
TUI Editor / MCP Tools / P2P Import
        │
        ▼
CustomAlgorithmPreset (data model)
        │
        ▼
AlgorithmPresetRegistry (persistence + runtime ID assignment)
        │
        ▼
AudioEngine::rebuildCustomAlgorithmInstances()
        │
        ▼
CustomAlgorithmInstance (extends GenerativeAlgorithm)
        │
        ▼
Audio callback: processMidi() → MIDI output
```

## Files Created

| File | Purpose |
|------|---------|
| `Source/App/CustomAlgorithmPreset.h/.cpp` | Data model: JSON serialization, validation, constants |
| `Source/App/AlgorithmPresetRegistry.h/.cpp` | Runtime registry: file persistence, ID allocation, thread-safe |
| `Source/Audio/Generators/CustomAlgorithmInstance.h/.cpp` | Audio: pattern-based MIDI generation |
| `Source/App/GenreCatalog.cpp` | Runtime-aware genre helpers |

## Files Modified

| File | Changes |
|------|---------|
| `Source/Audio/AudioEngine.h/.cpp` | uint16_t algorithmId, dual storage, registry pointer, rebuild method |
| `Source/App/AppState.h` | TrackState::algorithmId widened to uint16_t |
| `Source/App/MeterQueue.h` | activeAlgorithm widened to uint16_t[4] |
| `Source/App/ProjectIO.h/.cpp` | TrackSnapshot algorithmId widened + customAlgorithmRef field |
| `Source/App/AgentCommand.cpp` | Catalog JSON includes custom algorithms |
| `Source/UI/TuiApp.h` | Algorithm editor modal state variables |
| `Source/UI/TuiAppDrumCommands.cpp` | Algorithm editor implementation + save logic |
| `Source/UI/TuiAppRender.cpp` | Algorithm editor modal rendering |
| `Source/UI/TuiAppInput.cpp` | Algorithm editor input handling |
| `Source/UI/TuiAppNumberSelection.cpp` | Custom algorithms shown after built-in 20 |
| `Source/Mcp/McpTools.cpp` | 6 new MCP tool schemas + handlers |
| `Source/Mcp/P2PToolHandler.h/.cpp` | Algorithm P2P tool dispatch (extracted from Main.cpp) |
| `Source/Mcp/McpMode.h/.cpp` | MCP mode entry point (extracted from Main.cpp) |
| `Source/Main.cpp` | Thin orchestrator — delegates to CliOptions, StartupRuntime, JuceRuntime, McpMode |
| `Source/App/CommandQueue.h` | RebuildCustomAlgorithms command type |
| `Source/Security/ContentHeader.h` | ContentType::Algorithm = 2 |
| `Source/Security/PresetSerializer.h/.cpp` | createAlgorithmEnvelope + verifyAndLoadAlgorithm |
| `Source/Network/P2PClient.h/.cpp` | publishAlgorithm + requestAlgorithm + searchAlgorithms |
| `Source/Network/P2PDownloadRegistry.h/.cpp` | Algorithm metadata fields (track_index, genre_tags, preset_ref, version) |
| `CMakeLists.txt` | 4 new .cpp files added |

## CustomAlgorithmPreset Data Model

```cpp
struct CustomAlgorithmPreset {
    std::string id, name, description, author, version, createdAt;
    std::vector<std::string> tags;
    uint8_t trackIndex = 0;
    float noteLength = 0.75f;
    uint8_t stepCount = 16;          // 1..64
    float swing = 0.0f;              // 0.0..0.5
    std::pair<uint8_t, uint8_t> velocityRange{64, 110};
    std::pair<int8_t, int8_t> octaveRange{0, 1};
    uint16_t scaleMask = 0x7f;
    std::vector<uint8_t> rhythmicPattern;   // 0=no note, >0=gate
    std::vector<int8_t> melodicPattern;     // semitone intervals
    std::vector<float> densityCurve;         // per-step density multiplier
    std::vector<float> complexityCurve;      // per-step complexity multiplier
    uint32_t genreTags = 0;                  // bitmask matching GenreCatalog
    std::optional<uint16_t> compatibleBuiltinAlgorithmId;
};
```

## AlgorithmPresetRegistry

- Stores presets in `~/Library/Application Support/cendance/algorithms/`
- One JSON file per preset + `index.json` for stable ordering
- Runtime IDs assigned sequentially from `kCustomAlgorithmIdBase` (2048)
- Thread-safe with `std::mutex`
- `savePreset()` validates before writing
- `rebuildIndex()` reassigns runtime IDs after mutations

## AudioEngine Integration

- `builtinTrackAlgorithms`: fixed 4×20 array for built-in algorithms (IDs 0..19)
- `customTrackAlgorithms`: `std::array<std::vector<std::unique_ptr<CustomAlgorithmInstance>>, 4>` for custom instances
- `customAlgorithmCounts`: cached counts to avoid locking registry mutex on audio path
- `getTrackAlgorithm()`: returns built-in pointer for IDs < 20, custom instance for IDs ≥ 2048, fallback to built-in 0
- `rebuildCustomAlgorithmInstances()`: called on construction and via `RebuildCustomAlgorithms` command
- `getMaxAlgorithmIdForTrack()`: returns max of (19, 2048 + customCount - 1)

## CustomAlgorithmInstance

Extends `GenerativeAlgorithm` with pattern-based MIDI generation:

1. Quantizes by `stepCount` over a 4/4 bar
2. `rhythmicPattern[step] == 0` means no primary note
3. `density * densityCurve[step]` = note probability
4. `complexity * complexityCurve[step]` = extra notes / grace notes
5. `melodicPattern` intervals transpose from rootNote, clamped to -24..24, filtered to scaleMask
6. Drums map steps to track-appropriate drum notes
7. `noteLength` = fraction of step duration
8. `swing` delays odd-numbered steps by up to half a step
9. `velocityRange` maps density/complexity/random variation into MIDI velocity
10. No file I/O, registry locks, heap allocation, or JSON parsing in `processMidi()`

## ProjectIO Integration

- `TrackSnapshot::algorithmId` widened to `uint16_t`
- `TrackSnapshot::customAlgorithmRef` stores `local.algorithms/<preset_id>/<version>` for IDs ≥ 2048
- `snapshotFromState()` resolves customAlgorithmRef for custom IDs
- `applySnapshotToCommandQueue()` resolves custom refs back to runtime IDs (graceful fallback to 0)
- `validateSnapshot()` accepts both built-in and custom algorithm IDs

## TUI Integration

- Algorithm editor modal opened from track algorithm selection
- 16-step default grid (adjustable 1..64)
- Drum grid edits on/off gates; melodic tracks edit semitone intervals
- Sliders/steppers for noteLength, swing, velocityRange, octaveRange
- Genre tag selector using GenreCatalog names
- Name, description, author, tags text fields
- On save: validate → write to registry → dispatch RebuildCustomAlgorithms command
- Custom algorithms shown after built-in 20 with `*` prefix

## MCP Tools

| Tool | Description | ToS Required |
|------|-------------|--------------|
| `create_custom_algorithm` | Create preset from pattern JSON | No |
| `list_custom_algorithms` | List presets (filter by track/genre) | No |
| `get_algorithm_pattern` | Get full pattern data | No |
| `update_custom_algorithm` | Update existing preset | No |
| `delete_custom_algorithm` | Delete preset | No |
| `share_algorithm_on_network` | Publish to P2P network | Yes |

## P2P Sharing

- `ContentType::Algorithm = 2`
- `PresetSerializer::createAlgorithmEnvelope()` signs preset JSON with Ed25519
- `PresetSerializer::verifyAndLoadAlgorithm()` verifies signature, validates, installs to registry
- `P2PClient::publishAlgorithm()` publishes signed envelope to file store
- `P2PClient::requestAlgorithm()` retrieves by ID
- `P2PClient::searchAlgorithms()` lists available algorithms
- File store uses `.algorithm` extension
- All P2P operations use `std::async` (off audio thread)
- ToS-gated: returns error code 4001 if ToS not accepted

## Genre Catalog Integration

- `GenreCatalog::algorithmHasGenreRuntime()` resolves custom algorithm genre tags through registry
- `GenreCatalog::getAlgorithmGenreMaskRuntime()` returns genre bitmask for any algorithm ID
- `AudioEngine::RandomizeForGenre` iterates built-ins (0..19) and custom algorithms separately
- Custom algorithms participate in genre-based randomization

## Command Queue

- `Command::Type::RebuildCustomAlgorithms` notifies audio engine to rebuild custom instances
- Dispatched by TUI after registry mutations (save, import, delete)

## Build Verification

```bash
cmake -S . -B build -DSODIUM_INCLUDE_DIR=/opt/homebrew/include -DSODIUM_LIBRARY=/opt/homebrew/lib/libsodium.dylib
cmake --build build --target cendance
```

Expected: clean build, valid arm64 Mach-O binary.

## Known Limitations

- `SynthCatalog::getDefaultPresetForAlgorithm` still takes `uint8_t` — custom algorithms > 19 fall back to built-in defaults
- `searchAlgorithms()` accepts query/trackIndex/genreId filters but ignores all (file-store v1 limitation)
- Registry lookup methods return raw pointers — concurrent save/delete can invalidate them (documented risk)
