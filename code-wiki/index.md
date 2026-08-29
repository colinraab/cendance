# Code Wiki Index

> Agent-readable knowledge map for cendance. Read this first when working in this codebase.
> Last updated: 2026-05-14 | Total pages: 19

## Entities (Classes/Struct)

### App Layer
- [[AppState]] — Atomic shared state (BPM, playing, track params, arrangement)
- [[CommandQueue]] — Lock-free SPSC queue for UI→Audio commands
- [[MeterQueue]] — Lock-free SPSC queue for Audio→UI metering data
- [[Command]] — Command struct with Type enum and encoding helpers
- [[KeyMapping]] — Maps keyboard inputs to Command actions
- [[AgentCommand]] — Agent protocol command execution (thin dispatcher)
- [[AgentCommandUtils]] — Shared helpers (string utils, parsers, JSON builders, dispatch)
- [[AgentCommandState]] — State/catalog/meters/listen commands
- [[AgentCommandTransport]] — Transport commands (play/pause/stop/tempo)
- [[AgentCommandTrack]] — Track parameter commands
- [[AgentCommandMaster]] — Master bus commands
- [[AgentCommandMusical]] — Musical commands (key/progression/arrangement)
- [[AgentCommandPackages]] — Package and preset commands
- [[AlgorithmPresetRegistry]] — Custom algorithm preset CRUD and indexing
- [[CustomAlgorithmPreset]] — Custom algorithm preset data model
- [[ContributionPackage]] — P2P contribution package model + Library class
- [[ContributionPackageParse]] — JSON parsing/serialization for packages
- [[ProjectIO]] — Project snapshot, validation, apply, path/recent management
- [[ProjectIOLoad]] — JSON serialization/deserialization for project files
- [[PresetRef]] — Preset reference wrapper
- [[PresetRegistry]] — Base preset registry
- [[DrumSampleLibrary]] — Drum sample file management
- [[MelodicSampleLibrary]] — Melodic sample file management
- [[GenreCatalog]] — Genre definitions and metadata
- [[EffectPresetCatalog]] — Effect preset metadata and defaults
- [[SpotEffectCatalog]] — Spot FX trigger metadata and defaults
- [[SynthCatalog]] — Synth preset metadata
- [[DrumKitPresetCatalog]] — Drum kit preset metadata
- [[MelodicSampleCatalog]] — Melodic sample metadata
- [[StartupProjectInitializer]] — Project initialization on startup

### Audio Layer
- [[AudioEngine]] — Main audio callback, owns DSP chain, processes commands via CommandProcessor
- [[EffectProcessor]] — Owns all 40+ effect instances, effect configuration and processing
- [[CommandProcessor]] — Command dispatch from CommandQueue, custom algorithm storage
- [[Transport]] — BPM clock, playhead position, beat/bar tracking
- [[GenerativeAlgorithm]] — Abstract base for all MIDI generators
- [[DrumEngine]] — Drum synthesis engine
- [[BassEngine]] — Bass synthesis engine
- [[ChordEngine]] — Chord synthesis engine
- [[LeadEngine]] — Lead synthesis engine
- [[MelodicSampler]] — Melodic sample playback engine
- [[SoundEngine]] — Base synth engine interface
- [[CustomAlgorithmInstance]] — Runtime instance of a custom algorithm
- [[MasterEffect]] — Base class for master bus effects

### UI Layer
- [[TuiApp]] — Main FTXUI application loop and input handling (thin router: handleEventInput → modal handlers)
- [[TuiAppInput]] — Shared input constants and helpers (tui_input namespace)
- [[AgentProtocolServer]] — TCP server for agent protocol connections
- [[AgentInputBar]] — Agent command input bar component
- [[TrackPanel]] — Track display panel component
- [[TransportBar]] — Transport controls display component
- [[ParameterBar]] — Parameter display bar component
- [[ArrangementModal]] — Arrangement editing modal
- [[NumberSelectionModal]] — Number selection modal
- [[KeyEntryModal]] — Key selection modal
- [[ProjectPathModal]] — Project save/load path modal
- [[SpectrumView]] — Spectrum analyzer display component

### MCP Layer
- [[McpServer]] — MCP JSON-RPC server (stdio transport)
- [[McpJsonHelpers]] — MCP JSON argument extraction and result builders

### Network Layer
- [[P2PClient]] — P2P preset/sample/algorithm sharing client
- [[P2PDownloadRegistry]] — P2P download tracking registry

### Security Layer
- [[SecurityManager]] — Content signing and verification
- [[PresetSerializer]] — Preset serialization/deserialization
- [[ContentHeader]] — Content type header
- [[ToSGuard]] — Terms of service acceptance guard

### Config Layer
- [[ToSGuard]] — Terms of service acceptance

## Concepts (Patterns/Subsystems)

- [[ThreadModel]] — 3-thread architecture (Main, JUCE Message, Audio)
- [[CrossThreadCommunication]] — Lock-free SPSC queues and atomics
- [[CommandPattern]] — Command encoding, dispatch, and execution
- [[GenerativeAlgorithms]] — MIDI algorithm interface and built-in algorithms
- [[CustomAlgorithms]] — Custom algorithm preset system (sparse IDs, registry)
- [[EffectSystem]] — Insert FX vs Spot FX domains
- [[PresetSystem]] — Preset catalogs, slots, and assignment
- [[P2PPresetSharing]] — P2P preset/sample/algorithm sharing
- [[AgentProtocol]] — Agent command protocol (TUI and MCP)
- [[MCPIntegration]] — MCP stdio server and tool dispatch
- [[ProjectFileFormat]] — Project save/load format
- [[HarmonySystem]] — Scales, chord progressions, key mapping
- [[DrumSampleSystem]] — Drum sample slots, assignment, parameters
- [[ArrangementSystem]] — Song arrangement sections, chains, progression
- [[ProfilingSystem]] — Audio callback profiling and metrics

## Relationships (Data Flow)

- [[UI-to-Audio-Flow]] — Keystroke to audio output path
- [[Audio-to-UI-Flow]] — Metering data to display path
- [[MCP-to-Audio-Flow]] — MCP command to audio effect path
- [[AgentProtocol-Flow]] — Agent protocol to command dispatch
- [[P2P-to-PresetRegistry-Flow]] — P2P download to preset installation
