# Third-Party Notices

cendance depends on third-party software and assets. This file is an inventory,
not a replacement for the applicable license terms.

The cendance application source is licensed under `AGPL-3.0-only`; see the
root `LICENSE` file. The licenses below apply separately to the named
third-party components and assets.

## JUCE

JUCE is included as a Git submodule. The pinned checkout describes the JUCE
modules as dual-licensed under AGPLv3 or a commercial JUCE license. See
`JUCE/LICENSE.md` in the initialized submodule for the full terms and its own
dependency notices.

cendance uses JUCE under the AGPLv3 option. A distributor may instead rely on
an appropriate commercial JUCE license if they have obtained one directly
from JUCE.

## FTXUI

FTXUI is fetched by CMake at tag `v6.1.9` and is licensed under the MIT License.
Its copyright and license text are available in the fetched FTXUI source.

## libsodium

libsodium is a system dependency and is licensed under the ISC License. It is
not copied into this repository.

## Bundled audio

See `Resources/LICENSE.md` for the original cendance drum and melodic samples
and for the separate provenance and CC0 terms that apply to the bundled impulse
responses.
