# Public Repository Checklist

Use this checklist to track the private cendance repository and decide when it
is ready to become public.

## Curated snapshot

Create the new repository from one reviewed source commit with fresh Git
history. Do not copy the source repository's `.git` directory.

Include:

- `.github/`, `.gitignore`, `.gitattributes`, and `.gitmodules`
- `CMakeLists.txt`, `Source/`, and `Tests/`
- `Resources/`, including the required drum kits, melodic samples, impulse
  responses, manual, and asset license notice
- `README.md`, `INSTALL.md`, `THIRD_PARTY_NOTICES.md`, `docs/`, and `code-wiki/`
- `.vscode/settings.json`, which contains shareable project settings
- the JUCE Git submodule pinned to the source commit's exact revision

Exclude:

- Git history and source-repository remotes
- build directories, CMake output, release packages, coverage, and analysis
  output
- `.DS_Store` files and empty directories
- local editor, development-tool, and assistant state
- local recordings, projects, logs, credentials, certificates, and signing
  material

Empty drum-kit directories require no special handling. Git does not store
empty directories, and any `.DS_Store` files inside them are ignored.

## Required public-release gates

- [x] Review and commit the intended source snapshot.
- [x] Add AGPL-3.0 as the root software license, compatible with the AGPLv3
      option used for JUCE.
- [x] Confirm `Resources/LICENSE.md` accurately states the CC0-1.0 grant for
      original drum and melodic samples.
- [x] Replace the Voxengo-derived impulse responses with documented CC0 assets.
- [x] Confirm `git ls-files -ci --exclude-standard` produces no output.
- [x] Run a secret scanner against the curated snapshot.
- [x] Confirm no tracked file contains the old project name or private absolute
      filesystem paths.
- [x] Clone the private repository with `--recurse-submodules` into a clean
      directory.
- [x] Configure, build, and run the full test suite from that clean clone.
- [x] Review the complete initial commit on GitHub before changing visibility.
- [x] Add macOS 15 arm64 and Intel Release CI definitions that retain the
      packaged archives as workflow artifacts.
- [x] Add dependency-update automation for GitHub Actions and the JUCE
      submodule.
- [x] Add contributing, security, and community conduct policies.
- [x] Add a CPack archive containing the executable and required notices.
- [x] Confirm both macOS 15 CI jobs pass all 22 tests on GitHub.
- [x] Protect `main` with required CI, review, conversation-resolution, and
      linear-history rules.
- [x] Enable dependency vulnerability alerts and automated security fixes.
- [ ] Enable private vulnerability reporting after making the repository
      public. GitHub does not offer this setting while the repository is
      private.
- [x] Publish arm64 and x86_64 preview archives with SHA-256 checksum files.
- [x] Label the first archive clearly as an unsigned developer preview.
- [x] Document checksum verification, Gatekeeper's per-application exception,
      and the exact archive layout.
- [ ] Sign and notarize a polished macOS release.

## History and identity

Fresh history avoids carrying old repository objects into the public project.
Before the initial commit, configure the author email that should be visible to
the public, such as a GitHub-provided no-reply address.
