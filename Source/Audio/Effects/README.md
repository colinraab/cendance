# Effects Organization and Selector Rules

This directory is organized by numeric effect categories used by the FX number selector.

## Folder taxonomy

- `0_Dynamics`
- `1_Space`
- `2_Distortion`
- `3_Filters`
- `4_Modulation`
- `5_Pitch`
- `6_Degrade`
- `7_Rhythm`
- `8_Granular`
- `9_SpectralResonators`
- `Spot` (trigger-only spot effects)

## FX selector grammar

- Assign effect: `[slot][category][preset]`
  - `slot`: `1` to `3`
  - `category`: `0` to `9`
  - `preset`: `1+` digits, 1-based index inside that category
  - example: `101` means slot 1, category 0 (Spectral/Resonators), preset 1
- Clear slot: `[slot]-`
  - examples: `1-`, `2-`, `3-`

### Important validation rules

- `10`, `20`, `30` are invalid.
- Category `0` is Spectral/Resonators, not clear.
- Category-only input (for example `10`) is partial and invalid for submit.
- Spot effects are excluded from slot assignment and remain trigger-only.

## Popup staged preview behavior

- After first digit: `Slot N`
- After second character:
  - category digit: `Slot N -> Category`
  - dash: `Slot N -> Clear`
- After full valid assign input: `Slot N -> Preset Name`

## Current implemented insert-category mapping
 
- Category 0 (Spectral/Resonators): `PhysicalModelingResonator`, `SpectralBlur`, `SpectralDelay`
- Category 1 (Dynamics): `CompressorGlue`, `PeakLimiter`, `TransientShaper`, `MultibandOtt`
- Category 2 (Space): `ReverbWash`, `DelayEcho`, `ConvolutionReverb`, `TapeDelay`, `PingPongDelay`
- Category 3 (Distortion): `SaturationWaveshaper`, `SoftHardClip`, `Wavefolder`, `AsymShaper`
- Category 4 (Filters): `HighPassSweep`, `CombFilter`, `MultiModeEQ`, `FormantFilter`
- Category 5 (Modulation): `RingModulator`, `Chorus`, `Phaser`, `Flanger`
- Category 6 (Pitch): `FrequencyShifter`, `PitchShifter`, `Harmonizer`
- Category 7 (Degrade): `ReduxCrush`, `JitterDegrade`, `ErosionDegrade`
- Category 8 (Rhythm): `Autopan`, `TranceGate`, `SidechainDucker`, `BeatRepeatInsert`
- Category 9 (Granular): `TimeFreezer`, `GrainDelay`, `CloudGenerator`

Spot effects live in `Spot`:
- `TapeStop`
- `BeatRepeat`

## When adding or moving effects

1. Place files in the correct category folder.
2. Update includes in `Source/Audio/AudioEngine.h`.
3. Update source paths in `CMakeLists.txt` for both app and audio integration tests.
4. Update `Source/App/EffectPresetCatalog.h`:
   - `EffectType`
   - preset entries
   - type to category mapping for selector resolution
5. Keep spot-only effects out of slot-assignable mapping.
6. Update the category `TODO.md` file.
