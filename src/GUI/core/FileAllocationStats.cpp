#include "MemoryTracker.hpp"
#include "include/FileAllocationStats.hpp"
#include <map>
#include <algorithm>

std::vector<FileAllocStats> computeFileAllocStats() {
    std::map<QString, FileAllocStats> map;

    auto blocks = mp::MemoryTracker::instance().snapshotLive();
    for (const auto& b : blocks) {
        QString file = b.file ? QString::fromStdString(std::string(b.file)) : QString("?");
        QString key = file + ":" + QString::number(b.line);
        FileAllocStats& stat = map[key];
        stat.file = key;
        stat.count += 1;
        stat.total_bytes += b.size;
    }

    std::vector<FileAllocStats> out;
    out.reserve(map.size());
    for (auto& kv : map) out.push_back(std::move(kv.second));
    return out;
}

std::vector<FileAllocStats> computeTopAllocFiles(int topN) {
    auto stats = computeFileAllocStats();
    std::sort(stats.begin(), stats.end(), [](const auto& a, const auto& b) {
        return a.total_bytes > b.total_bytes;
    });
    if (stats.size() > topN) stats.resize(topN);
    return stats;
}