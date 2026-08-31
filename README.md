# cendance

cendance is a terminal-first generative music app with a built-in MCP server.
It uses JUCE for audio/DSP and FTXUI for the keyboard-driven interface.

## Status

- macOS is the primary path today. CI validates macOS 15 on both Apple Silicon
  and Intel; CMake targets macOS 13 by default, but macOS 13–14 runtime
  compatibility is not yet release-validated.
- The v0.1.0 release is planned as an unsigned macOS developer preview;
  Gatekeeper may warn because Developer ID signing and notarization are not yet
  configured.
- Windows is not release-ready yet.

## Quick Start

```bash
git clone --recurse-submodules https://github.com/colinraab/cendance.git
cd cendance

brew install cmake ninja libsodium
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release

./build-release/cendance_artefacts/cendance
```

Run the test suite:

```bash
ctest --test-dir build-release --output-on-failure
```

Create a redistributable `.tar.gz` containing the executable and license
notices:

```bash
cmake --build build-release --target package
```

## MCP Mode

cendance includes its MCP stdio server in the main binary:

```json
{
  "mcpServers": {
    "cendance": {
      "command": "/absolute/path/to/cendance",
      "args": ["--mcp"]
    }
  }
}
```

## Useful Docs

- Build guide: [docs/BUILD.md](docs/BUILD.md)
- Install notes: [INSTALL.md](INSTALL.md)
- Distribution plan: [docs/DISTRIBUTION_NEXT_STEPS.md](docs/DISTRIBUTION_NEXT_STEPS.md)
- Public repository checklist: [docs/PUBLIC_REPOSITORY_CHECKLIST.md](docs/PUBLIC_REPOSITORY_CHECKLIST.md)
- Codebase overview: [docs/CODEBASE_OVERVIEW.md](docs/CODEBASE_OVERVIEW.md)
- Bundled asset licensing: [Resources/LICENSE.md](Resources/LICENSE.md)
- Third-party notices: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)
- Terms template: [docs/TERMS_OF_SERVICE.md](docs/TERMS_OF_SERVICE.md)
- Contributing guide: [CONTRIBUTING.md](CONTRIBUTING.md)
- Security policy: [SECURITY.md](SECURITY.md)
- Code of conduct: [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)

## License

cendance source code is licensed under the
[GNU Affero General Public License version 3](LICENSE) (`AGPL-3.0-only`). The
original drum and melodic samples are dedicated under
[CC0 1.0 Universal](Resources/LICENSE.md). Third-party components and impulse
responses retain their respective licenses; see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and the resource notices.

## Release Checklist

Before inviting broad public use:

1. Confirm the macOS arm64 and Intel Release CI jobs pass on GitHub.
2. Publish the CI-built macOS archives with SHA-256 checksums.
3. Add Apple Developer ID signing and notarization for the polished macOS release.
4. Complete the Windows portability checklist before advertising Windows support.
