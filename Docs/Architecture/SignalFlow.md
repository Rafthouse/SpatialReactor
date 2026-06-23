# Signal Flow — Spatial Reactor v2

```
Input L/R (stereo)
    │
    ├─ 0. loudnessComp.captureInput()     ← raw energy before DC blocker
    │
    ├─ 1. DC Blocker (L, R)               ← 1st-order high-pass, fc ≈ 5 Hz
    │
    ├─ 2. preAnalyzer.analyze()            ← pre-DSP stereo snapshot
    │     balanceGuard.captureInputBalance()
    │
    ├─ 3. M/S Encode                       ← energy-preserving: M,S = (L±R) / √2
    │
    ├─ 4. Source Classifier                ← vocal/instrument/master/ambient (or manual)
    │
    ├─ 5. MacroMapper                      ← amount + profile → width/texture/air targets
    │
    ├─ 6. energyBudget.captureInput()      ← M/S energy before processing
    │
    ├─ 7. Complementary Spatial Generator  ← cascaded allpass decorrelation on Side
    │     (Direct Form II, 4 bands, per-stage coefficients)
    │
    ├─ 8. Texture Engine                   ← oversampled saturation on Side (Silk/Dust/Rust)
    │
    ├─ 9. Air Engine                       ← HF shelf exciter on Side
    │
    ├─ 10. Energy Budget Mixer             ← normalize M/S energy, limit >2× growth only
    │
    ├─ 11. Center Lock                     ← LF mono guard (Side low-shelf reduction)
    │
    ├─ 12. Correlation Governor            ← pull Side toward zero if correlation < threshold
    │
    ├─ 13. M/S Decode                      ← L,R = (M±S) / √2
    │
    ├─ 14. Channel Balance Guard           ← restore L/R balance to match input
    │
    ├─ 15. Loudness Compensation           ← linear-domain energy matching (input vs output)
    │
    ├─ 16. True-Peak Limiter               ← -0.3 dBTP ceiling, soft-knee
    │
    └─ 17. postAnalyzer.analyze()          ← correlation + trimDb → GUI atomics
         Output L/R
```

## Key Design Principles

1. **Energy preservation**: 1/√2 M/S matrix ensures encode→decode round-trip is unity gain.
2. **Loudness transparency**: Compensator captures raw input energy (before DC blocker) and matches output energy in linear domain, avoiding Jensen's inequality bias.
3. **DC immunity**: Input DC blockers prevent LF energy from biasing loudness measurement. Texture engine DC is managed by energy budget normalization.
4. **Balance safety**: ChannelBalanceGuard measures input L/R ratio and applies corrective gain after M/S decode to prevent drift.
5. **Correlation safety**: Governor reduces Side content when correlation drops below threshold, with Trust Mode (SAFE/FREE) controlling aggressiveness.
6. **Per-block coefficients**: All smoothing coefficients are computed per-block in `prepare()` using `exp(-blockSize / (sampleRate × timeConstant))`.
