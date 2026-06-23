# Mathematical Foundations — Spatial Reactor v2

## 1. Energy-Preserving M/S Matrix

```
Encode: M = (L + R) / √2,  S = (L - R) / √2
Decode: L = (M + S) / √2,  R = (M - S) / √2
```

Property: `|L|² + |R|² = |M|² + |S|²` (energy-preserving).
Previous version used `M = (L+R)*0.5, L = M+S` which amplified Side energy 2× at decode.

## 2. Allpass Decorrelation (Direct Form II)

Per-stage transfer function: `H(z) = (a + z⁻²) / (1 + a·z⁻²)`

State update (Direct Form II):
```
w₀ = x - a · s₁
y  = a · w₀ + s₁
s₁ = s₀
s₀ = w₀
```

4 stages with coefficients: `{0.6923878, 0.9360654322959, 0.9882295226860, 0.9987488452737}`

Property: |H(e^jω)| = 1 for all ω (allpass). Phase varies with frequency, creating decorrelation.

## 3. Loudness Compensation

Energy estimate (per block):
```
E = Σ(L² + R²) / (2·N)
```

Smoothed in linear domain (avoids Jensen's inequality):
```
Ê_in  = α · Ê_in  + (1-α) · E_in
Ê_out = α · Ê_out + (1-α) · E_out
α = exp(-blockSize / (sampleRate × 0.3))
```

Target gain: `g = √(Ê_in / Ê_out)`, clamped to [0.5, 2.0].
Gain smoothing: `g_smooth = β · g_smooth + (1-β) · g_target`, β with 50ms time constant.

## 4. Energy Budget Limiting

Only limits energy growth exceeding 2× input:
```
ratio = E_out / E_in
if ratio > 2.0:
    scale = √(2·E_in / E_out)
    apply scale to M and S
```

This prevents loudness explosions while allowing intentional widening to breathe.

## 5. DC Blocker

1st-order high-pass: `y[n] = x[n] - x[n-1] + R·y[n-1]`, R = exp(-2π·5/sampleRate) ≈ 0.9997.

## 6. Correlation Measurement

Standard Pearson formula on post-DSP L/R:
```
ρ = Σ(L·R) / √(Σ(L²) · Σ(R²))
```

Range: +1 (mono) to -1 (out-of-phase). Governor activates when ρ < threshold.

## 7. Channel Balance Guard

Per-block L/R RMS ratio captured at input, applied as corrective gain after M/S decode:
```
inputRatio = rmsL_in / rmsR_in
outputRatio = rmsL_out / rmsR_out
correction = inputRatio / outputRatio
```

Clamped to ±6 dB, smoothed with 50ms time constant.
