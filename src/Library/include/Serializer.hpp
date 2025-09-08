#pragma once
#include <string>
#include <vector>
#include "MemoryTracker.hpp"

namespace mp::serialize {

// Serializa métricas básicas a un JSON sencillo (sin libs externas)
std::string metricsJson(const MemoryTracker& tracker);

// Serializa un snapshot de bloques vivos a JSON sencillo
std::string snapshotJson(const std::vector<AllocationRecord>& records);

} // namespace mp::serialize
