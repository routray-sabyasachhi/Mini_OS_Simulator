#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

void runFCFSDisk() {
    ui::printSection("Disk Scheduling - FCFS");
    int n = ui::readInt("Enter number of requests (1-20): ", 1, 20);
    int diskSize = ui::readInt("Enter disk size (tracks, 1-5000): ", 1, 5000);
    int head = ui::readInt("Enter initial head position (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);

    std::vector<int> req(n);
    for (int i = 0; i < n; ++i) {
        req[i] = ui::readInt("Request " + std::to_string(i + 1) + " (0-" + std::to_string(diskSize - 1) + "): ", 0, diskSize - 1);
    }

    int total = 0; // Total head movement.
    std::cout << "\nSequence: " << head;
    for (int i = 0; i < n; ++i) {
        // FCFS services requests in given order.
        total += std::abs(req[i] - head);
        std::cout << " -> " << req[i] << " (move " << std::abs(req[i] - head) << ")";
        head = req[i];
    }
    std::cout << "\nTotal Head Movement: " << total << "\n";
}
