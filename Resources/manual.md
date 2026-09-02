# cendance MCP Manual

cendance is a generative music application with a terminal UI and an embedded MCP server. The MCP server is a JSON-RPC 2.0 stdio interface that lets any MCP-compatible agent read state, control transport, configure tracks, author preset packages, and apply them — all without source-code access.

---

## Quick Start

Connect your MCP client to the cendance binary using its stdio mode. Example client config:

```json
{
  "mcpServers": {
    "cendance": {
      "command": "/path/to/cendance_artefacts/cendance",
      "args": ["--mcp"]
    }
  }
}
```

When cendance launches in MCP mode it:
1. Redirects all startup logs to stderr (stdout is reserved for JSON-RPC)
2. Waits for `initialize` requests on stdin
3. Serves MCP tools, resources, prompts, and an agent authoring guide

After your client sends `initialize` and `initialize/notifications`, the server is ready.

---

## Transport Controls

| Tool | Description |
|------|-------------|
| `play_cendance` | Start playback |
| `pause_cendance` | Pause playback |
| `stop_playback` | Stop and reset arrangement |
| `set_tempo` | Set tempo (BPM) or adjust with delta mode |
| `set_key` | Set project key, e.g. "A minor", "Db major" |
| `set_progression` | Set chord progression by one-based catalog id |
| `set_genre` | Set project genre by one-based id or name |
| `randomize_for_genre` | Randomize tempo, key, progression, algorithms, and sounds for a genre |
| `set_arrangement_section` | Set current arrangement section by id |

Example:
```json
{"method":"tools/call","params":{"name":"set_tempo","arguments":{"value":125}},"id":1}
```

---

## Track Controls

cendance has 4 tracks: **1** = Drums, **2** = Bass, **3** = Chords, **4** = Lead.

| Tool | Description |
|------|-------------|
| `set_track_parameter` | Set density, complexity, tone, motion, or gain (0–2) |
| `set_track_algorithm` | Set track algorithm by one-based catalog id |
| `set_track_sound` | Set track sound by one-based catalog id |
| `set_track_mute` | Mute/unmute a track |
| `set_track_fx` | Set track FX slot (1–3) by effect catalog id |
| `set_master_fx` | Set master FX slot (1–3) by effect catalog id |

Example — set track 2 gain to 1.5:
```json
{"method":"tools/call","params":{"name":"set_track_parameter","arguments":{"track":2,"parameter":"gain","value":1.5}},"id":2}
```

---

## Discovery and Introspection

Before changing anything, discover available content:

| Tool | Description |
|------|-------------|
| `get_cendance_state` | Get current state snapshot. `full=true` for full dump |
| `get_cendance_catalog` | Discover algorithms, sounds, effects, progressions, or presets. Requires `kind` param |
| `get_cendance_preset_catalog` | Get merged preset catalog (built-in + installed packages) |
| `get_cendance_meters` | Get current meter and spectrum data |
| `listen_to_cendance` | Run listening heuristic over recent meter history (1–60s) |
| `get_cendance_agent_authoring_guide` | Get end-to-end workflow, ref format, rules |
| `get_cendance_package_schema` | Get package schema and examples. Optional `kind` filter |

**Recommended discovery flow:**
1. Call `get_cendance_state` to understand the current project
2. Call `get_cendance_preset_catalog` to discover refs for sounds and effects
3. Call `get_cendance_package_schema` to learn how to author packages
4. Call `get_cendance_agent_authoring_guide` for the full workflow

---

## Preset Refs

Refs are stable identifiers for presets:

```
<domain>:<source>:<id>
```

| Field | Values |
|-------|--------|
| **domain** | `effect`, `sound`, `drumKit`, `arrangement`, `scene`, `sample`, `midiGenerator`, `instrumentEngine` |
| **source** | `builtin` or `package` |
| **id** | `builtin-slug` or `packageId:itemId[@version]` |

**Examples:**
- Built-in sound: `sound:builtin:bright-gloss`
- Package effect: `effect:package:agent.demo.fx:soft-wide-delay`
- Package sound with version: `sound:package:agent.gloss.bass:gloss-bass@0.1.0`

**Ref tools:**
| Tool | Description |
|------|-------------|
| `apply_cendance_preset_ref` | Apply any PresetRef. Sound refs need only `ref`; effect refs need `track` and `slot` |
| `set_track_sound_ref` | Set track sound by durable PresetRef |
| `set_track_fx_ref` | Set track FX by PresetRef with track+slot |
| `set_master_fx_ref` | Set master FX by PresetRef with slot (track=5) |

**⚠️ Never invent refs.** Copy them from `get_cendance_preset_catalog`.

---

## Contribution Packages

cendance packages are data-only JSON files (`.cendance-package.json`) containing
presets, sound definitions, arrangements, scenes, drum kits, or sample assets.

### Package Kinds

| Kind | Description |
|------|-------------|
| `effectPresetPack` | Custom effect presets with paramA/B/C |
| `soundPresetPack` | Sound presets referencing existing engines |
| `drumKitPresetPack` | Drum kit slot configurations |
| `scenePresetPack` | Full scene snapshots (BPM, key, tracks, FX) |
| `arrangementPresetPack` | Arrangement section definitions |
| `samplePack` | Sample assets and metadata for validated local installation |

### Authoring Workflow

