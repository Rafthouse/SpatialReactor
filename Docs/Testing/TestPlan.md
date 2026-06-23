# Test Plan — Spatial Reactor v2

## Automated Baseline Tests (15 tests)

### Group 1: Identity (Amount=0)
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 1.1 Mono sine identity | 1kHz mono sine | loudness ±0.3 dB, balance ±0.1 dB, correlation ≥0.99, null <-40 dBFS | Bypass path is transparent |
| 1.2 Dual-mono identity | Dual-mono 1kHz | Same as 1.1 | Stereo bypass transparent |

### Group 2: Loudness Preservation
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 2.1 Mono sine Amount=0.5 | 1kHz mono | loudness ±0.3 dB | Compensator tracks sine |
| 2.2 Mono sine Amount=1.0 | 1kHz mono | loudness ±0.3 dB | Full width compensated |
| 2.3 Pink noise Amount=0.7 FREE | Pink noise | loudness ±0.3 dB | Broadband compensation |

### Group 3: Stereo Balance
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 3.1 Pink noise Amount=0.5 SAFE | Pink noise | balance ±0.15 dB | No L/R drift at moderate width |
| 3.2 Pink noise Amount=1.0 Rust | Pink noise | balance ±0.15 dB | Extreme settings don't drift |

### Group 4: Channel-Swap Symmetry
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 4.1 Left-only → swap check | 1kHz left only | dominant ch ≤1.0 dB | Processing is symmetric |
| 4.2 Right-only → swap check | 1kHz right only | dominant ch ≤1.0 dB | Mirror of 4.1 |

### Group 5: Correlation Floor
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 5.1 Mono sine Amount=0.5 SAFE | 1kHz mono | correlation ≥0.8 | SAFE mode preserves mono compat |
| 5.2 Mono sine Amount=1.0 FREE | 1kHz mono | correlation ≥-0.5 | FREE allows wider but not anti-phase |
| 5.3 Decorrelated noise Amount=0.5 | Decorrelated noise | correlation ≥-0.5 | Don't make bad signals worse |

### Group 6: Mono Fold-Down
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 6.1 Mono sine Amount=0.5 | 1kHz mono | fold ≥-0.5 dB | Mid content preserved |
| 6.2 Pink noise Amount=0.7 | Pink noise | fold ≥-0.5 dB | Broadband fold-down safe |

### Group 7: DC Offset
| Test | Signal | Threshold | Verifies |
|------|--------|-----------|----------|
| 7.1 Sine Amount=1.0 Rust texture=0.5 | 1kHz mono | DC <-60 dBFS | DC blockers working |

## pluginval Tests
- Strictness level: 10
- Timeout: 120 seconds
- All JUCE plugin validation tests must pass

## DAW Compatibility Tests
- REAPER v7.74: scan, load, GUI, unload
- Ableton Live 12: scan, load, GUI, unload (if available)
- See DAWCompatibility.md for results

## Test Infrastructure
- Test executable: `SpatialReactor_Baseline.exe`
- Sample rate: 48000 Hz, block size: 512
- Convergence: 500 blocks rendered, first 100 blocks skipped (warmup)
- CSV output: `Tests/Baseline/baseline_measurements.csv`
