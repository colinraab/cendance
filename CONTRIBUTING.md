# Contributing to cendance

Thank you for helping improve cendance.

## Before you start

- Search existing issues and pull requests before opening a duplicate.
- Use an issue to discuss large behavioral, architecture, or file-format
  changes before investing substantial work.
- Do not contribute audio or other assets unless you have the right to release
  them under clearly documented terms.

## Build and test

Initialize the JUCE submodule, install CMake, Ninja, and libsodium, then follow
[`docs/BUILD.md`](docs/BUILD.md). Before opening a pull request, run:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Keep changes focused. Add or update tests when behavior changes, and update the
user-facing documentation when commands, formats, or platform support change.

## Licensing contributions

By submitting a contribution, you agree that your source-code contribution is
licensed under `AGPL-3.0-only`. Audio submitted for bundling must be original
and dedicated under CC0 1.0, or have separately reviewed redistribution terms
and provenance. Document third-party dependencies and assets in
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) or the relevant resource
notice.

All contributors must follow the [Code of Conduct](CODE_OF_CONDUCT.md).
