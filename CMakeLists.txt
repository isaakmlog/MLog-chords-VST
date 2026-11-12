cmake_minimum_required(VERSION 3.22)
project(MLOGChordLab VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(MLOGChordLab COMPANY_NAME "MLOG" BUNDLE_ID "com.mlog.chordlab" PLUGIN_MANUFACTURER_CODE Mlog PLUGIN_CODE Mclb FORMATS VST3 PRODUCT_NAME "MLOG Chord Lab" IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE EDITOR_WANTS_KEYBOARD_FOCUS TRUE COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(MLOGChordLab PRIVATE Source/Plugin.cpp)
# PUBLIC so the definitions also reach the MLOGChordLab_VST3 wrapper target.
# JUCE_VST3_CAN_REPLACE_VST2=0: we only ship VST3, and JUCE 8 hits an #error in the VST3 wrapper otherwise.
target_compile_definitions(MLOGChordLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(MLOGChordLab PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_audio_processors juce::juce_gui_extra)
