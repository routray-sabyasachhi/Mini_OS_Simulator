#pragma once

#include <vector>

struct PagingResult {
    int memorySize = 0;     // in same unit as pageSize/proc sizes
    int pageSize = 0;       // page/frame size
    int totalFrames = 0;    // memorySize / pageSize
    int freeFrames = 0;
    int systemRemainder = 0; // memorySize % pageSize (wasted in fixed-frame system)

    // Per-process
    std::vector<int> pagesNeeded;                 // ceil(procSize/pageSize)
    std::vector<int> internal;                    // pagesNeeded*pageSize - procSize (0 if unallocated)
    std::vector<std::vector<int>> pageTable;      // pageTable[pid][page] = frame index, or -1

    int totalInternal = 0; // includes systemRemainder
    int unallocated = 0;
};

// Non-contiguous memory allocation using paging (frames are fixed-size and can be scattered).
// - memorySize: total physical memory size
// - pageSize: page/frame size
// - procSize: process sizes
PagingResult pagingAllocate(int memorySize, int pageSize, const std::vector<int>& procSize);

