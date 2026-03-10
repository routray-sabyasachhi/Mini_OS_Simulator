#ifndef DISK_ALGORITHMS_H
#define DISK_ALGORITHMS_H

#include <vector>

struct DiskResult {
    std::vector<int> sequence;
    int totalMovement = 0;
};

DiskResult diskFCFS(int diskSize, int head, const std::vector<int>& reqs);
DiskResult diskSSTF(int diskSize, int head, const std::vector<int>& reqs);
DiskResult diskSCAN(int diskSize, int head, const std::vector<int>& reqs, bool dirRight);
DiskResult diskCSCAN(int diskSize, int head, const std::vector<int>& reqs);

#endif
