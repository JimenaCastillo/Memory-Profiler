#pragma once
#include <vector>
#include "SocketServer.hpp"

namespace mp::gui {
void renderActiveBytesChart(const std::vector<Metrics>& series,
                            int width = 60, int height = 12);
} // namespace mp::gui
