cmake_minimum_required(VERSION 3.24)
project(SpatialReactor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Detect JUCE path — prefer environment, then sibling, then fallback
if(DEFINED ENV{JUCE_DIR})
    set(JUCE_DIR "$ENV{JUCE_DIR}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
else()
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
endif()

if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found at ${JUCE_DIR}. "
                        "Set JUCE_DIR environment variable or place JUCE at ${JUCE_DIR}")
endif()

# Add JUCE as a subdirectory to get all module targets
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_plugin(SpatialReactor
    PRODUCT_NAME            "Spatial Reactor"
    COMPANY_NAME            "Shchur Audio"
    COMPANY_WEBSITE         "https://github.com/rafth"
    COMPANY_EMAIL           ""
    VERSION                 1.0.0
    DESCRIPTION             "Spatial processing plugin with M/S matrix, decorrelation, texture, and air."
    BUNDLE_ID               "com.shchuraudio.spatialreactor"

    PLUGIN_MANUFACTURER_CODE Shir
    PLUGIN_CODE              SpRe

    # Formats
    FORMATS                  VST3 AU AUv3
    # VST3 target
    VST3_CATEGORY            "Fx|Spatial"

    # Plugin characteristics
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    EDITOR_PLUGIN_TYPE       Generic

    # Bus config
    INPUT_BUS_NAMES          "Input"
    INPUT_CHANNEL_SET        "{0, 1}"
    OUTPUT_BUS_NAMES         "Output"
    OUTPUT_CHANNEL_SET       "{0, 1}"
)

# --- JUCE modules ---
juce_generate_juce_header(SpatialReactor)

target_include_directories(SpatialReactor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_sources(SpatialReactor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/MSMatrix.h
        Source/dsp/SourceClassifier.h
        Source/dsp/HybridDecorrelator.h
        Source/dsp/TextureEngine.h
        Source/dsp/AirEngine.h
        Source/dsp/CorrelationGovernor.h
        Source/dsp/CenterLock.h
        Source/dsp/MacroMapper.h
        Source/widgets/BauhausButtonGroup.h
        Source/widgets/BauhausButtonGroup.cpp
        Source/widgets/BauhausRotaryKnob.h
        Source/widgets/BauhausRotaryKnob.cpp
        Source/widgets/BauhausCorrelationBar.h
        Source/widgets/BauhausCorrelationBar.cpp
        Source/widgets/BauhausSafeFreeToggle.h
        Source/widgets/BauhausSafeFreeToggle.cpp
        Source/GUI/GuiTheme.h
        Source/GUI/GuiLayout.h
        Source/GUI/SpatialRotaryKnob.h
        Source/GUI/SpatialRotaryKnob.cpp
        Source/GUI/SegmentedSelector.h
        Source/GUI/SegmentedSelector.cpp
        Source/GUI/TrustToggle.h
        Source/GUI/TrustToggle.cpp
        Source/GUI/CorrelationMeter.h
        Source/GUI/CorrelationMeter.cpp
        Source/GUI/ReactorCoreComponent.h
        Source/GUI/ReactorCoreComponent.cpp
        Source/GUI/ReadoutStrip.h
        Source/GUI/ReadoutStrip.cpp
        Source/GUI/FaceplateComponent.h
        Source/GUI/FaceplateComponent.cpp
)

# --- Binary resources (fonts, images) ---
juce_add_binary_data(SpatialReactor_Resources
    SOURCES
        Source/assets/Inter.ttf
        Source/assets/Inter-Bold.ttf
        Source/assets/PirataOne.ttf
        Source/assets/knob_body.svg
)

target_compile_definitions(SpatialReactor PRIVATE JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1)

target_link_libraries(SpatialReactor
    PRIVATE
        SpatialReactor_Resources
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_dsp
)

# ============================================================
# Hostile DAW Test Suite
# ============================================================
juce_add_console_app(SpatialReactor_Tests
    PRODUCT_NAME "SpatialReactor Test Suite"
    COMPANY_NAME "Shchur Audio"
    VERSION      1.0.0
)

target_sources(SpatialReactor_Tests
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/MSMatrix.h
        Source/dsp/SourceClassifier.h
        Source/dsp/HybridDecorrelator.h
        Source/dsp/TextureEngine.h
        Source/dsp/AirEngine.h
        Source/dsp/CorrelationGovernor.h
        Source/dsp/CenterLock.h
        Source/dsp/MacroMapper.h
        Source/widgets/BauhausButtonGroup.h
        Source/widgets/BauhausButtonGroup.cpp
        Source/widgets/BauhausRotaryKnob.h
        Source/widgets/BauhausRotaryKnob.cpp
        Source/widgets/BauhausCorrelationBar.h
        Source/widgets/BauhausCorrelationBar.cpp
        Source/widgets/BauhausSafeFreeToggle.h
        Source/widgets/BauhausSafeFreeToggle.cpp
        tests/main.cpp
        tests/HostileDAWTest.h
        tests/HostileDAWTest.cpp
)

target_compile_definitions(SpatialReactor_Tests
    PRIVATE
        JucePlugin_Name="Spatial Reactor"
        JucePlugin_IsMidiEffect=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsSynth=0
        JucePlugin_ManufacturerCode=0x53686972
        JucePlugin_PluginCode=0x53705265
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_PreferredChannelConfigurations
)

target_include_directories(SpatialReactor_Tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(SpatialReactor_Tests
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_dsp
        juce::juce_events
        juce::juce_core
)

# ============================================================
# Minimal test (isolate constructor crash)
# ============================================================
juce_add_console_app(SpatialReactor_MinTest
    PRODUCT_NAME "SpatialReactor MinTest"
    COMPANY_NAME "Shchur Audio"
    VERSION      1.0.0
)

target_sources(SpatialReactor_MinTest
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/MSMatrix.h
        Source/dsp/SourceClassifier.h
        Source/dsp/HybridDecorrelator.h
        Source/dsp/TextureEngine.h
        Source/dsp/AirEngine.h
        Source/dsp/CorrelationGovernor.h
        Source/dsp/CenterLock.h
        Source/dsp/MacroMapper.h
        Source/widgets/BauhausButtonGroup.h
        Source/widgets/BauhausButtonGroup.cpp
        Source/widgets/BauhausRotaryKnob.h
        Source/widgets/BauhausRotaryKnob.cpp
        Source/widgets/BauhausCorrelationBar.h
        Source/widgets/BauhausCorrelationBar.cpp
        Source/widgets/BauhausSafeFreeToggle.h
        Source/widgets/BauhausSafeFreeToggle.cpp
        tests/minimal_test.cpp
)

target_compile_definitions(SpatialReactor_MinTest
    PRIVATE
        JucePlugin_Name="Spatial Reactor"
        JucePlugin_IsMidiEffect=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsSynth=0
        JucePlugin_ManufacturerCode=0x53686972
        JucePlugin_PluginCode=0x53705265
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_PreferredChannelConfigurations
)

target_include_directories(SpatialReactor_MinTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(SpatialReactor_MinTest
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_events
        juce::juce_core
)

# ============================================================
# VST3 Stability Certification Test
# ============================================================
juce_add_console_app(SpatialReactor_CertTest
    PRODUCT_NAME "SpatialReactor CertTest"
    COMPANY_NAME "Shchur Audio"
    BUNDLE_ID "com.shchur-audio.spatialreactor.certtest"
    VERSION      1.0.0
)

target_sources(SpatialReactor_CertTest
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/MSMatrix.h
        Source/dsp/SourceClassifier.h
        Source/dsp/HybridDecorrelator.h
        Source/dsp/TextureEngine.h
        Source/dsp/AirEngine.h
        Source/dsp/CorrelationGovernor.h
        Source/dsp/CenterLock.h
        Source/dsp/MacroMapper.h
        Source/widgets/BauhausButtonGroup.h
        Source/widgets/BauhausButtonGroup.cpp
        Source/widgets/BauhausRotaryKnob.h
        Source/widgets/BauhausRotaryKnob.cpp
        Source/widgets/BauhausCorrelationBar.h
        Source/widgets/BauhausCorrelationBar.cpp
        Source/widgets/BauhausSafeFreeToggle.h
        Source/widgets/BauhausSafeFreeToggle.cpp
        tests/cert_test.cpp
        tests/seh_wrapper.c
)

target_compile_definitions(SpatialReactor_CertTest
    PRIVATE
        JucePlugin_Name="Spatial Reactor"
        JucePlugin_IsMidiEffect=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsSynth=0
        JucePlugin_ManufacturerCode=0x53686972
        JucePlugin_PluginCode=0x53705265
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_PreferredChannelConfigurations
)

target_include_directories(SpatialReactor_CertTest
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(SpatialReactor_CertTest
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_events
        juce::juce_core
)

# ============================================================
# Audio Check Test (verifies DSP changes audio)
# ============================================================
juce_add_console_app(SpatialReactor_AudioCheck
    PRODUCT_NAME "SpatialReactor AudioCheck"
    COMPANY_NAME "Shchur Audio"
    VERSION      1.0.0
)

target_sources(SpatialReactor_AudioCheck
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/dsp/MSMatrix.h
        Source/dsp/SourceClassifier.h
        Source/dsp/HybridDecorrelator.h
        Source/dsp/TextureEngine.h
        Source/dsp/AirEngine.h
        Source/dsp/CorrelationGovernor.h
        Source/dsp/CenterLock.h
        Source/dsp/MacroMapper.h
        Source/widgets/BauhausButtonGroup.h
        Source/widgets/BauhausButtonGroup.cpp
        Source/widgets/BauhausRotaryKnob.h
        Source/widgets/BauhausRotaryKnob.cpp
        Source/widgets/BauhausCorrelationBar.h
        Source/widgets/BauhausCorrelationBar.cpp
        Source/widgets/BauhausSafeFreeToggle.h
        Source/widgets/BauhausSafeFreeToggle.cpp
        tests/audio_check.cpp
)

target_compile_definitions(SpatialReactor_AudioCheck
    PRIVATE
        JucePlugin_Name="Spatial Reactor"
        JucePlugin_IsMidiEffect=0
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsSynth=0
        JucePlugin_ManufacturerCode=0x53686972
        JucePlugin_PluginCode=0x53705265
        JUCE_STANDALONE_APPLICATION=1
        JucePlugin_PreferredChannelConfigurations
)

target_include_directories(SpatialReactor_AudioCheck
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(SpatialReactor_AudioCheck
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_dsp
        juce::juce_events
        juce::juce_core
)
