cmake_minimum_required(VERSION 3.16)
project(Memory-Profiler)

# Usar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Librería de instrumentación ---
add_library(memory_profiler
        src/Library/core/ProfilerAPI.cpp
        src/Library/core/MemoryTracker.cpp
        src/Library/core/BlockInfo.cpp
        src/Library/core/SocketClient.cpp
        src/Library/core/Serializer.cpp
        tests/UnitTests/test_Leaks.cpp
)

# Headers públicos para usuarios de la librería
target_include_directories(memory_profiler PUBLIC
        src/Library/core
        src/utils
)

# Dependencias comunes
target_sources(memory_profiler PRIVATE
        src/utils/Logger.cpp
        src/utils/Configuration.cpp
        src/utils/Timer.cpp
)

# --- Ejecutable GUI ---
add_executable(memory_profiler_gui
        src/GUI/core/GUI.cpp
        src/GUI/core/Charts.cpp
        src/GUI/core/Views.cpp
        src/GUI/core/SocketServer.cpp
)

# Headers del GUI
target_include_directories(memory_profiler_gui PUBLIC
        src/GUI/core
        src/utils
)

# Enlazar GUI con la librería
target_link_libraries(memory_profiler_gui PRIVATE memory_profiler)

# --- Tests / Ejemplos ---
add_executable(memory_profiler_tests
        tests/UnitTests/test_MemoryTracker.cpp
        tests/UnitTests/test_BlockInfo.cpp
        tests/UnitTests/test_Leaks.cpp
        tests/UnitTests/test_Serializer.cpp
)

target_include_directories(memory_profiler_tests PUBLIC
        src/Library/core
        src/GUI/core
        src/utils
)

target_link_libraries(memory_profiler_tests PRIVATE memory_profiler)
