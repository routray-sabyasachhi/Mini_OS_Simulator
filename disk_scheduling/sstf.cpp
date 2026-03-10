#include <cmath>
#include <iostream>
#include <string>
#include <limits>
#include <vector>

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

void runSSTFDisk() {
    ui::printSection("Disk Scheduling - SSTF");
    int n = ui::readInt("Enter number of requests (1-20): ", 1, 20);
    int diskSize = ui::readInt("Enter disk size (tracks, 1-5000): ", 1, 5000);
    int head = ui::readInt("Enter initial head position (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);

    std::vector<int> req(n);
    for (int i = 0; i < n; ++i) {
        req[i] = ui::readInt("Request " + std::to_string(i + 1) + " (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);
    }

    std::vector<bool> done(n, false); // Tracks which requests are already serviced.
    int total = 0;
    std::cout << "\nSequence: " << head;

    for (int completed = 0; completed < n; ++completed) {
        int best = -1;
        int bestDist = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!done[i]) {
                // SSTF picks the nearest request to the current head.
                int dist = std::abs(req[i] - head);
                if (dist < bestDist) {
                    bestDist = dist;
                    best = i;
                }
            }
        }
        done[best] = true;
        total += bestDist;
        std::cout << " -> " << req[best] << " (move " << bestDist << ")";
        head = req[best];
    }

    std::cout << "\nTotal Head Movement: " << total << "\n";
}
