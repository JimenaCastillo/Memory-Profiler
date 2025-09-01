#include "OperatorOverrides.hpp"
#include "MemoryTracker.hpp"
#include "Callsite.hpp"      // para obtener file/line/type
#include "ProfilerAPI.hpp"   // para isSamplingEnabled()

#include <cstdlib>   // std::malloc, std::free
#include <new>       // std::bad_alloc
#include <cstdint>

// Bandera reentrante por hilo (definición)
namespace mp {
thread_local bool in_hook = false;
}

// RAII local para marcar la región del hook
namespace {
struct HookScope {
    bool prev;
    HookScope() : prev(mp::in_hook) { mp::in_hook = true; }
    ~HookScope() { mp::in_hook = prev; }
};
} // namespace

// --- Overrides globales ---
// NOTA: usamos malloc/free para evitar dependencia circular con el runtime de new/delete.
//       Registramos en MemoryTracker sólo cuando NO estamos reentrando.

void* operator new(std::size_t sz) {
    if (mp::in_hook) {
        void* p = std::malloc(sz ? sz : 1);
        if (!p) throw std::bad_alloc();
        return p;
    }

    HookScope scope;
    void* p = std::malloc(sz ? sz : 1);
    if (!p) throw std::bad_alloc();

    // Obtener metadatos desde TLS
    mp::CallsiteInfo cs = mp::currentCallsite();
    const char* file = cs.file ? cs.file : "unknown";
    int         line = cs.line;
    const char* type = cs.type_name ? cs.type_name : "unknown";

    if (mp::api::isSamplingEnabled()) {
        mp::MemoryTracker::instance().onAlloc(
            p, sz, type, file, line, /*is_array=*/false
        );
    }
    return p;
}

void operator delete(void* p) noexcept {
    if (!p) return;

    if (mp::in_hook) {
        std::free(p);
        return;
    }

    HookScope scope;
    if (mp::api::isSamplingEnabled()) {
        mp::MemoryTracker::instance().onFree(p, /*is_array=*/false);
    }
    std::free(p);
}

void* operator new[](std::size_t sz) {
    if (mp::in_hook) {
        void* p = std::malloc(sz ? sz : 1);
        if (!p) throw std::bad_alloc();
        return p;
    }

    HookScope scope;
    void* p = std::malloc(sz ? sz : 1);
    if (!p) throw std::bad_alloc();

    mp::CallsiteInfo cs = mp::currentCallsite();
    const char* file = cs.file ? cs.file : "unknown";
    int         line = cs.line;
    const char* type = cs.type_name ? cs.type_name : "unknown";

    if (mp::api::isSamplingEnabled()) {
        mp::MemoryTracker::instance().onAlloc(
            p, sz, type, file, line, /*is_array=*/true
        );
    }
    return p;
}

void operator delete[](void* p) noexcept {
    if (!p) return;

    if (mp::in_hook) {
        std::free(p);
        return;
    }

    HookScope scope;
    if (mp::api::isSamplingEnabled()) {
        mp::MemoryTracker::instance().onFree(p, /*is_array=*/true);
    }
    std::free(p);
}
