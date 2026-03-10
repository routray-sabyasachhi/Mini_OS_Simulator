#include "disk_algorithms.h"

#include <algorithm>
#include <cmath>
#include <limits>

DiskResult diskFCFS(int, int head, const std::vector<int>& reqs) {
    DiskResult res;
    res.sequence.push_back(head);
    for (int r : reqs) {
        res.totalMovement += std::abs(r - head);
        head = r;
        res.sequence.push_back(head);
    }
    return res;
}

DiskResult diskSSTF(int, int head, const std::vector<int>& reqs) {
    DiskResult res;
    int n = static_cast<int>(reqs.size());
    std::vector<bool> done(n, false);
    res.sequence.push_back(head);
    for (int completed = 0; completed < n; ++completed) {
        int best = -1;
        int bestDist = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!done[i]) {
                int dist = std::abs(reqs[i] - head);
                if (dist < bestDist) {
                    bestDist = dist;
                    best = i;
                }
            }
        }
        done[best] = true;
        res.totalMovement += bestDist;
        head = reqs[best];
        res.sequence.push_back(head);
    }
    return res;
}

DiskResult diskSCAN(int diskSize, int head, const std::vector<int>& reqs, bool dirRight) {
    DiskResult res;
    std::vector<int> sorted = reqs;
    std::sort(sorted.begin(), sorted.end());

    std::vector<int> sequence;
    if (dirRight) {
        for (int r : sorted) if (r >= head) sequence.push_back(r);
        if (head != diskSize - 1) sequence.push_back(diskSize - 1);
        for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; --i) {
            if (sorted[i] < head) sequence.push_back(sorted[i]);
        }
    } else {
        for (int i = static_cast<int>(sorted.size()) - 1; i >= 0; --i) {
            if (sorted[i] <= head) sequence.push_back(sorted[i]);
        }
        if (head != 0) sequence.push_back(0);
        for (int r : sorted) if (r > head) sequence.push_back(r);
    }

    res.sequence.push_back(head);
    for (int r : sequence) {
        res.totalMovement += std::abs(r - head);
        head = r;
        res.sequence.push_back(head);
    }
    return res;
}

DiskResult diskCSCAN(int diskSize, int head, const std::vector<int>& reqs) {
    DiskResult res;
    std::vector<int> sorted = reqs;
    std::sort(sorted.begin(), sorted.end());

    std::vector<int> sequence;
    for (int r : sorted) if (r >= head) sequence.push_back(r);
    if (head != diskSize - 1) sequence.push_back(diskSize - 1);
    sequence.push_back(0);
    for (int r : sorted) if (r < head) sequence.push_back(r);

    res.sequence.push_back(head);
    for (int r : sequence) {
        res.totalMovement += std::abs(r - head);
        head = r;
        res.sequence.push_back(head);
    }
    return res;
}
