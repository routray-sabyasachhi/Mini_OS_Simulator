#ifndef MEMORY_ALGORITHMS_H
#define MEMORY_ALGORITHMS_H

#include <vector>

struct MemoryResult {
    std::vector<int> allocation;
    std::vector<int> internal;
    int totalInternal = 0;
    int totalExternal = 0;
    int unallocated = 0;
};

MemoryResult firstFit(const std::vector<int>& blockSize, const std::vector<int>& procSize);
MemoryResult bestFit(const std::vector<int>& blockSize, const std::vector<int>& procSize);
MemoryResult nextFit(const std::vector<int>& blockSize, const std::vector<int>& procSize);

#endif
