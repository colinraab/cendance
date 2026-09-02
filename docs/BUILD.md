# cendance — Build Guide

## Prerequisites

- **macOS** (arm64 or x86_64) is the currently supported local build platform
- **CMake** ≥ 3.24 — install via `brew install cmake`
- **Ninja** (recommended generator) — `brew install ninja`
- **libsodium** — `brew install libsodium`
- **Command Line Tools** — `xcode-select --install`
- No Xcode.app required; CLT is sufficient

Windows is not release-ready yet. There are still POSIX-only paths in the
runtime/MCP server and the localhost agent protocol is explicitly disabled on
Windows. See [DISTRIBUTION_NEXT_STEPS.md](DISTRIBUTION_NEXT_STEPS.md) for the
Windows porting and packaging checklist.

## VS Code CMake Tools defaults (recommended)

- Kit: the Apple Clang compiler detected for your Mac
- Generator: `Ninja`
- Build type: `Debug`
- If switching generators (for example, Makefiles to Ninja), move the stale
  build directory to the Trash before reconfiguring.

### Quick VS Code commands

- `CMake: Select a Kit` -> choose the detected Apple Clang kit for your Mac
- `CMake: Select Variant` -> choose `Debug`
- `CMake: Delete Cache and Reconfigure`
- `CMake: Build`

## Quick Build

```bash
# Configure (first time fetches FTXUI and builds juceaide)
cmake -B build -G Ninja

# Build
cmake --build build

# Run
./build/cendance_artefacts/cendance
```

## Release Build

```bash
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure

# Create cendance-<version>-<system>-<architecture>.tar.gz
cmake --build build-release --target package

# Release executable
./build-release/cendance_artefacts/Release/cendance --help
```

## CLI Options

```bash
# Use a specific audio device (useful on Windows or multi-device setups)
./build/cendance_artefacts/cendance --device "MacBook Pro Speakers"

# Run the embedded MCP stdio server
./build/cendance_artefacts/cendance --mcp
```

## Clean Rebuild

Move `build/` to the Trash in Finder, then run:

```bash
cmake -B build -G Ninja
cmake --build build
```

## Troubleshooting

### `fatal error: 'algorithm' file not found`

This is a known macOS issue where stale C++ header files in the CommandLineTools
toolchain path shadow the real headers in the SDK.

**Symptoms:** Any C++ standard library header (`<algorithm>`, `<vector>`, etc.)
fails to compile, even outside the project.

**Root cause:** The directory `/Library/Developer/CommandLineTools/usr/include/c++/v1/`
contains 3 leftover files from an old CLT installation (`__functional_03`,
`__functional_base_03`, `__sso_allocator` — all dated April 2022). Modern CLT
ships all C++ stdlib headers inside the SDK at
`<SDK>/usr/include/c++/v1/`. The stale toolchain directory makes clang
think it has its own C++ headers, so it never searches the SDK.

The project's `CMakeLists.txt` includes an automatic workaround
that detects this condition and passes `-nostdinc++ -isystem <SDK>/usr/include/c++/v1`
to the compiler. This runs automatically, so no destructive toolchain cleanup
is required. Reinstall Apple's Command Line Tools if you want to repair the
system installation itself.

### `cmake: command not found`

Install CMake with `brew install cmake`. If Homebrew is installed but your
shell cannot find its commands, follow the `brew shellenv` instruction printed
by the Homebrew installer. The common setup commands are:

```bash
# Apple silicon
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile

# Intel
echo 'eval "$(/usr/local/bin/brew shellenv)"' >> ~/.zprofile
```

Run only the command for your Mac, then open a new terminal.

### `runDispatchLoopUntil` error

If you see this error, it means `JUCE_MODAL_LOOPS_PERMITTED` is disabled (the
default for console apps). The project uses `runDispatchLoop()` /
`stopDispatchLoop()` instead, which doesn't require modal loops.

## Project Structure

```
cendance/
├── CMakeLists.txt       # App, package, and test targets
├── JUCE/                # JUCE framework submodule
├── Resources/           # Manual, bundled audio, and asset licenses
├── Source/
│   ├── App/             # State, commands, packages, and persistence
│   ├── Audio/           # Engine, DSP, generators, synths, and harmony
│   ├── Config/          # User-data paths and sharing acknowledgement
│   ├── Mcp/             # Embedded MCP stdio server and tool handlers
│   ├── Network/         # Local/HTTP sharing backends and discovery
│   ├── Security/        # Signed envelopes and verification
│   ├── UI/              # FTXUI app and localhost agent protocol
│   └── Main.cpp         # Process startup and runtime orchestration
├── Tests/               # Unit and integration test executables
└── build/               # Generated output (ignored)
```

`CMakeLists.txt` currently registers 22 CTest targets. Use
`ctest --test-dir build -N` to list the authoritative set; do not maintain a
separate hard-coded test inventory in documentation.

## Dependencies

| Dependency | Version | How |
|-----------|---------|-----|
| JUCE | pinned submodule commit | `git submodule update --init --recursive` |
| FTXUI | v6.1.9, pinned commit | Auto-fetched by CMake via `FetchContent` |
| libsodium | 1.0.18 or newer | System package; statically linked on macOS when available |
