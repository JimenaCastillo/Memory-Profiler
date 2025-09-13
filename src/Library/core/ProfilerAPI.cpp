#include "ProfilerAPI.hpp"
#include "Callbacks.hpp"
#include "Serializer.hpp"
#include <atomic>

namespace { std::atomic<bool> g_enabled{true}; }

namespace mp {

  void start()      { g_enabled.store(true,  std::memory_order_relaxed); }
  void stop()       { g_enabled.store(false, std::memory_order_relaxed); }
  bool is_enabled() { return g_enabled.load(std::memory_order_relaxed); }

  SnapshotId snapshot() {
    const auto& cb = get_callbacks();
    return cb.snapshot();
  }

  std::string summary_json() {
    const auto& cb = get_callbacks();
    return make_summary_json(cb.bytesInUse(), cb.peakBytes(), cb.allocCount());
  }

  std::string live_allocs_csv() {
    const auto& cb = get_callbacks();
    return make_live_allocs_csv(cb.liveBlocks());
  }

  std::string summary_message_json() {
    const auto& cb = get_callbacks();
    auto payload = make_summary_json(cb.bytesInUse(), cb.peakBytes(), cb.allocCount());
    return make_message_json("SUMMARY", payload);
  }

  std::string live_allocs_message_json() {
    const auto& cb = get_callbacks();
    auto payload = make_live_allocs_json(cb.liveBlocks());
    return make_message_json("LIVE_ALLOCS", payload);
  }

  ScopedSection::ScopedSection(const char* /*name*/) {}
  ScopedSection::~ScopedSection() {}

  // ---- Wrappers de compatibilidad (demo/SocketClient) ----
  namespace api {
    std::string getMetricsJson()  { return summary_message_json(); }
    std::string getSnapshotJson() { return live_allocs_message_json(); }
  }

} // namespace mp
