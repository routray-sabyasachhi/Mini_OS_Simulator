#include "memory_algorithms.h"

MemoryResult firstFit(const std::vector<int>& blockSize, const std::vector<int>& procSize) {
    MemoryResult res;
    int numBlocks = static_cast<int>(blockSize.size());
    int numProcs = static_cast<int>(procSize.size());
    res.allocation.assign(numProcs, -1);
    res.internal.assign(numProcs, 0);
    std::vector<bool> used(numBlocks, false);

    for (int i = 0; i < numProcs; ++i) {
        int chosen = -1;
        for (int b = 0; b < numBlocks; ++b) {
            if (!used[b] && blockSize[b] >= procSize[i]) {
                chosen = b;
                break;
            }
        }
        if (chosen != -1) {
            res.allocation[i] = chosen;
            used[chosen] = true;
            res.internal[i] = blockSize[chosen] - procSize[i];
        }
    }

    for (int i = 0; i < numProcs; ++i) {
        if (res.allocation[i] == -1) res.unallocated++;
        res.totalInternal += res.internal[i];
    }
    for (int b = 0; b < numBlocks; ++b) {
        if (!used[b]) res.totalExternal += blockSize[b];
    }
    return res;
}

MemoryResult bestFit(const std::vector<int>& blockSize, const std::vector<int>& procSize) {
    MemoryResult res;
    int numBlocks = static_cast<int>(blockSize.size());
    int numProcs = static_cast<int>(procSize.size());
    res.allocation.assign(numProcs, -1);
    res.internal.assign(numProcs, 0);
    std::vector<bool> used(numBlocks, false);

    for (int i = 0; i < numProcs; ++i) {
        int chosen = -1;
        int bestSize = 1e9;
        for (int b = 0; b < numBlocks; ++b) {
            if (!used[b] && blockSize[b] >= procSize[i] && blockSize[b] < bestSize) {
                bestSize = blockSize[b];
                chosen = b;
            }
        }
        if (chosen != -1) {
            res.allocation[i] = chosen;
            used[chosen] = true;
            res.internal[i] = blockSize[chosen] - procSize[i];
        }
    }

    for (int i = 0; i < numProcs; ++i) {
        if (res.allocation[i] == -1) res.unallocated++;
        res.totalInternal += res.internal[i];
    }
    for (int b = 0; b < numBlocks; ++b) {
        if (!used[b]) res.totalExternal += blockSize[b];
    }
    return res;
}

MemoryResult nextFit(const std::vector<int>& blockSize, const std::vector<int>& procSize) {
    MemoryResult res;
    int numBlocks = static_cast<int>(blockSize.size());
    int numProcs = static_cast<int>(procSize.size());
    res.allocation.assign(numProcs, -1);
    res.internal.assign(numProcs, 0);
    std::vector<bool> used(numBlocks, false);

    int nextIndex = 0;
    for (int i = 0; i < numProcs; ++i) {
        int chosen = -1;
        int count = 0;
        while (count < numBlocks) {
            if (!used[nextIndex] && blockSize[nextIndex] >= procSize[i]) {
                chosen = nextIndex;
                break;
            }
            nextIndex = (nextIndex + 1) % numBlocks;
            count++;
        }
        if (chosen != -1) {
            res.allocation[i] = chosen;
            used[chosen] = true;
            res.internal[i] = blockSize[chosen] - procSize[i];
            nextIndex = (chosen + 1) % numBlocks;
        }
    }

    for (int i = 0; i < numProcs; ++i) {
        if (res.allocation[i] == -1) res.unallocated++;
        res.totalInternal += res.internal[i];
    }
    for (int b = 0; b < numBlocks; ++b) {
        if (!used[b]) res.totalExternal += blockSize[b];
    }
    return res;
}
