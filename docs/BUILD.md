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

- Kit: `Clang 21.0.0 arm64-apple-darwin25.4.0`
- Generator: `Ninja`
- Build type: `Debug`
- If switching generators (e.g. Makefiles -> Ninja), clear stale caches with `rm -rf build` before reconfigure.

### Quick VS Code commands

- `CMake: Select a Kit` -> choose `Clang 21.0.0 arm64-apple-darwin25.4.0`
- `CMake: Select Variant` -> choose `Debug`
- `CMake: Delete Cache and Reconfigure`
- `CMake: Build`

## Quick Build

```bash
# Configure (first time fetches FTXUI and builds juceaide)
/opt/homebrew/bin/cmake -B build -G Ninja

# Build
/opt/homebrew/bin/cmake --build build

# Run
./build/cendance_artefacts/cendance
```

## Release Build

```bash
/opt/homebrew/bin/cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
/opt/homebrew/bin/cmake --build build-release
ctest --test-dir build-release --output-on-failure

# Release executable
./build-release/cendance_artefacts/cendance --help
```

## CLI Options

```bash
# Use a specific audio device (useful on Windows or multi-device setups)
./build/cendance_artefacts/cendance --device "MacBook Pro Speakers"

# Run the embedded MCP stdio server
./build/cendance_artefacts/cendance --mcp
```

## Clean Rebuild

```bash
rm -rf build
/opt/homebrew/bin/cmake -B build
/opt/homebrew/bin/cmake --build build
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

**Permanent fix (recommended):**
```bash
sudo rm -rf /Library/Developer/CommandLineTools/usr/include/c++/v1
```
This removes only the 3 stale files. The real C++ headers remain safely in the SDK.

**Alternative:** The project's `CMakeLists.txt` includes an automatic workaround
that detects this condition and passes `-nostdinc++ -isystem <SDK>/usr/include/c++/v1`
to the compiler. This runs automatically — no manual intervention needed.

### `cmake: command not found`

CMake is installed via Homebrew at `/opt/homebrew/bin/cmake`. If your shell
can't find it, ensure Homebrew's bin is in your PATH:

```bash
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
source ~/.zprofile
```

### `runDispatchLoopUntil` error

If you see this error, it means `JUCE_MODAL_LOOPS_PERMITTED` is disabled (the
default for console apps). The project uses `runDispatchLoop()` /
`stopDispatchLoop()` instead, which doesn't require modal loops.

## Project Structure

```
cendance/
├── CMakeLists.txt          # Build config (JUCE + FTXUI)
├── JUCE/                   # JUCE framework (git submodule)
├── Source/
│   └── Main.cpp            # Entry point
└── build/                  # Build output (gitignored)
    └── cendance_artefacts/
        └── cendance          # The executable
```

## Dependencies

| Dependency | Version | How |
|-----------|---------|-----|
| JUCE | latest (git submodule) | `git submodule update --init` |
| FTXUI | v6.1.9 | Auto-fetched by CMake via `FetchContent` |
