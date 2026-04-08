#include "paging_algorithms.h"

#include <cmath>

static int ceilDiv(int a, int b) {
    if (b <= 0) return 0;
    return (a + b - 1) / b;
}

PagingResult pagingAllocate(int memorySize, int pageSize, const std::vector<int>& procSize) {
    PagingResult res{};
    res.memorySize = memorySize;
    res.pageSize = pageSize;
    if (pageSize <= 0 || memorySize <= 0) return res;

    res.totalFrames = memorySize / pageSize;
    res.freeFrames = res.totalFrames;
    res.systemRemainder = memorySize % pageSize;

    int numProcs = static_cast<int>(procSize.size());
    res.pagesNeeded.assign(numProcs, 0);
    res.internal.assign(numProcs, 0);
    res.pageTable.assign(numProcs, {});

    std::vector<bool> frameUsed(res.totalFrames, false);

    for (int i = 0; i < numProcs; ++i) {
        int pages = ceilDiv(procSize[i], pageSize);
        res.pagesNeeded[i] = pages;

        if (pages <= 0) {
            res.pageTable[i] = {};
            res.internal[i] = 0;
            continue;
        }

        if (pages > res.freeFrames) {
            res.unallocated++;
            res.pageTable[i].assign(pages, -1);
            res.internal[i] = 0;
            continue;
        }

        std::vector<int> mapping(pages, -1);
        int allocated = 0;
        for (int f = 0; f < res.totalFrames && allocated < pages; ++f) {
            if (!frameUsed[f]) {
                frameUsed[f] = true;
                mapping[allocated] = f;
                allocated++;
            }
        }

        // Since we checked freeFrames, allocated should equal pages.
        res.pageTable[i] = mapping;
        res.freeFrames -= pages;
        res.internal[i] = pages * pageSize - procSize[i];
        res.totalInternal += res.internal[i];
    }

    res.totalInternal += res.systemRemainder;
    return res;
}

