# Test Report — Spatial Reactor v2 Rebuild

Date: 2026-06-23
Branch: `rebuild/spatial-reactor-core-v2`
Build: Release, MSVC 2022, x86_64

## Baseline Tests: 15/15 PASSED

| # | Test | Result | Key Metric |
|---|------|--------|------------|
| 1.1 | Identity mono sine | PASS | Δ loudness: 0.00 dB, null: -55.04 dBFS |
| 1.2 | Identity dual-mono | PASS | Δ loudness: 0.00 dB, null: -55.04 dBFS |
| 2.1 | Loudness sine 0.5 | PASS | Δ loudness: -0.00 dB |
| 2.2 | Loudness sine 1.0 | PASS | Δ loudness: -0.01 dB |
| 2.3 | Loudness pink 0.7 FREE | PASS | Δ loudness: +0.03 dB |
| 3.1 | Balance pink 0.5 SAFE | PASS | Balance error: -0.05 dB |
| 3.2 | Balance pink 1.0 Rust | PASS | Balance error: -0.00 dB |
| 4.1 | Swap-A left-only | PASS | Dominant-ch diff: 0.51 dB |
| 4.2 | Swap-B right-only | PASS | (mirror test) |
| 5.1 | Corr mono 0.5 SAFE | PASS | Correlation: 0.97 |
| 5.2 | Corr mono 1.0 FREE | PASS | Correlation: 0.88 |
| 5.3 | Corr decorrelated | PASS | Correlation: 0.06 |
| 6.1 | Mono fold sine 0.5 | PASS | Fold diff: -0.06 dB |
| 6.2 | Mono fold pink 0.7 | PASS | Fold diff: +0.03 dB |
| 7.1 | DC offset Rust tex | PASS | DC: L=-93.9 R=-89.9 dBFS |

## pluginval: PASSED (strictness 10)

All JUCE plugin validation tests passed at maximum strictness.

## DAW Tests

| DAW | Version | Scan | Load | GUI | Unload |
|-----|---------|------|------|-----|--------|
| REAPER | v7.74 | PASS | PASS | PASS | PASS |

## Baseline vs Final Comparison

| Metric | Baseline (v1) | Final (v2) | Threshold |
|--------|--------------|------------|-----------|
| Loudness (sine 0.5) | +1.19 dB | -0.00 dB | ±0.3 dB |
| Loudness (pink 0.7) | +1.82 dB | +0.03 dB | ±0.3 dB |
| Balance (sine 0.5) | 4.25 dB | 0.05 dB | ±0.15 dB |
| Balance (pink 1.0) | 4.25 dB | -0.00 dB | ±0.15 dB |
| DC offset (Rust) | -26 dBFS | -89.9 dBFS | <-60 dBFS |
| Correlation SAFE | N/A | 0.97 | ≥0.8 |
| Null depth (bypass) | N/A | -55.04 dBFS | <-40 dBFS |
| Mono fold (pink 0.7) | N/A | +0.03 dB | ≥-0.5 dB |
