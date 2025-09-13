#include "OperatorOverrides.hpp"
#include "ProfilerNew.hpp"
#include "Callbacks.hpp"
#include "Callsite.hpp"


#include <new>
#include <cstdlib>

// 🔁 Flag de reentrancia visible desde otros TU (p.ej. SocketClient)
namespace mp { thread_local bool in_hook = false; }

void* operator new(std::size_t sz) {
  if (sz == 0) sz = 1;
  void* p = std::malloc(sz);
  if (!p) throw std::bad_alloc{};
  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();
    cb.onAlloc(p, sz, nullptr);
    mp::in_hook = false;
  }
  return p;
}

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

void* operator new[](std::size_t sz) {
  if (sz == 0) sz = 1;
  void* p = std::malloc(sz);
  if (!p) throw std::bad_alloc{};
  if (!mp::in_hook) {
    mp::in_hook = true;
    const auto& cb = mp::get_callbacks();
    cb.onAlloc(p, sz, nullptr);
    mp::in_hook = false;
  }
  return p;
}

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

// Sized delete para silenciar warnings
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete[](p); }
