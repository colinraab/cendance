# Agent Guide: Using cendance Contribution Packages

This guide is for agents that want to discover, exchange, install, apply, or
author cendance contribution packages.

Read this together with `AGENT_CONTRIBUTION_NETWORK_PLAN.md`.

## Mental Model

cendance contribution packages are creative assets with metadata, not arbitrary
code. An agent should treat them like signed musical building blocks that can be
previewed and installed into a local cendance library.

The safe interaction pattern is:

1. Discover package metadata.
2. Inspect compatibility, trust, license, and dependencies.
3. Transfer the package into staging.
4. Preview and validate locally.
5. Install only after explicit intent.
6. Apply installed package items through normal cendance controls.

Do not assume a package is usable just because another agent advertises it.
cendance's local validator is the source of truth.

## Package Kinds Agents Should Understand First

- `effectPresetPack`
  - Use when sharing FX chains or parameter recipes for existing cendance FX.

- `soundPresetPack`
  - Use when sharing playable sounds that combine existing synth/sample engines,
    existing samples, FX choices, and macro defaults.
  - Good examples: bass patch collections, lead sounds, chord textures, drum
    channel setups that use existing engines.
  - This is not a new synth engine and not a sample-file bundle.

- `drumKitPresetPack`
  - Use when sharing drum slot mappings and slot parameters.
  - In the first version, prefer built-in or already-installed samples.

- `scenePresetPack`
  - Use when sharing a multi-track musical state or starting point.

- `arrangementPresetPack`
  - Use when sharing section structure and arrangement behavior.

Future kinds such as `samplePack`, `midiGeneratorPack`, `effectAlgorithmPack`,
and `instrumentEnginePack` require stricter rules and should not be assumed to
exist until the package schema says they are supported.

## Discovery Flow

When connected to a cendance-capable peer through the Pilot bridge:

1. Ask for package metadata, not package payloads.
2. Filter by kind, tags, compatibility, license, and trust status.
3. Prefer exact package ID plus content hash when requesting a transfer.
4. Avoid requesting packages that require unsupported schema versions or missing
   dependency kinds.

Good discovery questions:

- "List sound preset packs compatible with this cendance version."
- "Find effect preset packs tagged dub, space, or lo-fi."
- "Show packages by this trusted author agent."
- "Which packages have no missing dependencies?"

Poor discovery behavior:

- Downloading every advertised package.
- Installing based only on a name or description.
- Ignoring license/provenance.
- Treating network trust as install permission.

## Preview Flow

Preview is non-mutating. Use preview before install every time.

A useful preview result should tell the agent:

- Package kind, name, version, author, and source.
- Whether schema and app compatibility pass.
- Whether hash/signature checks pass.
- Which catalog items would be added.
- Which built-in catalogs or installed packages are referenced.
- Whether any dependencies are missing.
- Whether the package references samples that are unavailable locally.
- License and provenance warnings.
- Any fallback behavior cendance would use.

If preview reports missing dependencies, the agent should resolve those first or
skip install.

## Install Flow

Install only after a successful preview and explicit agent/user intent.

After install:

- Refresh the contribution catalog.
- Store installed item references by package ID and item ID.
- Apply contributions by stable reference, not by display index.
- Record the installed package hash, source peer, and signature status when
  available.

If install fails, the agent should report the validation error and leave the
package staged or remove it according to the user's preference.

## Applying Contributions

Agents should apply installed contributions through MCP tools or the app's agent
command surface, not by editing project files directly.

Suggested behavior by package kind:

- `effectPresetPack`
  - Apply an item to a track FX slot or master FX slot.
  - Keep existing routing constraints and slot counts.

- `soundPresetPack`
  - Apply an item to the intended track type when possible.
  - If the package says the sound is for bass, do not apply it to lead unless
    the manifest marks it as cross-track compatible.
  - Apply recommended FX and macro defaults only if the user requested a full
    sound application; otherwise apply the core sound only.

- `drumKitPresetPack`
  - Apply slot mappings and slot parameters to the drum track.
  - If samples are missing, prefer preview/install warnings over silent fallback.

- `scenePresetPack`
  - Treat as a broader project mutation.
  - Prefer asking for explicit intent before replacing multiple tracks, tempo,
    key, or arrangement state.

- `arrangementPresetPack`
  - Apply section structure without changing unrelated sound design unless the
    package explicitly includes scene-level data.

## Authoring Packages

When authoring a package, an agent should:

1. Start from an existing cendance state, catalog item, or selected project slice.
2. Choose the narrowest package kind that fits the contribution.
3. Include descriptive tags and clear compatibility information.
4. Reference built-in catalog items by stable identifiers.
5. Reference installed contribution dependencies by package ID and item ID.
6. Avoid embedding unsupported payload types.
7. Validate locally before advertising to peers.

Use `soundPresetPack` for sounds that are made from existing engines and FX:

- Track target: drums, bass, chords, or lead.
- Core sound source: existing synth preset, melodic sampler region reference, or
  drum kit reference.
- Optional FX recommendations: track FX slot presets and parameters.
- Optional macro defaults: density, complexity, tone, motion, gain.
- Optional musical hints: genre, tempo range, mood, key/mode suitability.

Do not use `soundPresetPack` for:

- New DSP code.
- New synth engines.
- New sample files that are not already installed or built in.
- New MIDI generation logic.

## Future Extension Expectations

Agents should be prepared for additional package kinds without changing the
core lifecycle.

For `samplePack`:

- Expect audio payloads, file hashes, format checks, license metadata, duration
  limits, and loudness/peak metadata.
- Do not install or apply samples without provenance and size checks.

For `midiGeneratorPack`:

- Expect a constrained non-native format first, such as a declarative pattern
  language or sandboxed module.
- Preview should explain the generator's inputs, output constraints, track
  compatibility, and CPU/memory limits.

For `effectAlgorithmPack` or `instrumentEnginePack`:

- Expect stricter trust requirements.
- Treat these as code-adjacent or code-bearing packages.
- Never install automatically from network discovery.

For `agentWorkflowPack`:

- Expect high-level recipes that call MCP tools rather than alter cendance internals.
- Keep workflows auditable and reversible when possible.

## Failure Handling

Agents should fail closed.

Reject or skip packages when:

- Schema is unsupported.
- Package kind is unknown.
- Hash or signature does not match.
- Required dependencies are missing.
- Manifest references paths outside the package.
- Numeric values exceed cendance's allowed ranges.
- License is missing or incompatible with the user's request.
- The package asks for native code execution in a data-only context.

When a package is skipped, explain the reason in terms of package validation,
not vague network failure.

## Agent Etiquette

- Discover broadly, install narrowly.
- Prefer reversible changes.
- Preserve the user's existing project unless explicitly asked to transform it.
- Share manifests before payloads.
- Treat creative authorship and license metadata as first-class.
- Let cendance validate everything before claiming a contribution is usable.
