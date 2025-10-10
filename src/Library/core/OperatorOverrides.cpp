#include "../include/OperatorOverrides.hpp"
#include "../include/ProfilerNew.hpp"
#include "../include/Callbacks.hpp"
#include "../include/Callsite.hpp"

#include <new>
#include <cstdlib>

// Flag de reentrancia visible desde otros TU (Translation Units)
namespace mp { thread_local bool in_hook = false; }

// === Sobrecarga del operador new ===
void* operator new(std::size_t sz) {
  if (sz == 0) sz = 1;
  void* p = std::malloc(sz);
  if (!p) throw std::bad_alloc{};

  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();

    // IMPORTANTE: Capturar callsite ANTES de cualquier operación
    auto cs = mp::currentCallsite();

    // Notificar asignación
    cb.onAlloc(p, sz, cs.type_name, cs.file, cs.line, false);

    // Limpiar callsite DESPUÉS de notificar
    mp::clearCallsite();

    mp::in_hook = false;
  }
  return p;
}

// === Sobrecarga del operador delete ===
void operator delete(void* p) noexcept {
  if (!p) return;
  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();
    cb.onFree(p);
    mp::in_hook = false;
  }
  std::free(p);
}

// === Sobrecarga del operador new[] ===
void* operator new[](std::size_t sz) {
  if (sz == 0) sz = 1;
  void* p = std::malloc(sz);
  if (!p) throw std::bad_alloc{};

  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();

    // IMPORTANTE: Capturar callsite ANTES de cualquier operación
    auto cs = mp::currentCallsite();

    // Notificar asignación de arreglo
    cb.onAlloc(p, sz, cs.type_name, cs.file, cs.line, true);

    // Limpiar callsite DESPUÉS de notificar
    mp::clearCallsite();

    mp::in_hook = false;
  }
  return p;
}

// === Sobrecarga del operador delete[] ===
void operator delete[](void* p) noexcept {
  if (!p) return;
  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();
    cb.onFree(p);
    mp::in_hook = false;
  }
  std::free(p);
}

// === Sobrecargas de delete con tamaño ===
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete[](p); }