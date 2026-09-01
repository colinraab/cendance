# Installing cendance

cendance v0.1.0 is available as an unsigned macOS 15 developer preview for
Apple silicon and Intel Macs. You can run the release executable in place,
install it on your `PATH`, or build it from source.

## Download the macOS Release

Open [the v0.1.0 release](https://github.com/colinraab/cendance/releases/tag/v0.1.0)
and download the archive and matching `.sha256` file for your Mac:

- Apple silicon: `cendance-0.1.0-Darwin-arm64.tar.gz`
- Intel: `cendance-0.1.0-Darwin-x86_64.tar.gz`

Run `uname -m` if you are unsure which architecture you have. It prints
`arm64` on Apple silicon and `x86_64` on an Intel Mac.

### Verify and extract

Keep the archive and checksum file in the same directory. For Apple silicon:

```bash
cd ~/Downloads
shasum -a 256 -c cendance-0.1.0-Darwin-arm64.tar.gz.sha256
tar -xzf cendance-0.1.0-Darwin-arm64.tar.gz
cd cendance-0.1.0-Darwin-arm64
./bin/cendance --help
./bin/cendance
```

For Intel, replace `arm64` with `x86_64`. Continue only if `shasum` reports
`OK`.

The release executable includes its required libsodium code. You do not need
Homebrew or a separate libsodium installation to run the downloaded archive.

### Allow the unsigned executable

The preview is not signed with an Apple Developer ID or notarized. Gatekeeper
may block its first launch even when the checksum is correct. Do not disable
Gatekeeper globally.

After the first blocked launch:

1. Open **System Settings > Privacy & Security**.
2. Scroll to **Security** and select **Open Anyway** for `cendance`.
3. Confirm the prompt, then run `./bin/cendance` again.

Apple documents this per-application exception in
[Open a Mac app from an unknown developer](https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unknown-developer-mh40616/mac).
Only override Gatekeeper after you trust the source and the checksum reports
`OK`.

As a terminal-only alternative, remove the quarantine attribute from the
verified executable only:

```bash
xattr -d com.apple.quarantine bin/cendance
./bin/cendance
```

If `xattr` reports that the attribute does not exist, no quarantine attribute
was attached; run the executable normally.

### Optional command-line installation

To make `cendance` available outside the extracted directory:

```bash
sudo install -m 755 bin/cendance /usr/local/bin/cendance
cendance --help
```

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

Source builds are local binaries and normally do not inherit the quarantine
attribute applied to browser downloads.

## MCP Mode

Use the same binary with `--mcp`:

```json
{
  "mcpServers": {
    "cendance": {
      "command": "/absolute/path/to/cendance-0.1.0-Darwin-arm64/bin/cendance",
      "args": ["--mcp"]
    }
  }
}
```

If you installed the executable into `/usr/local/bin`, use
`/usr/local/bin/cendance` as the command instead. Start the executable once and
complete any required Gatekeeper exception before configuring an MCP client.

## Windows

Windows builds are not supported yet. The codebase still has POSIX-only runtime
paths and the localhost agent protocol is disabled on Windows. See
`docs/DISTRIBUTION_NEXT_STEPS.md` for the Windows porting and packaging plan.
