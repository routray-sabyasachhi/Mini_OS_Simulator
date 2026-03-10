#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

void runCSCAN() {
    ui::printSection("Disk Scheduling - C-SCAN");
    int n = ui::readInt("Enter number of requests (1-20): ", 1, 20);
    int diskSize = ui::readInt("Enter disk size (tracks, 1-5000): ", 1, 5000);
    int head = ui::readInt("Enter initial head position (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);

    std::vector<int> req(n);
    for (int i = 0; i < n; ++i) {
        req[i] = ui::readInt("Request " + std::to_string(i + 1) + " (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);
    }
    std::sort(req.begin(), req.end());

    std::vector<int> sequence; // Service order for C-SCAN (circular).
    for (int r : req) if (r >= head) sequence.push_back(r);
    if (head != diskSize - 1) sequence.push_back(diskSize - 1);
    sequence.push_back(0); // wrap to start without servicing in between
    for (int r : req) if (r < head) sequence.push_back(r);

    int total = 0;
    std::cout << "\nSequence: " << head;
    for (int r : sequence) {
        total += std::abs(r - head);
        std::cout << " -> " << r << " (move " << std::abs(r - head) << ")";
        head = r;
    }

    std::cout << "\nTotal Head Movement: " << total << "\n";
}
