#include "Views.hpp"
#include <iostream>

namespace mp::gui {

void renderInitialMetrics(const Metrics& m) {
    std::cout << "================= Memory Profiler - Initial Metrics =================\n";
    std::cout << "Active Bytes     : " << m.active_bytes  << " B\n";
    std::cout << "Peak Bytes       : " << m.peak_bytes    << " B\n";
    std::cout << "Active Allocs    : " << m.active_allocs << "\n";
    std::cout << "Total Allocs     : " << m.total_allocs  << "\n";
    std::cout << "=====================================================================\n";
}

void renderAllocCounter(std::uint64_t total_allocs) {
    std::cout << "[Alloc Counter] total_allocs = " << total_allocs << "\n";
}

} // namespace mp::gui
