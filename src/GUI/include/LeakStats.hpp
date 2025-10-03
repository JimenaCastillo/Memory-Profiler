#pragma once
#include <QString>
#include <vector>
#include <QHash>
#include <cstddef>

struct LeakInfo {
    QString file;
    int line;
    std::size_t size;
    QString type;
    void* address;
    std::uint64_t timestamp_ns;
};

struct LeakSummary {
    std::size_t total_leaked_bytes = 0;
    std::size_t total_leaks = 0;
    std::size_t largest_leak_size = 0;
    QString largest_leak_file;
    QHash<QString, int> leakCountByFile;
    QHash<QString, std::size_t> leakBytesByFile;
    std::vector<LeakInfo> leaks;
};

LeakSummary computeLeakSummary();