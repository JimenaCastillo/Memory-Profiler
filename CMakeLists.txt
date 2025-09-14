# Versión mínima de CMake requerida para procesar este archivo
cmake_minimum_required(VERSION 3.16)

project(memory_profiler VERSION 0.1.0 LANGUAGES CXX)

# Configuración del estándar de C++
set(CMAKE_CXX_STANDARD 17)            # Usar C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)   # Obligar a que sea exactamente C++17 o superior
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Generar código position-independent (necesario para librerías compartidas)

# Activar AUTOMOC para que Qt procese automáticamente los headers con Q_OBJECT
set(CMAKE_AUTOMOC ON)

# Si no se especifica tipo de build, usar RelWithDebInfo (optimizado pero con símbolos de depuración)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# ============================================================
# =============== LIBRERÍA DE INSTRUMENTACIÓN ================
# ============================================================

# Buscar recursivamente todos los .cpp dentro de src/Library
file(GLOB_RECURSE MEMORY_PROFILER_SOURCES
        CONFIGURE_DEPENDS
        src/Library/*.cpp)

# Si no se encontró ningún archivo fuente, abortar con error
if(MEMORY_PROFILER_SOURCES STREQUAL "")
    message(FATAL_ERROR "No se encontraron fuentes en src/Library/*.cpp")
endif()

# Crear la librería estática 'memory_profiler' con esos fuentes
add_library(memory_profiler STATIC ${MEMORY_PROFILER_SOURCES}
        src/GUI/include/Charts.hpp
        src/GUI/include/Views.hpp
        src/GUI/include/GUI.hpp)

# Incluir directorios para que otros targets puedan usar los headers de la librería
target_include_directories(memory_profiler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include> # Durante la compilación local
        $<INSTALL_INTERFACE:include>                                       # Si se instala la librería
)

# Declarar que esta librería requiere C++17
target_compile_features(memory_profiler PUBLIC cxx_std_17)

# Opciones de compilación extra para GCC y Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(memory_profiler PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion  # Activar advertencias estrictas
            -fno-omit-frame-pointer                # Mantener puntero de pila (útil para depuración/profiling)
    )
endif()

# Buscar la librería de hilos del sistema
find_package(Threads REQUIRED)

# Enlazar la librería de hilos al target
target_link_libraries(memory_profiler PUBLIC Threads::Threads)

# ============================================================
# ======================= GUI (Qt6) ==========================
# ============================================================

# Buscar Qt6 con los módulos Widgets y Charts
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

# Buscar recursivamente todos los .cpp de la GUI
file(GLOB_RECURSE GUI_SOURCES
        CONFIGURE_DEPENDS
        src/GUI/core/*.cpp)

# Buscar recursivamente todos los headers de la GUI
file(GLOB_RECURSE GUI_HEADERS
        CONFIGURE_DEPENDS
        src/GUI/include/*.h)

# Crear el ejecutable de la GUI con los fuentes y headers
# Nota: incluir los headers aquí ayuda a que AUTOMOC procese los que tengan Q_OBJECT
add_executable(memory_profiler_gui
        ${GUI_SOURCES}
        ${GUI_HEADERS}
        src/GUI/include/Charts.hpp
        src/GUI/include/Views.hpp
        src/GUI/include/GUI.hpp
)

# Incluir directorios para que la GUI encuentre sus headers y los de la librería
target_include_directories(memory_profiler_gui
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include
)

# Enlazar la GUI con:
# - La librería de instrumentación
# - Qt6 Widgets
# - Qt6 Charts
target_link_libraries(memory_profiler_gui
        PRIVATE
        memory_profiler
        Qt6::Widgets
        Qt6::Charts
)