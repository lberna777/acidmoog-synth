cmake_minimum_required(VERSION 3.22)

project(AcidMoog VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: si usa una copia locale se esiste (-DJUCE_PATH=... o clone accanto al repo),
# altrimenti viene scaricata automaticamente. Cosi' su una macchina nuova basta
# clonare questo repo e lanciare cmake, senza preparare nulla a mano.
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE-shared")
endif()

if(EXISTS "${JUCE_PATH}/CMakeLists.txt")
    message(STATUS "AcidMoog: uso JUCE locale in ${JUCE_PATH}")
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    message(STATUS "AcidMoog: JUCE non trovato in ${JUCE_PATH}, scarico da GitHub")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.15
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(AcidMoog
    COMPANY_NAME            "Lorenzo"
    PLUGIN_MANUFACTURER_CODE Lrnz
    PLUGIN_CODE             Acmg
    # Standalone richiede juce_audio_devices (e quindi ALSA su Linux): si aggiunge
    # con -DACIDMOOG_BUILD_STANDALONE=ON dopo aver installato libasound2-dev.
    FORMATS                 VST3 AU
    PRODUCT_NAME            "AcidMoog"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(AcidMoog PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Params/ParameterLayout.cpp
    Source/DSP/Oscillator.cpp
    Source/DSP/SamplerSource.cpp
    Source/DSP/LadderFilter.cpp
    Source/DSP/Envelope.cpp
    Source/DSP/GlideProcessor.cpp
    Source/DSP/AccentProcessor.cpp
    Source/DSP/Voice.cpp
    Source/Voicing/MonoVoiceManager.cpp
    Source/UI/LookAndFeel/AcidMoogLookAndFeel.cpp
    Source/UI/Components/KnobComponent.cpp
    Source/UI/Components/ToggleSwitchComponent.cpp
    Source/UI/Components/LedComponent.cpp
    Source/UI/Components/WaveformDisplay.cpp
    Source/UI/Components/FileDropZone.cpp
    Source/UI/Panels/OscillatorPanel.cpp
    Source/UI/Panels/FilterPanel.cpp
    Source/UI/Panels/EnvelopePanel.cpp
    Source/UI/Panels/PerformancePanel.cpp)

target_include_directories(AcidMoog PRIVATE Source)

target_compile_definitions(AcidMoog PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(AcidMoog PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_formats
    juce::juce_gui_extra
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

# ---- Test DSP (console app, non richiede host audio) ----
option(ACIDMOOG_BUILD_TESTS "Build DSP unit tests" ON)

if(ACIDMOOG_BUILD_TESTS)
    juce_add_console_app(AcidMoogTests PRODUCT_NAME "AcidMoogTests")

    target_sources(AcidMoogTests PRIVATE
        Tests/TestMain.cpp
        Source/DSP/Oscillator.cpp
        Source/DSP/SamplerSource.cpp
        Source/DSP/LadderFilter.cpp
        Source/DSP/Envelope.cpp
        Source/DSP/GlideProcessor.cpp
        Source/DSP/AccentProcessor.cpp
        Source/Voicing/MonoVoiceManager.cpp)

    target_include_directories(AcidMoogTests PRIVATE Source)

    target_compile_definitions(AcidMoogTests PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_UNIT_TESTS=1)

    target_link_libraries(AcidMoogTests PRIVATE
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