1. **Discover refs**: Call `get_cendance_preset_catalog` and copy refs you want to reuse
2. **Get schema**: Call `get_cendance_package_schema` for your target kind
3. **Write JSON**: Create a `.cendance-package.json` using the schema + copied refs
4. **Preview**: Call `preview_cendance_package` — fix any validation errors
5. **Install**: Call `install_cendance_package` after successful preview
6. **Find new refs**: Call `get_cendance_preset_catalog` again to see your installed package refs
7. **Apply**: Use `apply_cendance_preset_ref` or the specialized ref tools
8. **Evaluate**: Call `listen_to_cendance` and `get_cendance_meters` to check results

### Package Management Tools

| Tool | Description |
|------|-------------|
| `list_cendance_packages` | List installed packages and their items |
| `get_cendance_contribution_catalog` | Get installed contribution catalog grouped by package kind |
| `preview_cendance_package` | Validate a package at local path (required: `path`) |
| `install_cendance_package` | Install a reviewed package (required: `path`) |
| `remove_cendance_package` | Remove by stable package id (required: `packageId`) |
| `export_cendance_package` | Export a package template (required: `path`, `kind`, `packageId`, `name`) |
| `apply_cendance_contribution` | Apply installed item by kind+packageId+itemId (optional: track, slot for effects) |

---

## Rules for Agent Authors

1. **Do not invent built-in refs** — discover them from `get_cendance_preset_catalog`
2. **Do not claim numeric preset ids** — package item ids are namespaced under `packageId`
3. **Always preview before install** — `preview_cendance_package` catches schema errors
4. **Data-only packages** — v1 packages contain no executable code
5. **No native code in packages** — custom MIDI patterns and samples are data;
   native synth, generator, and DSP code is not accepted
6. **Always evaluate** — use `listen_to_cendance` and `get_cendance_meters` after applying changes

---

## Raw Command Escape Hatch

`send_cendance_command` lets you send raw agent protocol commands. Use this for debugging or features not yet exposed as tools.

Genre-related raw commands are also supported:

```text
genre House
genre randomize "UK Garage"
genre randomize none
```

## Experimental Sharing

Sharing tools can sign and exchange presets, samples, custom sound presets,
custom algorithms, arrangements, and projects. The application does not display
or require a separate sharing or terms acknowledgement.

The default backend is a local file store. Setting `CENDANCE_P2P_ENDPOINT`
selects a user-operated HTTP backend; cendance does not provide a hosted public
endpoint. The localhost agent protocol and mDNS discovery are separate
experimental features, and cendance is not currently integrated with Pilot
Protocol. See `docs/SHARING_ARCHITECTURE.md` for the exact boundaries.

---

## Effect Types

Core/insert effect types:

`None`, `HighPassSweep`, `ReverbWash`, `ReduxCrush`, `DelayEcho`, `SaturationWaveshaper`, `SoftHardClip`, `Wavefolder`, `AsymShaper`, `CompressorGlue`, `PeakLimiter`, `TransientShaper`, `CombFilter`, `MultiModeEQ`, `FormantFilter`, `Autopan`, `RingModulator`, `Chorus`, `Phaser`, `Flanger`, `JitterDegrade`, `ErosionDegrade`, `TranceGate`, `SidechainDucker`, `BeatRepeatInsert`, `FrequencyShifter`, `PitchShifter`, `Harmonizer`, `TimeFreezer`, `GrainDelay`, `PhysicalModelingResonator`, `MultibandOtt`, `ConvolutionReverb`, `TapeDelay`, `PingPongDelay`, `CloudGenerator`, `SpectralBlur`, `SpectralDelay`

Trigger-only spot effect types:

`TapeStop`, `BeatRepeat`

---

## Genre System

cendance supports 8 genres that drive tempo, key/progression selection, algorithms, and sounds. Press `g` or `G` in the TUI to open the genre selector.

| Genre | BPM Range |
|-------|-----------|
| House | 118–130 |
| UK Garage | 130–140 |
| DnB | 160–175 |
| Trap | 130–150 |
| Hip-Hop | 80–100 |
| Techno | 120–140 |
| Trance | 130–150 |
| Synth Pop | 100–130 |

Selecting a genre:
1. Sets the active project genre
2. Randomizes tempo inside the genre BPM range
3. Randomizes project key
4. Picks a genre-tagged chord progression
5. Assigns genre-compatible algorithms and sounds across tracks

Use `set_genre` to set the active genre without changing the rest of the project:

```json
{"method":"tools/call","params":{"name":"set_genre","arguments":{"name":"Techno"}},"id":3}
```

Use `randomize_for_genre` for full genre-aware randomization:

```json
{"method":"tools/call","params":{"name":"randomize_for_genre","arguments":{"name":"UK Garage"}},"id":4}
```

Pass `id: 0` or `name: "none"` to `randomize_for_genre` for unfiltered randomization:

```json
{"method":"tools/call","params":{"name":"randomize_for_genre","arguments":{"id":0}},"id":5}
```

Genre ids are one-based:

| ID | Genre |
|----|-------|
| 1 | House |
| 2 | UK Garage |
| 3 | DnB |
| 4 | Trap |
| 5 | Hip-Hop |
| 6 | Techno |
| 7 | Trance |
| 8 | Synth Pop |

Algorithms, sounds, and chord progressions expose `genre_tags` bitmasks using the same genre layout. Call `get_cendance_catalog` with `kind` set to `algorithms`, `sounds`, or `progressions` to inspect those tags.

---

## Technical Notes

- **Transport**: MCP stdio with Content-Length framing (`Content-Type: application/json\r\nContent-Length: N\r\n\r\n<JSON>`)
- **JSON-RPC**: Strict 2.0 compliance — notifications (`id` absent) produce no response
- **Startup logs**: Redirected to stderr during JUCE initialization
- **Shutdown**: SIGINT/SIGTERM handled gracefully; stdin EOF exits loop
