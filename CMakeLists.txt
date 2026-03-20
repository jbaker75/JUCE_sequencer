cmake_minimum_required(VERSION 3.15)

project(JUCE_sequencer VERSION 0.0.1)

# Force ALSA and JACK flags for the entire project
add_compile_definitions(JUCE_ALSA=1 JUCE_JACK=1 JUCE_USE_CURL=1)

# Add JUCE (assuming it's in lib/JUCE)
add_subdirectory(lib/JUCE)

# Create the application target
juce_add_gui_app(JUCE_sequencer
    PRODUCT_NAME "JUCE Sequencer"
    VERSION "0.0.1"
    COMPANY_NAME "GeminiCLI"
)

# Add source files
target_sources(JUCE_sequencer
    PRIVATE
        Source/Main.cpp
        Source/SequencerEngine.cpp
        Source/SequencerEngine.h
        Source/StepColumn.cpp
        Source/StepColumn.h
)

# Link against JUCE modules
target_link_libraries(JUCE_sequencer
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
)

# Standard strict flags
target_compile_features(JUCE_sequencer PRIVATE cxx_std_17)

# Enable console app for debugging (optional, nice for printf)
juce_generate_juce_header(JUCE_sequencer)

# Manually add GTK, WebKit, CURL, and ALSA includes
if (UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(ALSA REQUIRED alsa)
    
    target_include_directories(JUCE_sequencer PRIVATE 
        ${GTK3_INCLUDE_DIRS} 
        ${WEBKIT_INCLUDE_DIRS} 
        ${CURL_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
    )
    target_link_libraries(JUCE_sequencer PRIVATE 
        ${GTK3_LIBRARIES} 
        ${WEBKIT_LIBRARIES} 
        ${CURL_LIBRARIES}
        ${ALSA_LIBRARIES}
    )
endif()
