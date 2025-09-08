#pragma once
#include <cstddef>

//
// Macros de captura (por ahora placeholders)
//
#define MP_ALLOC_TYPE(t) #t
#define MP_FILE __FILE__
#define MP_LINE __LINE__

namespace mp {

// Bandera reentrante (un valor por hilo). Definición en OperatorOverrides.cpp
extern thread_local bool in_hook;

// Guard de conveniencia para desactivar el registro dentro de una sección
struct ScopedHookGuard {
    bool prev;
    ScopedHookGuard() : prev(in_hook) { in_hook = true; }
    ~ScopedHookGuard() { in_hook = prev; }
};

} // namespace mp

// Declaraciones de los overrides globales (definidos en OperatorOverrides.cpp)
// Nota: no necesitas incluir nada especial para usarlos; al linkear la lib quedan activos.
