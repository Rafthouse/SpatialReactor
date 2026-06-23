# Final Report — Spatial Reactor Core v2 Rebuild

Date: 2026-06-23
Branch: `rebuild/spatial-reactor-core-v2`

## Verdict: REBUILD COMPLETE

The Spatial Reactor DSP core has been fully rebuilt from scratch. All 7 conditions that forced the rebuild (per spec section 4) have been resolved. All 15 automated baseline tests pass. pluginval passes at strictness 10. REAPER DAW testing confirms scan, load, GUI, and unload.

## Root Causes Found and Fixed

### 1. Left-channel drift (4.25–10.54 dB → 0.05 dB)
- **Root cause**: Additive Side injection without DC blockers + non-energy-preserving M/S matrix (0.5/1.0).
- **Fix**: Energy-preserving 1/√2 M/S matrix, input DC blockers, energy budget mixer, channel balance guard.

### 2. Loudness increase (+1.19 to +1.82 dB → ±0.03 dB)
- **Root cause**: Three modules added Side energy with no normalization. M/S decode amplified Side 2×.
- **Fix**: Energy budget mixer (limits >2× growth), loudness compensator (linear-domain energy matching).
- **Subtlety**: Loudness compensator `captureInput()` must be called BEFORE DC blockers, not after. DC blockers remove LF energy from pink noise; if measured post-DC-blocker, compensator sees reduced input and under-corrects.

### 3. DC offset (-26 dBFS → -89.9 dBFS)
- **Root cause**: TextureEngine Rust mode even-order distortion generating DC in Side.
- **Fix**: Input DC blockers + energy budget normalization.

### 4. Correlation meter (wrong formula → correct Pearson)
- **Root cause**: Used `1 - rms_s/rms_m` instead of standard `Σ(L·R)/√(Σ(L²)·Σ(R²))`.
- **Fix**: Post-DSP L/R Pearson correlation in StereoAnalyzer, fed to GUI via atomic.

### 5. Heap allocations in processBlock (4 per call → 0)
- **Root cause**: TextureEngine, AirEngine, CenterLock, SourceClassifier allocated juce::AudioBuffer in process().
- **Fix**: All buffers pre-allocated in prepare().

### 6. Missing modules (7 → all implemented)
- EnergyBudgetMixer, LoudnessCompensator, ChannelBalanceGuard, TruePeakLimiter, DCBlocker, NewCorrelationGovernor, StereoAnalyzer — all implemented and integrated.

## Architecture Diagram

```
L/R → captureInput → DC Block → Analyze → M/S Encode
    → Classify → Macro Map → Budget Capture
    → Spatial Gen → Texture → Air
    → Budget Apply → Center Lock → Corr Governor
    → M/S Decode → Balance Guard → Loudness Comp
    → Peak Limiter → Post Analyze → L/R out
```

## Changed Files

### New DSP Modules
- `Source/dsp/EnergyPreservingMS.h` — 1/√2 encode/decode
- `Source/dsp/ComplementarySpatialGenerator.h` — Direct Form II allpass cascade
- `Source/dsp/LoudnessCompensator.h` — linear-domain energy matching
- `Source/dsp/EnergyBudgetMixer.h` — M/S energy normalization
- `Source/dsp/ChannelBalanceGuard.h` — L/R balance restoration
- `Source/dsp/NewCorrelationGovernor.h` — per-block correlation guard
- `Source/dsp/DCBlocker.h` — 1st-order high-pass
- `Source/dsp/TruePeakLimiter.h` — -0.3 dBTP soft-knee limiter
- `Source/dsp/StereoAnalyzer.h` — pre/post stereo analysis
- `Source/dsp/NewTextureEngine.h` — oversampled Side saturation
- `Source/dsp/NewAirEngine.h` — HF shelf exciter
- `Source/dsp/NewCenterLock.h` — LF mono guard
- `Source/dsp/NewSourceClassifier.h` — spectral source classification
- `Source/dsp/MacroMapper.h` — amount/profile to DSP targets

### Core Files Modified
- `Source/PluginProcessor.cpp` — complete rewrite of processBlock signal flow
- `Source/PluginProcessor.h` — all DSP module members
- `Source/PluginEditor.cpp/h` — JUCE GUI with APVTS binding

### GUI Components
- `Source/gui/GuiTheme.h` — dark theme constants
- `Source/gui/GuiLayout.h` — layout coordinates
- `Source/gui/SpatialRotaryKnob.h` — custom rotary with readout
- `Source/gui/SegmentedSelector.h` — mode/profile selector
- `Source/gui/TrustToggle.h` — SAFE/FREE toggle
- `Source/gui/CorrelationMeter.h` — real-time correlation bar
- `Source/gui/ReactorCoreComponent.h` — animated reactor visualization
- `Source/gui/ReadoutStrip.h` — bottom info strip
- `Source/gui/FaceplateComponent.h` — background faceplate

### Test Files
- `Tests/Baseline/BaselineTests.cpp` — 15 automated tests

## Measurement Table: Baseline vs Final

| Metric | Baseline (v1) | Final (v2) | Threshold | Status |
|--------|--------------|------------|-----------|--------|
| Loudness Δ (sine 0.5) | +1.19 dB | -0.00 dB | ±0.3 dB | PASS |
| Loudness Δ (sine 1.0) | +1.19 dB | -0.01 dB | ±0.3 dB | PASS |
| Loudness Δ (pink 0.7) | +1.82 dB | +0.03 dB | ±0.3 dB | PASS |
| Balance (sine 0.5) | 4.25 dB | 0.05 dB | ±0.15 dB | PASS |
| Balance (pink 0.5) | 4.25 dB | -0.05 dB | ±0.15 dB | PASS |
| Balance (pink 1.0 Rust) | 4.25 dB | -0.00 dB | ±0.15 dB | PASS |
| DC offset L | -26 dBFS | -93.9 dBFS | <-60 dBFS | PASS |
| DC offset R | -26 dBFS | -89.9 dBFS | <-60 dBFS | PASS |
| Correlation (SAFE 0.5) | N/A | 0.97 | ≥0.8 | PASS |
| Correlation (FREE 1.0) | N/A | 0.88 | ≥-0.5 | PASS |
| Null depth (bypass) | N/A | -55.04 dBFS | <-40 dBFS | PASS |
| Mono fold (sine 0.5) | N/A | -0.06 dB | ≥-0.5 dB | PASS |
| Mono fold (pink 0.7) | N/A | +0.03 dB | ≥-0.5 dB | PASS |
| Swap symmetry (dominant) | 4.27 dB | 0.51 dB | ≤1.0 dB | PASS |
| pluginval strictness 10 | FAIL | PASS | PASS | PASS |

## Test Results Summary
- Baseline tests: **15/15 PASSED**
- pluginval strictness 10: **PASSED**
- REAPER v7.74: scan ✓, load ✓, GUI ✓, unload ✓

## Build Artifacts
- VST3: `Build/SpatialReactor_artefacts/Release/VST3/Spatial Reactor.vst3`
- Deployed: `C:\Program Files\Common Files\VST3\Spatial Reactor.vst3`

## GUI Screenshot
REAPER v7.74 with Spatial Reactor loaded — Reactor Core centered on Amount knob, all controls visible: Source Profile (AUTO), Density/Texture/Air knobs, Mode selector (SILK), Trust toggle (SAFE), Correlation meter, MYKYTA SHCHUR branding.
