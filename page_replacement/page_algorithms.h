#ifndef PAGE_ALGORITHMS_H
#define PAGE_ALGORITHMS_H

#include <vector>

struct PageResult {
    std::vector<int> refs;
    std::vector<std::vector<int>> framesSteps;
    std::vector<bool> hits;
    int faults = 0;
};

PageResult pageFIFO(int framesCount, const std::vector<int>& refs);
PageResult pageLRU(int framesCount, const std::vector<int>& refs);
PageResult pageOptimal(int framesCount, const std::vector<int>& refs);

#endif
