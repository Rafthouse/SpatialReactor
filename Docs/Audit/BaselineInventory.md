# Baseline Inventory — Spatial Reactor

Date: 2026-06-22
Branch: rebuild/spatial-reactor-core-v2
Baseline commit: 868044b

## DSP Modules

| Module | File | Status | Verdict |
|--------|------|--------|---------|
| MSMatrix | `Source/dsp/MSMatrix.h` | 0.5-based encode, 1.0-based decode. Energy NOT preserved in M/S domain. | REBUILD |
| HybridDecorrelator | `Source/dsp/HybridDecorrelator.h` | Additive Side injection (mid→side) with no energy budget. Non-deterministic LFO. | REBUILD |
| TextureEngine | `Source/dsp/TextureEngine.h` | Heap alloc in processBlock. Rust mode has DC-generating x^2 term. 4x OS (1ch). | REBUILD |
| AirEngine | `Source/dsp/AirEngine.h` | Heap alloc in processBlock. Additive HF energy with no budget. Single IIR. | REBUILD |
| CenterLock | `Source/dsp/CenterLock.h` | Heap alloc in processBlock. Permanently disabled (centerLockEnabled=false). | REBUILD |
| CorrelationGovernor | `Source/dsp/CorrelationGovernor.h` | Uses peak ratio not correlation. Non-SR-dependent smoothing. No per-band. | REBUILD |
| SourceClassifier | `Source/dsp/SourceClassifier.h` | Heap alloc in audio thread. SmoothedValue misuse. | REBUILD |
| MacroMapper | `Source/dsp/MacroMapper.h` | Polynomial curves. Functional. | KEEP with modifications |

## APVTS Parameters

| ID | Type | Range | Default | Status |
|----|------|-------|---------|--------|
| amount | float | 0-1 | 0 | KEEP |
| density | float | 0-1 | 0.5 | KEEP |
| texture | float | 0-1 | 0 | KEEP |
| textureMode | choice | Silk/Dust/Rust | 0 | KEEP |
| air | float | 0-1 | 0 | KEEP |
| trustMode | bool | - | true | KEEP |
| profile | choice | Auto/Vocal/Inst/Master/Ambient | 0 | KEEP |

## Missing Modules (per spec)

- Energy-preserving M/S matrix (1/sqrt(2))
- Stereo Integrity Analyzer (pre/post)
- Complementary Spatial Generator
- Energy Budget Mixer
- Automatic Loudness Compensator
- Channel Balance Guard
- True-Peak Safety Trim
- DC Guard / DC Blockers
- Per-band Correlation Governor

## GUI

- v6 implemented (900x560 fixed)
- 16 GUI source files in Source/GUI/
- All APVTS attachments working
- Correlation meter connected (but to wrong formula)

## Build

- CMake 4.3.3 + VS Community 2026
- JUCE (sibling dir)
- Formats: VST3, AU, AUv3
- Test targets: SpatialReactor_Tests, _CertTest, _AudioCheck, _MinTest (all reference old widget files)

## Latency

- Current: 0 samples (getTailLengthSeconds = 0.0)
- TextureEngine uses 4x oversampling but doesn't report latency

## Known Heap Allocations in Audio Thread

1. TextureEngine::process — `juce::AudioBuffer<float> drySide(1, numSamples)`
2. AirEngine::process — `juce::AudioBuffer<float> band(1, numSamples)`
3. CenterLock::process — `juce::AudioBuffer<float> lf(1, numSamples)`
4. SourceClassifier::classify — `juce::AudioBuffer<float> sideCopy(1, numSamples)`

## Critical Defects Summary

1. **LOUDNESS**: No energy normalization. Additive Side processing amplified 2x by decode.
2. **LEFT DRIFT**: No DC blockers. Even-order harmonics (Rust x^2) create DC in Side → L/R asymmetry.
3. **CORRELATION METER**: Wrong formula (1-rms_s/rms_m instead of broadband correlation). Measures M/S pre-decode.
4. **MONO SAFETY**: No mono fold-down testing. No LF side guard active.
5. **ALLOCATIONS**: 4 heap allocations per processBlock call.
6. **SYMMETRY**: DC bias in Side processing breaks channel-swap symmetry.
