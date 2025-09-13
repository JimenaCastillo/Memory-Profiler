#include "Callbacks.hpp"
#include <mutex>

namespace mp {

  static Callbacks g_cb;
  static std::once_flag g_init_once;

  static void init_noop() {
    g_cb.onAlloc    = [](void*, std::size_t, const char*){};
    g_cb.onFree     = [](void*){};
    g_cb.bytesInUse = []{ return std::size_t(0); };
    g_cb.peakBytes  = []{ return std::size_t(0); };
    g_cb.allocCount = []{ return std::size_t(0); };
    g_cb.snapshot   = []{ return std::uint64_t(0); };
    g_cb.liveBlocks = []{ return std::vector<BlockInfo>{}; };
  }

  void register_callbacks(const Callbacks& c) {
    std::call_once(g_init_once, init_noop);
    g_cb = c;
    if (!g_cb.onAlloc)    g_cb.onAlloc    = [](void*, std::size_t, const char*){};
    if (!g_cb.onFree)     g_cb.onFree     = [](void*){};
    if (!g_cb.bytesInUse) g_cb.bytesInUse = []{ return std::size_t(0); };
    if (!g_cb.peakBytes)  g_cb.peakBytes  = []{ return std::size_t(0); };
    if (!g_cb.allocCount) g_cb.allocCount = []{ return std::size_t(0); };
    if (!g_cb.snapshot)   g_cb.snapshot   = []{ return std::uint64_t(0); };
    if (!g_cb.liveBlocks) g_cb.liveBlocks = []{ return std::vector<BlockInfo>{}; };
  }

  const Callbacks& get_callbacks() {
    std::call_once(g_init_once, init_noop);
    return g_cb;
  }

} // namespace mp
