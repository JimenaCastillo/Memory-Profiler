#include "Charts.hpp"
#include <string>
#include <iostream>
#include <algorithm>

namespace mp::gui {

void renderActiveBytesChart(const std::vector<Metrics>& series, int width, int height) {
    if (series.empty()) {
        std::cout << "[Chart] (no data)\n";
        return;
    }

    int n = static_cast<int>(series.size());
    int from = std::max(0, n - width);

    std::uint64_t maxy = 1;
    for (int i = from; i < n; ++i) maxy = std::max(maxy, series[i].active_bytes);
    if (maxy == 0) maxy = 1;

    std::vector<std::string> canvas(height, std::string(static_cast<size_t>(n - from), ' '));
    for (int i = from; i < n; ++i) {
        std::uint64_t v = series[i].active_bytes;
        int h = static_cast<int>((static_cast<long double>(v) * height) / maxy);
        if (h <= 0) h = 1;
        if (h > height) h = height;

        size_t x = static_cast<size_t>(i - from);
        for (int k = 0; k < h; ++k) {
            canvas[height - 1 - k][x] = '*';
        }
    }

    std::cout << "Active Bytes (last " << (n - from) << " samples), max=" << maxy << "B\n";
    for (auto& row : canvas) std::cout << row << "\n";
}

} // namespace mp::gui
