#include "../Library/include/MemoryTracker.hpp"
#include "../include/LeakStats.hpp"

LeakSummary computeLeakSummary() {
    LeakSummary summary;
    auto blocks = mp::MemoryTracker::instance().snapshotLive();

    for (const auto& b : blocks) {
        LeakInfo info;
        info.file = b.file ? QString::fromStdString(std::string(b.file)) : QString("?");
        info.line = b.line;
        info.size = b.size;
        info.type = b.type_name ? QString::fromStdString(std::string(b.type_name)) : QString("unknown");
        info.address = b.ptr;
        info.timestamp_ns = b.timestamp_ns;

        summary.total_leaked_bytes += b.size;
        summary.total_leaks += 1;
        summary.leaks.push_back(info);

        if (b.size > summary.largest_leak_size) {
            summary.largest_leak_size = b.size;
            summary.largest_leak_file = info.file;
        }

        summary.leakCountByFile[info.file] += 1;
        summary.leakBytesByFile[info.file] += b.size;
    }

    return summary;
}
