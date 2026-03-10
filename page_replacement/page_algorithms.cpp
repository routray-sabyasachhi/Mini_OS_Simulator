#include "page_algorithms.h"

#include <algorithm>
#include <queue>

PageResult pageFIFO(int framesCount, const std::vector<int>& refs) {
    PageResult res;
    res.refs = refs;
    std::vector<int> frames(framesCount, -1);
    std::queue<int> q;

    for (size_t i = 0; i < refs.size(); ++i) {
        int page = refs[i];
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        res.hits.push_back(hit);
        if (!hit) {
            res.faults++;
            if (static_cast<int>(q.size()) < framesCount) {
                for (int f = 0; f < framesCount; ++f) {
                    if (frames[f] == -1) {
                        frames[f] = page;
                        q.push(f);
                        break;
                    }
                }
            } else {
                int idx = q.front();
                q.pop();
                frames[idx] = page;
                q.push(idx);
            }
        }
        res.framesSteps.push_back(frames);
    }
    return res;
}

PageResult pageLRU(int framesCount, const std::vector<int>& refs) {
    PageResult res;
    res.refs = refs;
    std::vector<int> frames(framesCount, -1);
    std::vector<int> lastUsed(framesCount, -1);

    for (size_t i = 0; i < refs.size(); ++i) {
        int page = refs[i];
        bool hit = false;
        for (int f = 0; f < framesCount; ++f) {
            if (frames[f] == page) {
                lastUsed[f] = static_cast<int>(i);
                hit = true;
                break;
            }
        }
        res.hits.push_back(hit);
        if (!hit) {
            res.faults++;
            int empty = -1;
            for (int f = 0; f < framesCount; ++f) {
                if (frames[f] == -1) { empty = f; break; }
            }
            if (empty != -1) {
                frames[empty] = page;
                lastUsed[empty] = static_cast<int>(i);
            } else {
                int lruIndex = 0;
                for (int f = 1; f < framesCount; ++f) {
                    if (lastUsed[f] < lastUsed[lruIndex]) lruIndex = f;
                }
                frames[lruIndex] = page;
                lastUsed[lruIndex] = static_cast<int>(i);
            }
        }
        res.framesSteps.push_back(frames);
    }
    return res;
}

PageResult pageOptimal(int framesCount, const std::vector<int>& refs) {
    PageResult res;
    res.refs = refs;
    std::vector<int> frames(framesCount, -1);

    for (size_t i = 0; i < refs.size(); ++i) {
        int page = refs[i];
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        res.hits.push_back(hit);
        if (!hit) {
            res.faults++;
            int empty = -1;
            for (int f = 0; f < framesCount; ++f) {
                if (frames[f] == -1) { empty = f; break; }
            }
            if (empty != -1) {
                frames[empty] = page;
            } else {
                int victim = -1;
                int farthest = -1;
                for (int f = 0; f < framesCount; ++f) {
                    int nextUse = 1e9;
                    for (size_t j = i + 1; j < refs.size(); ++j) {
                        if (refs[j] == frames[f]) { nextUse = static_cast<int>(j); break; }
                    }
                    if (nextUse > farthest) {
                        farthest = nextUse;
                        victim = f;
                    }
                }
                frames[victim] = page;
            }
        }
        res.framesSteps.push_back(frames);
    }
    return res;
}
