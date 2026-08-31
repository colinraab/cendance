# cendance Distribution Next Steps

Last reviewed: 2026-08-31

## Current State

cendance is close to an early public macOS release, but it is not yet packaged
as a consumer-ready download.

- App target: terminal-first JUCE + FTXUI executable, not a `.app` bundle.
- Primary platform: macOS. CI validates macOS 15 on arm64 and x86_64. CMake
  targets macOS 13 by default, but macOS 13–14 runtime compatibility is not yet
  release-validated.
- Build system: CMake + JUCE submodule + FTXUI FetchContent + libsodium.
- Tests: 22 CTest targets, with assertions kept active in Release builds.
- MCP: built into the same binary; launch with `cendance --mcp`.
- Packaging: CPack creates a `.tar.gz` containing the executable, software
  license, audio license, IR provenance, and third-party notices.
- Windows: not release-ready. Some code paths are still POSIX-only and the
  localhost agent protocol reports "not implemented on Windows yet."

## Publish Blockers

Address these before inviting broad external users:

1. Confirm the macOS arm64 and x86_64 Release jobs pass in GitHub Actions.
2. Publish the first release as an explicitly unsigned developer preview.
3. Publish SHA-256 checksums for every preview artifact.
4. Decide whether CPack tarballs remain acceptable or whether the first polished
   macOS artifact should be a signed `.pkg` installer.
5. Enable GitHub private vulnerability reporting and recommended branch
   protections before making the repository public.
6. Do a Windows portability pass before advertising Windows support.

## macOS Distribution

Apple's current public docs say Mac software distributed outside the App Store
should use a Developer ID certificate and notarization so it passes Gatekeeper
under default settings:

- Apple Developer Program: https://developer.apple.com/programs/
- Developer ID: https://developer.apple.com/developer-id/
- Notarization: https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution

Apple Developer Program membership is currently listed by Apple as 99 USD per
membership year. You need that membership for Developer ID signing.

### Recommended Early Path

For a terminal executable, the cleanest public macOS artifact is a signed and
notarized `.pkg` that installs `cendance` into `/usr/local/bin` or another
documented location. A plain `.tar.gz` is easier for previews, but a `.pkg`
gives users a standard installer and can be stapled after notarization.

Apple allows notarizing ZIP, PKG, and DMG containers. Apple also documents that
ZIP archives cannot be stapled directly, so prefer `.pkg` for a command-line
tool if you want offline Gatekeeper validation.

### Local Release Build

```bash
cmake -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
ctest --test-dir build-release --output-on-failure

cmake --build build-release --target package
shasum -a 256 build-release/cendance-*.tar.gz
```

The current Ninja output path is `build-release/cendance_artefacts/cendance`;
there is no `cendance_artefacts/Release/` directory in a single-config Ninja
build.

### Signing and Notarization Sketch

Set these locally or in CI secrets:

```bash
export CENDANCE_DEVELOPER_ID_APP="Developer ID Application: Your Name (TEAMID)"
export CENDANCE_DEVELOPER_ID_INSTALLER="Developer ID Installer: Your Name (TEAMID)"
export CENDANCE_NOTARY_PROFILE="cendance-notary"
```

Sign the executable with hardened runtime:

```bash
codesign --force --timestamp --options runtime \
  --sign "$CENDANCE_DEVELOPER_ID_APP" \
  dist/macos-arm64/cendance

codesign --verify --strict --verbose=2 dist/macos-arm64/cendance
spctl -a -vv -t execute dist/macos-arm64/cendance
```

Package, sign, notarize, staple, and validate:

```bash
# Move an existing dist/pkgroot to the Trash before rerunning this sequence.
mkdir -p dist/pkgroot/usr/local/bin
cp dist/macos-arm64/cendance dist/pkgroot/usr/local/bin/cendance

pkgbuild \
  --root dist/pkgroot \
  --identifier app.cendance.cli \
  --version 0.1.0 \
  --install-location / \
  --sign "$CENDANCE_DEVELOPER_ID_INSTALLER" \
  dist/cendance-macos-arm64.pkg

xcrun notarytool submit dist/cendance-macos-arm64.pkg \
  --keychain-profile "$CENDANCE_NOTARY_PROFILE" \
  --wait

xcrun stapler staple dist/cendance-macos-arm64.pkg
xcrun stapler validate dist/cendance-macos-arm64.pkg
spctl -a -vv -t install dist/cendance-macos-arm64.pkg
```

