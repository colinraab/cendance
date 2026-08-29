# cendance — Codebase Overview & Development Guide

This guide provides a high-level understanding of the `cendance` codebase, architecture, and the standard procedures for adding new features. Coding agents should refer to this document to understand the repository structure quickly and adhere to established patterns when contributing.

## Architecture at a Glance

`cendance` is a keyboard-driven generative music application operating entirely within the terminal. It uses JUCE for headless audio and DSP, and FTXUI for the terminal user interface.

### Thread Model (3 Threads)

The application relies on three distinct threads. It is absolutely critical that lock-free data structures are used for cross-thread communication, especially when interacting with the real-time audio thread.

1. **Main Thread (UI)**: Runs the FTXUI `ScreenInteractive::Loop()`. Responsible for keyboard capture, rendering the terminal UI, and dispatching commands.
2. **JUCE Message Thread (Background)**: Runs `juce::MessageManager`. Handles timer callbacks, async updates, and non-RT housekeeping.
3. **Audio Thread (Real-Time)**: Triggered by `juce::AudioDeviceManager` callback. Responsible for real-time DSP, the sequencer clock, executing generative algorithms, and processing audio effects.

### Communication Flow

The Main Thread (UI) and the Audio Thread (Real-Time) communicate constantly, but they must NEVER block each other.
- **UI → Audio (`CommandQueue`)**: A lock-free SPSC (Single-Producer, Single-Consumer) queue. Keystrokes are mapped to `Command` structs and pushed to the audio thread for real-time execution.
- **Audio → UI (`MeterQueue`)**: A lock-free SPSC queue. Carries DSP metering, current beat tracking, and UI feedback data at audio rate back to the main thread for rendering.
- **`AppState`**: Contains atomic configurations managed on the Audio Thread to ensure synchronized parameter states (density, complexity, BPM, algos).

### Effect Domains

`cendance` now treats Insert FX and Spot FX as separate domains:

- **Insert FX (preset + slot domain)**
	- Assignable via selector flow (`F`) to track/master slots.
	- Persisted in project files through `effectPresetSlots` and `masterEffectPresetSlots`.
	- Catalog and ID mapping live in `EffectPresetCatalog`.

- **Spot FX (trigger domain)**
	- Triggered by dedicated commands (`SpotEffectOn`, `SpotEffectOff`, `SpotEffectToggle`).
	- Runtime-only and intentionally reset on project load (not persisted).
	- Metadata/defaults live in `SpotEffectCatalog`.

---

## Directory Structure

Key components of the system are categorized by their domain in the `Source/` folder. Always put new code in the corresponding subdirectory.

```
Source/
├── App/                # App-level concurrency & state
│   ├── AppState.h      # Atomic parameter state shared across threads
│   ├── CommandQueue.h  # Lock-free UI → Audio commands
│   ├── MeterQueue.h    # Lock-free Audio → UI feedback
│   ├── KeyMapping.h    # Maps UI inputs to Command actions
│   └── SpotEffectCatalog.h # Spot FX trigger metadata + defaults
│
├── Audio/              # Core DSP System
│   ├── AudioEngine     # Main wrapper resolving AudioDeviceManager callbacks
│   ├── MixBus          # Bus summing and Master Effects chain processing
│   ├── Transport       # BPM clock, global beat/bar syncing
│   ├── Effects/        # DSP Master Effects (TapeStop, Reverb, etc.)
│   ├── Generators/     # MIDI Generative Algorithms (Drums, Bass, Chords, Lead)
│   ├── Synths/         # Actual Sound Engines handling Synthesis/MIDI
│   └── Harmony/        # Scale mapping & Progression definitions
│
├── UI/                 # Terminal UI Layer
│   ├── TuiApp.h/.cpp   # Main UI loop running FTXUI fullscreen
│   ├── Components/     # Modular FTXUI visual elements (TrackPanel, Meters, etc.)
│   └── Themes/         # UI styling and ANSI colors
└── Main.cpp            # Entry point syncing JUCE with FTXUI
```

---

## 🛠 Feature Development Checklist

When implementing new features, making layout changes, or editing DSP, follow these checklists carefully to preserve the thread-safety and architecture of `cendance`.

### 1. General Rules
- [ ] Ensure any newly added `.cpp` files are included in the `CMakeLists.txt` (under `target_sources` for `cendance`).
- [ ] No allocations (`new`/`delete`, vectors resizing, standard mutexes, file I/O, `std::cout`) are allowed on the Audio Thread.
- [ ] Changes to state must pass through lock-free SPSC queues (`CommandQueue`) or atomic variables (`AppState`).

### 2. Adding New Keybindings & Controls
- [ ] Define the behavior mapping inside `Source/App/KeyMapping.h` and the command enum in `Source/App/CommandQueue.h` if needed.
- [ ] **Crucial UI Requirement**: If the keybind affects user operations, you **must add the new keybind documentation to the Help Popup in the TUI** (typically found or referenced in `Source/UI/`). 
- [ ] Do not handle UI commands instantly; always dispatch them to the `CommandQueue` and let the Audio Engine process them, or map them functionally in UI side if it's purely a visual change.
- [ ] Keep command intent explicit: slot assignment flows are Insert domain commands; trigger gestures are Spot domain commands.

### 3. Adding New Audio Effects or Generators
- [ ] To add a Generator, implement the `GenerativeAlgorithm` interface, add the behavior logic inside `Source/Audio/Generators/`, add it to `AlgorithmCatalog`, and wire the owned instance in `AudioEngine::initializeAlgorithmMap()`.
- [ ] To add a Synth, create a new sub-class of `SoundEngine` inside `Source/Audio/Synths/`.
- [ ] To add a Master Effect, extend the `MasterEffect` interface and add the effect to the chain processed in `Source/Audio/MixBus` and `Source/Audio/Effects/`.
- [ ] For **Insert FX presets**, add/update entries in `EffectPresetCatalog` and keep slot assignment validation in sync.
- [ ] For **Spot FX**, add trigger metadata/defaults in `SpotEffectCatalog`, then wire command handling + processing in `AudioEngine` without introducing slot assignment pathways.
- [ ] Confirm proper memory initialization in `prepareToPlay()` or `prepare()` methods where sample rates and buffer allocations are specified. Reset state on `reset()`.

### 4. Updating the UI (FTXUI)
- [ ] TUI elements are constructed functionally using FTXUI. Do not maintain long-lived component state needlessly—rebuild the tree efficiently based on data obtained periodically from the `MeterQueue`.
- [ ] Ensure TUI stays strictly constrained within terminal dimensions using FTXUI's layout flexboxes and decorators. Try minimizing deeply nested closures to keep the rendering loop light. 

### 5. Testing Strategy (Required)
- [ ] Add or update targeted unit-style tests under `Tests/` for any logic changes in `App/`, `Audio/Harmony/`, `Audio/Transport`, or key mapping.
- [ ] Keep tests deterministic and free of external dependencies (no audio device assumptions, no filesystem/network IO).
- [ ] For real-time sensitive code paths, test pure decision/state logic around the path rather than callback wall-clock behavior.
- [ ] Minimum test run before handoff: changed-area tests + `ctest --test-dir build --output-on-failure`.
- [ ] Current test binaries: `cendance_tests` (queues), `cendance_harmony_tests` (harmony), `cendance_keymapping_tests` (input mapping), `cendance_core_tests` (AppState/Transport/harmony helpers).
