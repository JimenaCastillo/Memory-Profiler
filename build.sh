#!/usr/bin/env bash
set -euo pipefail

# Generador por defecto: Ninja (puedes pasar "Unix Makefiles" como primer arg si quieres make)
GENERATOR="${1:-Ninja}"

# Carpeta de build (puedes exportar BUILD_DIR=... antes de llamar el script)
BUILD_DIR="${BUILD_DIR:-build}"

# Tipo de build: Debug, Release, RelWithDebInfo (por defecto), MinSizeRel
BUILD_TYPE="${CMAKE_BUILD_TYPE:-RelWithDebInfo}"

echo "==> Configurando con CMake"
cmake -S . -B "${BUILD_DIR}" -G "${GENERATOR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "==> Compilando objetivo: memory_profiler"
cmake --build "${BUILD_DIR}" --target memory_profiler -j"$(nproc)"

echo "✅ Listo. Archivo generado:"
# Muestra la ruta final (Ninja/Make colocan la .a en ${BUILD_DIR})
find "${BUILD_DIR}" -name "libmemory_profiler.a" -print || true
