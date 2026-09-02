# Cendance repository instructions

## Project purpose
- `cendance` is a terminal-first generative music app: FTXUI UI + JUCE audio runtime.
- The executable target is `cendance` (console app), with tests `cendance_tests`.

## Architecture you must preserve
- Keep the 3-thread model intact: main/UI thread, JUCE message thread, and real-time audio callback thread (`Source/Main.cpp`).
- Cross-thread communication is lock-free SPSC only:
  - UI -> audio via `CommandQueue` (`Source/App/CommandQueue.h`)
  - audio -> UI via `MeterQueue` (`Source/App/MeterQueue.h`)
- Shared runtime parameters are atomics in `AppState` (`Source/App/AppState.h`); avoid introducing mutex-protected shared state for hot paths.
- UI key events map to `Command` values in `mapKeyToCommand()` (`Source/App/KeyMapping.cpp`), then get consumed on audio thread in `AudioEngine::processCommands()` (`Source/Audio/AudioEngine.cpp`).

## Real-time audio constraints (critical)
- Assume `AudioEngine::audioDeviceIOCallbackWithContext()` is real-time critical (`Source/Audio/AudioEngine.cpp`).
- Do not add blocking work, locks, file I/O, heap churn, logging, or long loops in audio callback paths.
- Allocate/prepare DSP state in `audioDeviceAboutToStart()` and `prepare*` methods, not per-sample/per-block unless trivial.
- Keep spot-effect toggles as bitmask-driven state (`AppState::activeSpotEffects`) and process in fixed order after insert/master chains as currently done.

## UI and input patterns
- `TuiApp::run()` owns the FTXUI event/render loop and polls `MeterQueue::popLatest()` at ~30 FPS (`Source/UI/TuiApp.cpp`).
- Track selection (`1-4`) is UI-local; parameter changes should go through command queue.
- If you add/change keybindings, update BOTH:
  - mapping in `Source/App/KeyMapping.cpp`
  - help overlay text in `Source/UI/TuiApp.cpp`

## Build/test workflows
- Prefer VS Code CMake Tools for configure/build over ad-hoc terminal build commands when available.
- Preferred local defaults on macOS ARM:
  - Kit: `Clang`
  - Generator: `Ninja`
  - Build type: `Debug`
- Configure: `cmake -B build`
- Build app + tests: `cmake --build build`
- Run app: `./build/cendance_artefacts/Release/cendance [--device "Device Name"] [--mcp]`
- Run tests: `./build/cendance_tests`
- Run full suite: `ctest --test-dir build --output-on-failure`
- Testing strategy for agent changes:
  - Prefer deterministic, allocation-free logic tests for `App/`, `Audio/Harmony/`, and `Audio/Transport`.
  - Use headless integration tests for `AudioEngine` command/state pathways (no real device initialization).
  - Avoid tests that require real audio devices or timing-sensitive sleeps unless unavoidable.
  - When changing command/key behavior, update or add key mapping tests and verify `Command` payload fields.
  - For queue/threading changes, run `cendance_tests` in addition to targeted tests.
  - After targeted tests pass, run the full CTest suite before handoff.
- macOS note: `CMakeLists.txt` contains a CommandLineTools C++ header workaround; preserve it unless explicitly replacing with a verified fix.
- If generator changes cause a juceaide mismatch, move the existing build
  directory to the Trash and reconfigure.

## Extension points
- New generators: implement under `Source/Audio/Generators/` and register in `AlgorithmFactory`.
- Harmony/progression behavior lives in `Source/Audio/Harmony/` (`Scale`, `ChordProgression`).
- New master effects should follow existing `Effects/` structure and be wired in `AudioEngine` prepare + process flow.

## Repository boundaries
- `JUCE/` is vendored upstream framework code; avoid edits there unless the task explicitly requires JUCE internals.
- Prefer changes under `Source/` and `Tests/`.
- Read `docs/CODEBASE_OVERVIEW.md` before making architectural changes. For
  package exchange or network work, also read `docs/SHARING_ARCHITECTURE.md`.
