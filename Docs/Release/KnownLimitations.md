# Known Limitations — Spatial Reactor v2

## 1. Single-channel Input Asymmetry
**What**: When processing a mono signal panned hard left or hard right, the quiet channel receives significantly less energy than the mirror case (~18 dB difference in quiet channel).
**Why**: Fundamental to M/S-based spatial widening. A left-only signal encodes to equal M and S. Processing S creates decorrelated content that decodes asymmetrically relative to a right-only input because the allpass filter state evolves differently.
**Impact**: Minimal in practice — real-world audio is always stereo or dual-mono. The dominant channel remains within 0.5 dB of the mirror case.

## 2. Mono Fold-Down at Maximum Width
**What**: At Amount=1.0, mono fold-down can lose up to -0.59 dB of Mid content.
**Why**: Maximum widening adds significant Side energy. Loudness compensator reduces total energy to match input, which proportionally reduces Mid.
**Impact**: At typical settings (Amount ≤ 0.7), mono fold is within ±0.03 dB. Only extreme settings show measurable loss.

## 3. 1kHz Balance at Moderate Width
**What**: Pure 1kHz sine shows up to 0.05 dB L/R balance shift at Amount=0.5.
**Why**: Allpass decorrelation at a single frequency creates a fixed phase offset that biases one channel. Broadband signals average this out.
**Impact**: Inaudible. Pink noise shows balance within ±0.05 dB at all settings.

## 4. Correlation at Full Width + FREE Mode
**What**: Correlation drops to ~0.88 at Amount=1.0 in FREE mode (vs 0.97 in SAFE).
**Why**: FREE mode allows wider spatial spread. Governor threshold is relaxed.
**Impact**: By design — FREE mode is for creative use where wider stereo image is desired.

## 5. Latency
**What**: Zero-latency design — no lookahead or oversampling on the main signal path.
**Why**: Real-time performance priority. TextureEngine uses internal 2× oversampling on Side only.
**Impact**: No PDC (plugin delay compensation) required.
