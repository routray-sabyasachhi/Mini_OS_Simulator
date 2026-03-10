#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

static void printAllocationTable(const std::vector<int>& procSize,
                                 const std::vector<int>& blockSize,
                                 const std::vector<int>& allocation,
                                 const std::vector<int>& internal) {
    std::cout << "\n";
    std::cout << std::left << std::setw(8) << "PID"
              << std::setw(12) << "ProcSize"
              << std::setw(10) << "Block"
              << std::setw(12) << "BlockSize"
              << std::setw(12) << "IntFrag" << "\n";
    std::cout << std::string(54, '-') << "\n";
    for (size_t i = 0; i < procSize.size(); ++i) {
        std::cout << std::left << std::setw(8) << (i + 1)
                  << std::setw(12) << procSize[i];
        if (allocation[i] == -1) {
            std::cout << std::setw(10) << "-"
                      << std::setw(12) << "-"
                      << std::setw(12) << "-";
        } else {
            std::cout << std::setw(10) << (allocation[i] + 1)
                      << std::setw(12) << blockSize[allocation[i]]
                      << std::setw(12) << internal[i];
        }
        std::cout << "\n";
    }
}

void runFirstFit() {
    ui::printSection("Memory Allocation - First Fit");
    int numBlocks = ui::readInt("Enter number of memory blocks (1-20): ", 1, 20);
    int numProcs = ui::readInt("Enter number of processes (1-20): ", 1, 20);

    std::vector<int> blockSize(numBlocks);
    for (int i = 0; i < numBlocks; ++i) {
        blockSize[i] = ui::readInt("Block " + std::to_string(i + 1) + " size (1-1024): ", 1, 1024);
    }

    std::vector<int> procSize(numProcs);
    for (int i = 0; i < numProcs; ++i) {
        procSize[i] = ui::readInt("Process " + std::to_string(i + 1) + " size (1-1024): ", 1, 1024);
    }

    // allocation[i] holds the chosen block index for process i, or -1 if not allocated.
    std::vector<int> allocation(numProcs, -1);
    std::vector<int> internal(numProcs, 0);
    std::vector<bool> used(numBlocks, false);

    for (int i = 0; i < numProcs; ++i) {
        int chosen = -1;
        for (int b = 0; b < numBlocks; ++b) {
            // First Fit picks the first block that can hold the process.
            if (!used[b] && blockSize[b] >= procSize[i]) {
                chosen = b;
                break;
            }
        }

        if (chosen != -1) {
            allocation[i] = chosen;
            used[chosen] = true;
            // Internal fragmentation is the unused space inside the allocated block.
            internal[i] = blockSize[chosen] - procSize[i];
        }
    }

    printAllocationTable(procSize, blockSize, allocation, internal);

    int totalInternal = 0;
    int totalExternal = 0;
    int unallocated = 0;
    for (int i = 0; i < numProcs; ++i) {
        if (allocation[i] == -1) unallocated++;
        totalInternal += internal[i];
    }
    for (int b = 0; b < numBlocks; ++b) {
        // External fragmentation is the sum of free blocks left unused.
        if (!used[b]) totalExternal += blockSize[b];
    }

    std::cout << "\nTotal Internal Fragmentation: " << totalInternal << "\n";
    std::cout << "Total External Fragmentation: " << totalExternal << "\n";
    std::cout << "Unallocated Processes: " << unallocated << "\n";
}
