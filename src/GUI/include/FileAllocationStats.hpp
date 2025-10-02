#pragma once
#include <QString>
#include <vector>
#include <unordered_map>

struct FileAllocStats {
    QString file;
    int count = 0;
    std::size_t total_bytes = 0;
};

std::vector<FileAllocStats> computeFileAllocStats();
std::vector<FileAllocStats> computeTopAllocFiles(int topN = 3);