# DAW Compatibility — Spatial Reactor v2

## Tested Configurations

### REAPER v7.74 (Windows 11, x64)
| Test | Result | Notes |
|------|--------|-------|
| VST3 scan | PASS | Scanned without errors |
| Plugin load | PASS | Instantiates on stereo track |
| GUI render | PASS | Full JUCE GUI with Reactor Core, all controls |
| Parameter interaction | PASS | Knobs, selectors, toggles respond |
| Editor open/close | PASS | No crashes |
| Plugin unload | PASS | Clean removal from FX chain |

### pluginval (strictness 10)
| Test | Result |
|------|--------|
| Open/close editor | PASS |
| Parameter fuzz | PASS |
| Bus layout changes | PASS |
| State save/restore | PASS |
| All strictness-10 tests | PASS |

## VST3 Installation Path
`C:\Program Files\Common Files\VST3\Spatial Reactor.vst3`

## Channel Configuration
- Input: Stereo (2 channels)
- Output: Stereo (2 channels)
- Mono input → stereo output: Not supported (requires stereo input)

## Parameters (APVTS)
| ID | Name | Range | Default |
|----|------|-------|---------|
| amount | Amount | 0.0–1.0 | 0.0 |
| density | Density | 0.0–1.0 | 0.5 |
| texture | Texture | 0.0–1.0 | 0.0 |
| textureMode | Texture Mode | Silk/Dust/Rust | Silk |
| air | Air | 0.0–1.0 | 0.0 |
| trustMode | Trust Mode | Safe/Free | Safe |
| profile | Profile | Auto/Vocal/Instrument/Master/Ambient | Auto |
