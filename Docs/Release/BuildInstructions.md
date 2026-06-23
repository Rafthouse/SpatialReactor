# Build Instructions — Spatial Reactor

## Prerequisites
- CMake 4.x+
- Visual Studio 2022 (MSVC v143 toolset)
- JUCE (included as submodule in `JUCE/`)
- Windows 10/11 x64

## Build Steps

### Release VST3
```bash
cd C:\Users\rafth\SpatialReactor
cmake -B Build -DCMAKE_BUILD_TYPE=Release
cmake --build Build --config Release --target SpatialReactor_VST3
```

Output: `Build/SpatialReactor_artefacts/Release/VST3/Spatial Reactor.vst3`

### Baseline Tests
```bash
cmake --build Build --config Release --target SpatialReactor_Baseline
./Build/SpatialReactor_Baseline_artefacts/Release/SpatialReactor_Baseline.exe
```

### Install (deploy to system VST3 folder)
```bash
cmake --install Build --config Release
```
Deploys to: `C:\Program Files\Common Files\VST3\Spatial Reactor.vst3`

### Validation
```bash
pluginval.exe --validate "Build/SpatialReactor_artefacts/Release/VST3/Spatial Reactor.vst3" --strictness-level 10
```

## Project Structure
```
SpatialReactor/
├── Source/
│   ├── PluginProcessor.cpp/h    — main audio processing
│   ├── PluginEditor.cpp/h       — GUI editor
│   ├── dsp/                     — DSP modules
│   │   ├── EnergyPreservingMS.h
│   │   ├── ComplementarySpatialGenerator.h
│   │   ├── LoudnessCompensator.h
│   │   ├── EnergyBudgetMixer.h
│   │   ├── ChannelBalanceGuard.h
│   │   ├── NewCorrelationGovernor.h
│   │   ├── DCBlocker.h
│   │   ├── TruePeakLimiter.h
│   │   └── ... (texture, air, classifier, etc.)
│   └── gui/                     — GUI components
├── Tests/Baseline/              — automated test suite
├── Docs/                        — documentation
├── Resources/                   — fonts, images
├── JUCE/                        — JUCE framework (submodule)
└── CMakeLists.txt
```
