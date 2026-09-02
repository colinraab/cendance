# cendance Agent Contribution Network Plan

> Archived on 2026-09-02. This original design assumed a future Pilot bridge and
> predates the implemented sharing backends. See
> [../SHARING_ARCHITECTURE.md](../SHARING_ARCHITECTURE.md) for current behavior.

## Goal

Make cendance extensible by agents without requiring remote agents to modify or
execute native app code. Agents should be able to discover, exchange, preview,
install, and apply creative contributions through the existing MCP control
surface and a Pilot-facing bridge.

The first version should prove that agents can safely share useful musical
building blocks. Later versions can expand from data-only packages into sound
file transfer, contributed MIDI generation algorithms, contributed DSP, and
other creative extension types.

## Architecture

Use three layers:

1. **cendance app**
   - Owns validation, local installation, runtime catalog loading, and audio
     safety.
   - Keeps all package parsing and catalog mutation off the real-time audio
     thread.
   - Continues to apply state changes through existing command queues and app
     state boundaries.

2. **cendance MCP server**
   - Continues to expose playable app controls.
   - Adds contribution package tools for discovery, preview, install, removal,
     export, and catalog reads.
   - Remains the stable local automation API for agents.

3. **Pilot bridge agent**
   - Wraps the MCP server for network discovery and peer-to-peer exchange.
   - Advertises cendance contribution capabilities to other agents.
   - Transfers manifests/packages between trusted peers.
   - Does not bypass cendance validation or install policy.

This keeps Pilot as the agent-network transport/discovery layer, while cendance
defines what a valid creative contribution means.

## V1 Package Framework

Create a data-only `cendancePackage.v1` format. Every package has a manifest and
zero or more payload files.

Required manifest fields:

- `schema`: fixed value `cendancePackage.v1`
- `id`: stable reverse-domain or DID-like package identifier
- `version`: semantic package version
- `kind`: package type
- `name`
- `description`
- `authorAgent`
- `createdAt`
- `license`
- `compatibility`: minimum and maximum supported cendance package schema/app
  versions
- `contentHash`: hash of the canonical package contents
- `signature`: optional in local development, required for network exchange
- `dependencies`: package IDs, built-in catalog references, or required sample
  references
- `tags`: searchable terms such as genre, mood, instrument, tempo range, or
  energy

Supported V1 package kinds:

- `effectPresetPack`
  - Data-only insert/master FX presets that use existing effect types.
  - Can define effect chains, parameter values, names, descriptions, tags, and
    suggested track/master usage.

- `soundPresetPack`
  - Data-only sound presets that use existing synth/sample engines and existing
    FX.
  - Can define track sound settings, synth preset references, melodic sampler
    region references, drum kit references, recommended FX slots, and macro
    defaults such as density, complexity, tone, motion, and gain.
  - Must not introduce new DSP engines or executable code.
  - May reference built-in samples or already-installed sample IDs by stable
    package references.

- `drumKitPresetPack`
  - Drum kit slot mappings and per-slot parameters using existing sample slots.
  - In V1, should prefer built-in samples or already-installed samples.
  - External sound file payloads are reserved for a later package capability.

- `scenePresetPack`
  - Multi-track snapshots: algorithms, sound presets, FX, key, progression,
    tempo, arrangement section defaults, and mix settings.
  - Must be validated like project snapshots, with strict compatibility checks.

- `arrangementPresetPack`
  - Arrangement section layouts, section lengths, progression overrides, track
    masks, chain settings, and optional per-section parameter automation.

V1 package kinds are data-only. Any package that tries to include native code,
scripts, executable hooks, unrecognized payload types, or unsupported schema
fields must be rejected unless explicitly allowed by a later schema version.

## Local Storage And Catalog Rules

Store user-installed contributions outside the compiled built-in catalogs.
Recommended locations:

- Package staging: user data directory under `Contributions/Staging`
- Installed manifests: user data directory under `Contributions/Installed`
- Payload cache: user data directory under `Contributions/Payloads`

Catalog rules:

- Built-in IDs remain stable and keep their current meaning.
- Installed contribution IDs are namespaced and never inserted into built-in
  numeric ranges.
- Runtime catalog views can merge built-ins and installed contributions for UI
  and MCP display.
- Project files should persist contribution references by stable package ID and
  item ID, not by transient merged display index.
- If a referenced contribution is missing, load the project with a clear warning
  and fall back to a safe built-in default.

## MCP Tool Additions

Add package-oriented tools to the MCP server after the local package format is
defined:

