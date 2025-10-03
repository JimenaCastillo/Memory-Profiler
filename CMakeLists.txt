cmake_minimum_required(VERSION 3.16)

project(memory_profiler
        VERSION 0.1.0
        LANGUAGES CXX)

# Recomendado para diagnósticos de memoria
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)  # -fPIC para librerías

# Activar moc para señales/slots de Qt
set(CMAKE_AUTOMOC ON)

# Tipo de build por defecto si no se especifica
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

# Buscar Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts Gui Core Network)

# Recoger fuentes de la biblioteca principal
file(GLOB_RECURSE MEMORY_PROFILER_SOURCES
        CONFIGURE_DEPENDS
        src/Library/*.cpp)

if(MEMORY_PROFILER_SOURCES STREQUAL "")
  message(FATAL_ERROR "No se encontraron fuentes en src/Library/*.cpp")
endif()

# Crear la biblioteca estática
add_library(memory_profiler STATIC ${MEMORY_PROFILER_SOURCES}
        src/Library/core/Callbacks.cpp
        src/Library/include/Callbacks.hpp
        src/Library/core/CallbacksRegistration.cpp
        src/Library/include/ReentryGuard.hpp
        src/Library/include/CallbacksRegistration.hpp)

# Incluir headers públicos
target_include_directories(memory_profiler
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include>
        $<INSTALL_INTERFACE:include>)

# Activar características modernas de C++
target_compile_features(memory_profiler PUBLIC cxx_std_17)

# Warnings útiles y frame pointers para mejores perfiles
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(memory_profiler PRIVATE
          -Wall -Wextra -Wpedantic -Wconversion
          -fno-omit-frame-pointer)
endif()

# Enlazar con pthread (usado por SocketClient)
find_package(Threads REQUIRED)
target_link_libraries(memory_profiler PUBLIC Threads::Threads)

# Recoger fuentes de la GUI
file(GLOB_RECURSE GUI_SOURCES
        CONFIGURE_DEPENDS
        src/GUI/*.cpp)

file(GLOB_RECURSE GUI_HEADERS
        CONFIGURE_DEPENDS
        src/GUI/*.hpp)

# Crear el ejecutable con la interfaz gráfica
add_executable(memory_profiler_app
        ${GUI_SOURCES}
        src/GUI/core/MainWindow.cpp
        src/GUI/include/MainWindow.hpp
        src/GUI/core/ProfilerController.cpp
        src/GUI/include/ProfilerController.hpp
        src/GUI/core/MemoryChart.cpp
        src/GUI/include/MemoryChart.hpp
        src/GUI/core/main.cpp
        src/GUI/core/MemoryMapView.cpp
        src/GUI/include/MemoryMapView.hpp
        src/GUI/core/FileAllocationStats.cpp
        src/GUI/include/FileAllocationStats.hpp
        src/GUI/core/FileAllocationsTab.cpp
        src/GUI/include/FileAllocationsTab.hpp
        src/GUI/core/MemoryLeaksTab.cpp
        src/GUI/include/MemoryLeaksTab.hpp
        src/GUI/core/LeakStats.cpp
        src/GUI/include/LeakStats.hpp
)

# Incluir headers de la GUI y la biblioteca
target_include_directories(memory_profiler_app
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Library/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI)

# Enlazar el ejecutable con la biblioteca y Qt
target_link_libraries(memory_profiler_app
        memory_profiler
        Qt6::Widgets
        Qt6::Charts
        Qt6::Gui
        Qt6::Core
        Qt6::Network
        Threads::Threads
)

add_executable(demo_client examples/demo.cpp
        examples/demo.cpp)
target_include_directories(demo_client PRIVATE ${CMAKE_SOURCE_DIR}/Library/include)
target_link_libraries(demo_client PRIVATE memory_profiler)

# (Opcional) Instalar la librería y headers
# install(TARGETS memory_profiler EXPORT memory_profilerTargets
#         ARCHIVE DESTINATION lib)
# install(DIRECTORY src/Library/include/ DESTINATION include)