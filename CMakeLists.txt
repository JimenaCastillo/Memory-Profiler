cmake_minimum_required(VERSION 3.16)

project(memory_profiler
  VERSION 0.1.0
  LANGUAGES CXX)

# Recomendado para diagnósticos de memoria
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # -fPIC para librerías

# Opciones de build por defecto si no se especifica
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# Recoger fuentes de la librería (ajusta la ruta si tus .cpp están en otro lugar)
file(GLOB_RECURSE MEMORY_PROFILER_SOURCES
     CONFIGURE_DEPENDS
     src/Library/*.cpp)

if(MEMORY_PROFILER_SOURCES STREQUAL "")
  message(FATAL_ERROR "No se encontraron fuentes en src/Library/*.cpp")
endif()

add_library(memory_profiler STATIC ${MEMORY_PROFILER_SOURCES})

# Incluye los headers públicos (ajusta si tus headers están en otra carpeta)
target_include_directories(memory_profiler
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include>
    $<INSTALL_INTERFACE:include>)

target_compile_features(memory_profiler PUBLIC cxx_std_17)

# Warnings útiles y frame pointers para mejores perfiles
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(memory_profiler PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion
    -fno-omit-frame-pointer)
endif()

# pthread si lo usas (bloqueos, TLS, etc.)
find_package(Threads REQUIRED)
target_link_libraries(memory_profiler PUBLIC Threads::Threads)

# (Opcional) defines para evitar recursión en hooks u opciones de la lib
# target_compile_definitions(memory_profiler PUBLIC MP_ENABLE_DEMANGLE=1)

# (Opcional) Instalar la librería y headers
# install(TARGETS memory_profiler EXPORT memory_profilerTargets
#         ARCHIVE DESTINATION lib)
# install(DIRECTORY src/Library/include/ DESTINATION include)
