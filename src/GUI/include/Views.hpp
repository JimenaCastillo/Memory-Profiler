#pragma once
#include <cstdint>
#include "SocketServer.hpp"

namespace mp::gui {
void renderInitialMetrics(const Metrics& m);
void renderAllocCounter(std::uint64_t total_allocs);
} // namespace mp::gui
