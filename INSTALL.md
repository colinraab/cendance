# Installing cendance

cendance is currently published as a source-buildable macOS terminal app.
Packaged releases are not available yet.

## macOS From Source

```bash
git clone --recurse-submodules https://github.com/colinraab/cendance.git
cd cendance

brew install cmake ninja libsodium
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure

./build-release/cendance_artefacts/cendance
```

## MCP Mode

Use the same binary with `--mcp`:

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

## Manual macOS Release Install

Once GitHub release tarballs exist:

```bash
tar xzf cendance-macos-arm64.tar.gz
chmod +x cendance
sudo mv cendance /usr/local/bin/cendance
brew install libsodium
cendance --help
```

If the release is not signed and notarized, macOS Gatekeeper may warn when the
downloaded binary is first run. The public release path should move to a signed
and notarized package.

## Windows

Windows builds are not supported yet. The codebase still has POSIX-only runtime
paths and the localhost agent protocol is disabled on Windows. See
`docs/DISTRIBUTION_NEXT_STEPS.md` for the Windows porting and packaging plan.