No entitlements file is currently documented here because the app is a command
line audio tool and no specific entitlement need has been identified. Add only
the entitlements required by a concrete notarization or runtime failure.

### macOS Continuous Integration

`.github/workflows/ci.yml` builds, tests, and packages Release on macOS 15 arm64
and Intel. Both archives are retained as short-lived workflow artifacts. Keep
polished release publishing separate from pull-request CI: a tag-triggered
release workflow should sign, notarize, checksum, and upload only after both
build jobs pass.

## Homebrew

A separate tap is a good early macOS distribution channel after the first
GitHub Release exists.

```ruby
class Cendance < Formula
  desc "Terminal generative music app with an embedded MCP server"
  homepage "https://github.com/colinraab/cendance"
  url "https://github.com/colinraab/cendance/releases/download/v0.1.0/cendance-0.1.0-Darwin-arm64.tar.gz"
  sha256 "REPLACE_WITH_SHA256"
  version "0.1.0"
  license "AGPL-3.0-only"

  depends_on "libsodium"

  def install
    bin.install "cendance"
  end

  test do
    assert_match "cendance options", shell_output("#{bin}/cendance --help")
  end
end
```

Users would install with:

```bash
brew tap YOUR_HOMEBREW_TAP/cendance
brew install cendance
```

## Windows Status and Deployment

Do not advertise Windows support yet. Before building Windows artifacts, port
or guard the remaining POSIX-only runtime paths:

- `Source/Main.cpp` includes `<unistd.h>` unconditionally for `gethostname`.
- `Source/Mcp/McpStdio.cpp` uses POSIX `sigaction`, `fd_set`, `select`, and
  `STDIN_FILENO` without a Windows implementation.
- `Source/UI/AgentProtocolServer.cpp` explicitly returns "not implemented on
  Windows yet."

After those are fixed, add a `windows-latest` CI job using Visual Studio 2022,
CMake, Ninja, and a reliable libsodium source such as vcpkg. Make sure the
release artifact includes any required runtime DLLs.

For Windows distribution, the current Microsoft guidance is:

- Microsoft Store/MSIX is the most complete path and Microsoft signs Store
  MSIX packages after certification:
  https://learn.microsoft.com/en-us/windows/apps/package-and-deploy/code-signing-options
- MSIX packages distributed outside the Store must be signed:
  https://learn.microsoft.com/en-us/windows/msix/package/signing-package-overview
- For website or GitHub `.exe`/`.zip` releases, plan on code signing. Unsigned
  binaries are likely to show Microsoft Defender SmartScreen warnings.
- As of May 2026, Microsoft announced free company developer registration, and
  individual registration was already free in the newer onboarding flow:
  https://blogs.windows.com/windowsdeveloper/2026/05/07/publish-to-microsoft-store-as-a-company-now-with-free-registration-and-faster-onboarding/

Recommended sequence for Windows:

1. Port the POSIX-only source paths.
2. Add Windows CI build and test.
3. Produce a zip for developer testing.
4. Choose Store/MSIX for the lowest-friction trusted install path.
5. Add winget only after there is a stable signed installer or Store package.

## MCP Client Configuration

The MCP server is built into the cendance binary. Clients must pass `--mcp`.

```json
{
  "mcpServers": {
    "cendance": {
      "command": "/usr/local/bin/cendance",
      "args": ["--mcp"]
    }
  }
}
```

After Homebrew install:

```json
{
  "mcpServers": {
    "cendance": {
      "command": "cendance",
      "args": ["--mcp"]
    }
  }
}
```

## Recommended Release Order

1. Confirm macOS arm64 and Intel CI pass on GitHub.
2. Enable private vulnerability reporting and branch protection.
3. Publish CPack preview tarballs with SHA-256 checksums.
4. Enroll in the Apple Developer Program.
5. Add Developer ID signing and notarized `.pkg` generation.
6. Create a Homebrew tap.
7. Invite the first external macOS users.
8. Port Windows runtime gaps and add Windows CI.
9. Package Windows via Store/MSIX or a signed installer.
