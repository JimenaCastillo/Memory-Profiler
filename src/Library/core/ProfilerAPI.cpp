#include "ProfilerAPI.hpp"
#include "MemoryTracker.hpp"
#include "Serializer.hpp"

#include <atomic>
#include <exception>
#include <string>

namespace mp::api {

static std::atomic<bool> g_sampling_enabled{true};

void setSamplingEnabled(bool enabled) noexcept {
    g_sampling_enabled.store(enabled, std::memory_order_release);
}

bool isSamplingEnabled() noexcept {
    return g_sampling_enabled.load(std::memory_order_acquire);
}

ScopedSamplingPause::ScopedSamplingPause(bool pause) noexcept
    : prev_(isSamplingEnabled()) {
    if (pause) setSamplingEnabled(false);
}

ScopedSamplingPause::~ScopedSamplingPause() {
    setSamplingEnabled(prev_);
}

std::string getMetricsJson() noexcept {
    try {
        // Read-only access; MemoryTracker guards its own state.
        return serialize::metricsJson(MemoryTracker::instance());
    } catch (const std::exception&) {
        return std::string("{\"error\":\"getMetricsJson failed\"}");
    } catch (...) {
        return std::string("{\"error\":\"getMetricsJson unknown error\"}");
    }
}

std::string getSnapshotJson() noexcept {
    try {
        auto snap = MemoryTracker::instance().snapshotLive();
        return serialize::snapshotJson(snap);
    } catch (const std::exception&) {
        return std::string("{\"error\":\"getSnapshotJson failed\"}");
    } catch (...) {
        return std::string("{\"error\":\"getSnapshotJson unknown error\"}");
    }
}

} // namespace mp::api
