cmake_minimum_required(VERSION 3.16)

project(memory_profiler VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# 1) Fuentes de la LIB (solo Library/, NO GUI aquí)
file(GLOB_RECURSE MEMORY_PROFILER_SOURCES CONFIGURE_DEPENDS
     src/Library/*.cpp)

if(MEMORY_PROFILER_SOURCES STREQUAL "")
  message(FATAL_ERROR "No se encontraron fuentes en src/Library/*.cpp")
endif()

add_library(memory_profiler STATIC ${MEMORY_PROFILER_SOURCES})

# 2) INCLUDES públicos de la LIB (para que vea BlockInfo.hpp, ProfilerAPI.hpp, Callbacks.hpp)
target_include_directories(memory_profiler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(memory_profiler PUBLIC cxx_std_17)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(memory_profiler PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion
    -fno-omit-frame-pointer)
endif()

find_package(Threads REQUIRED)
target_link_libraries(memory_profiler PUBLIC Threads::Threads)

# === Opcional: ejemplo CLI para probar rápidamente ===
option(MP_BUILD_EXAMPLES "Build examples" ON)
if (MP_BUILD_EXAMPLES)
  add_executable(mp_demo examples/demo.cpp)
  target_link_libraries(mp_demo PRIVATE memory_profiler)
  target_compile_features(mp_demo PRIVATE cxx_std_17)
endif()

# === Opcional: construir GUI como ejecutable aparte ===
option(MP_BUILD_GUI "Build GUI" OFF)
if (MP_BUILD_GUI)
  add_executable(mp_gui
    src/GUI/core/main_gui.cpp
    src/GUI/core/GUI.cpp
    src/GUI/core/Views.cpp
    src/GUI/core/Charts.cpp
    src/GUI/core/SocketServer.cpp
  )
  target_include_directories(mp_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/include
  )
  target_link_libraries(mp_gui PRIVATE memory_profiler Threads::Threads)
  if (WIN32)
    target_link_libraries(mp_gui PRIVATE ws2_32)
  endif()
endif()

# (Opcional) instalación
# install(TARGETS memory_profiler EXPORT memory_profilerTargets ARCHIVE DESTINATION lib)
# install(DIRECTORY src/Library/include/ DESTINATION include)
