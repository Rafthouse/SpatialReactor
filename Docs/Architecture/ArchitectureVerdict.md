# Architecture Verdict: REBUILD

Date: 2026-06-22
Branch: rebuild/spatial-reactor-core-v2

## Verdict: REBUILD

The current architecture is fundamentally incompatible with the product requirements. Local patches cannot fix the issues because they are structural.

## Evidence

### 1. Left-channel drift: 4.25–10.54 dB
- **Root cause**: Additive Side injection without DC blockers. HybridDecorrelator adds all-pass filtered Mid to Side (line 48: `side[i] += decorrelated * width`). TextureEngine Rust mode generates DC via even-order harmonics (`y = x + 0.7*x*x + 0.3*x*x*x` — the x^2 term is always positive). No DC blocker exists anywhere in the chain. DC in Side channel becomes L/R asymmetry after M/S decode: `L = M + S_dc`, `R = M - S_dc`.
- **Measured**: Dual-mono 1kHz at Amount=0.5 → L=-7.16 dBFS, R=-11.41 dBFS (4.25 dB drift)
- **Why not fixable locally**: Every module that touches Side (decorrelator, texture, air) can introduce DC. Need architectural DC guard + per-module DC blockers.

### 2. Loudness increase: +1.19 to +1.82 dB
- **Root cause**: MSMatrix uses 0.5-based encode (`M=(L+R)*0.5`) but 1.0-based decode (`L=M+S`). Round-trip preserves signal, but any energy added to Side during M/S processing is amplified 2x at decode. Three modules add Side energy (decorrelator, texture exciter, air) with zero energy budget or normalization.
- **Measured**: Amount=1.0 SAFE → +1.19 dB. With Rust texture → +1.82 dB.
- **Why not fixable locally**: Even switching to 1/sqrt(2) matrix, all three processing modules are purely additive. Need energy budget mixer.

### 3. Channel-swap asymmetry: up to 4.27 dB
- **Root cause**: Combination of DC bias and non-symmetric allpass state.
- **Measured**: Left-only outR=-31.48 dBFS vs Right-only outL=-27.21 dBFS → 4.27 dB mirror difference.
- **Why not fixable locally**: Follows from #1 and #2.

### 4. DC offset: 0.05 linear (≈-26 dBFS)
- **Root cause**: TextureEngine Rust mode even-order distortion. No DC blocker after nonlinearity.
- **Measured**: L=+0.05, R=-0.05 (opposite polarity confirms Side-domain DC).

### 5. Heap allocations in processBlock: 4 per call
- TextureEngine, AirEngine, CenterLock, SourceClassifier each allocate juce::AudioBuffer in process().
- Requires preallocated buffers in prepare().

### 6. Correlation meter: wrong formula
- Uses `1 - rms_s/rms_m` instead of `sum(L*R)/sqrt(sum(L^2)*sum(R^2))`.
- Measured in M/S domain before decode, not post-DSP L/R.

### 7. Missing modules
- Energy Budget Mixer
- Automatic Loudness Compensation
- Channel Balance Guard
- True-Peak Safety Trim
- DC Guard / DC Blockers
- Per-band Correlation Governor
- Stereo Integrity Analyzer (pre/post)

## Conditions that force REBUILD (per spec section 4)

- [x] Uncontrolled side gain
- [x] Missing energy normalization
- [x] Missing real post-DSP correlation analysis
- [x] Allocations in processBlock
- [x] Left/right permutation test fails
- [x] Channel-asymmetric state (DC bias)

## New Target Architecture

See `Docs/Architecture/SignalFlow.md`
