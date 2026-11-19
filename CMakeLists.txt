cmake_minimum_required(VERSION 3.22)
project(Aurora106 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.13
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(AURORA_FORMATS VST3 AU Standalone)
else()
    set(AURORA_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Aurora106
    COMPANY_NAME "Independent Audio"
    BUNDLE_ID "audio.independent.aurora106"
    PLUGIN_MANUFACTURER_CODE IAud
    PLUGIN_CODE Au16
    FORMATS ${AURORA_FORMATS}
    PRODUCT_NAME "Aurora 106"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Synth)

juce_generate_juce_header(Aurora106)

target_sources(Aurora106 PRIVATE
    Source/SynthEngine.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(Aurora106 PRIVATE Source)
target_compile_definitions(Aurora106 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Aurora106 PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)

# No JUCE dependency in the DSP smoke test.
option(AURORA_BUILD_DSP_TESTS "Build standalone DSP smoke test" ON)
if(AURORA_BUILD_DSP_TESTS)
    enable_testing()
    add_executable(aurora_dsp_test tests/dsp_test.cpp Source/SynthEngine.cpp)
    target_include_directories(aurora_dsp_test PRIVATE Source)
    add_test(NAME synth_smoke_test COMMAND aurora_dsp_test)
endif()