- `list_cendance_packages`
  - Lists installed and staged packages with kind, version, author, tags, and
    validation status.

- `preview_cendance_package`
  - Validates a package from disk, a staged transfer, or a received manifest.
  - Returns a non-mutating summary of added presets, dependencies, warnings,
    compatibility, license, and trust metadata.

- `install_cendance_package`
  - Installs a previously previewed package.
  - Requires explicit agent/user intent.
  - Returns installed item references and updated catalog summaries.

- `remove_cendance_package`
  - Disables or removes an installed contribution package.
  - Must report any projects or presets that still reference it when known.

- `export_cendance_package`
  - Exports selected local presets/settings into a package manifest and payload.

- `get_cendance_contribution_catalog`
  - Returns installed contribution catalog items, grouped by kind.

- `apply_cendance_contribution`
  - Applies a specific installed contribution item to a track, master slot, or
    project context using stable contribution references.

## Pilot Bridge Behavior

The Pilot bridge should expose the MCP package workflow to other agents without
giving peers direct write access to the app.

Default network flow:

1. Local bridge advertises cendance contribution capability.
2. Remote agent discovers the bridge and requests package metadata.
3. Local bridge returns manifests and trust metadata only.
4. Remote agent requests a selected package by ID and content hash.
5. Package is transferred into the receiver's staging area.
6. Receiver previews and validates the package locally.
7. Receiver explicitly installs selected packages.
8. Receiver applies installed contributions through normal MCP/app commands.

Default install policy is **review then install**. Auto-install is a future
trusted-peer policy, not V1 default behavior.

## Safety And Trust

V1 safety requirements:

- Reject unknown package kinds unless the app explicitly supports them.
- Reject unsupported schema versions.
- Reject path traversal and absolute payload paths.
- Enforce size limits for manifests and payload bundles.
- Validate all numeric ranges against existing app constraints.
- Verify package hashes before preview and before install.
- Require signatures for network-sourced packages once signing is implemented.
- Surface license and provenance before install.
- Do not load files, allocate package structures, or parse contribution data on
  the audio thread.

Trust model:

- Local packages can be unsigned during development.
- Network packages should include author identity, content hash, and signature.
- Pilot peer trust allows discovery and transfer, but cendance validation remains
  authoritative.
- Installed package state should record source peer, install time, package hash,
  and signature status.

## Future Extension Lanes

The package framework should be deliberately extensible. Future package kinds
should follow the same lifecycle: discover, transfer, stage, preview, validate,
install, catalog, apply, remove.

Planned future kinds:

- `samplePack`
  - Transfers audio files with manifests, hashes, licenses, duration limits,
    supported formats, peak/loudness metadata, and sample-role tags.
  - Can power shared drum kits, melodic sampler instruments, and one-shot
    libraries.

- `midiGeneratorPack`
  - Contributes new MIDI generation behavior.
  - Should not be native C++ in its first form.
  - Prefer a constrained pattern DSL, declarative sequencer graph, or WASM/Lua
    sandbox with strict CPU, memory, and output limits.

- `effectAlgorithmPack`
  - Contributes new DSP/effect behavior.
  - Requires a much stricter sandbox or reviewed native plugin boundary.
  - Should be treated as post-V1 because DSP runs near the real-time audio path.

- `instrumentEnginePack`
  - Adds new synth or sampler engines.
  - Requires the same safety model as contributed effects plus project
    compatibility and preset migration rules.

- `agentWorkflowPack`
  - Shares higher-level agent workflows such as "make this loop darker,"
    "generate four compatible scenes," or "remix installed contributions."
  - Should execute outside the audio engine through MCP commands.

## Implementation Order

1. Define `cendancePackage.v1` manifest schema and validation rules.
2. Add local package staging, preview, install, removal, and catalog loading.
3. Add V1 package kind support for `effectPresetPack` and `soundPresetPack`.
4. Add `drumKitPresetPack`, `arrangementPresetPack`, and `scenePresetPack`.
5. Extend project/reference persistence to use stable contribution references.
6. Add MCP tools for package lifecycle operations.
7. Add a Pilot bridge around the MCP package tools.
8. Add package signing/trust metadata.
9. Expand into `samplePack` once licensing, payload size, and provenance rules
   are solid.
10. Explore `midiGeneratorPack` with a constrained non-native algorithm format.

## Non-Goals For V1

- No remote native code execution.
- No arbitrary C++ plugin installation from other agents.
- No direct writes from Pilot peers into cendance's installed contribution store.
- No audio-thread package parsing or network calls.
- No silent installation of network-sourced packages.
