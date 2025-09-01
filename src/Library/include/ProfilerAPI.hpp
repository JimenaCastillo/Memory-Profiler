#pragma once
#include <string>

/**
 * Public API for consumers (apps/GUI) to retrieve memory metrics/snapshots
 * and to control sampling.
 *
 * All functions are noexcept and safe to call from any thread.
 * Internals rely on MemoryTracker's own synchronization.
 */
namespace mp::api {

/**
 * @brief Return current metrics as JSON.
 *
 * Structure (example):
 * {
 *   "active_bytes": <uint64>,
 *   "peak_bytes": <uint64>,
 *   "total_allocs": <uint64>,
 *   "active_allocs": <uint64>
 * }
 *
 * @return std::string JSON payload; returns {"error": "..."} on failure.
 */
std::string getMetricsJson() noexcept;

/**
 * @brief Return a snapshot of current live allocations as JSON.
 *
 * Each record typically contains:
 *   ptr, size, type_name, timestamp_ns, thread_id, file, line, is_array
 *
 * @return std::string JSON payload; returns {"error": "..."} on failure.
 */
std::string getSnapshotJson() noexcept;

/**
 * @brief Enable or disable sampling (registration into the tracker).
 *
 * When disabled, hooks may skip registering onAlloc/onFree (requires the
 * small check in OperatorOverrides — see patch below).
 */
void setSamplingEnabled(bool enabled) noexcept;

/** @brief Returns current sampling state. */
bool isSamplingEnabled() noexcept;

/**
 * @brief RAII helper that pauses sampling for the scope duration.
 *        Restores the previous state on destruction.
 */
class ScopedSamplingPause {
public:
    explicit ScopedSamplingPause(bool pause = true) noexcept;
    ~ScopedSamplingPause();
private:
    bool prev_;
};

} // namespace mp::api
