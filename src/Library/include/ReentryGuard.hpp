#pragma once
namespace mp { extern thread_local bool in_hook; }

namespace mp {
struct ScopedHookGuard {
    bool prev{};
    ScopedHookGuard() : prev(mp::in_hook) { mp::in_hook = true; }
    ~ScopedHookGuard() { mp::in_hook = prev; }
};
} // namespace mp
