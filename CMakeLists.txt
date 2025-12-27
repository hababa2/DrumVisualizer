cmake_minimum_required(VERSION 3.15)
project(DrumVisualizer VERSION 1.0.0)

# This project currently only supports Windows
if(NOT WIN32)
    message(FATAL_ERROR "This project currently only supports Windows")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Explicit source list
set(SOURCES
    src/Buffer.cpp
    src/Main.cpp
    src/Renderer.cpp
    src/Visualizer.cpp
    src/Window.cpp
)

set(LIB_SOURCES
    lib/include/glad/glad.c
    lib/include/rtmidi/RtMidi.cpp
)

add_executable(DrumVisualizer ${SOURCES} ${LIB_SOURCES})

target_include_directories(DrumVisualizer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/lib/include
)

# Validate library exists
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lib/glfw3.lib")
    message(FATAL_ERROR "GLFW3 library not found at: ${CMAKE_SOURCE_DIR}/lib/glfw3.lib")
endif()

target_link_libraries(DrumVisualizer PRIVATE
    "${CMAKE_SOURCE_DIR}/lib/glfw3.lib"
    winmm
    windowsapp
)

target_compile_definitions(DrumVisualizer PRIVATE
    WIN32
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
    $<$<CONFIG:Debug>:_DEBUG;_CONSOLE>
    $<$<CONFIG:Release>:NDEBUG>
    UNICODE
    _UNICODE
)

# MSVC compiler flags for feature parity with Visual Studio project
if(MSVC)
    target_compile_options(DrumVisualizer PRIVATE
        /W3           # Warning level 3
        /sdl          # Security Development Lifecycle checks
        /permissive-  # Conformance mode
        $<$<CONFIG:Release>:/O2 /Oi /GL /Gy>
    )
    target_link_options(DrumVisualizer PRIVATE
        $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
    )

    # Hide console for Release builds
    set_target_properties(DrumVisualizer PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()
