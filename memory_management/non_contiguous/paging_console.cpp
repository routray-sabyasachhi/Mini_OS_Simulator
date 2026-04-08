#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "paging_algorithms.h"

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

static void printPagingSummary(const std::vector<int>& procSize, const PagingResult& res) {
    std::cout << "\n";
    std::cout << std::left << std::setw(8) << "PID"
              << std::setw(12) << "ProcSize"
              << std::setw(12) << "Pages"
              << std::setw(12) << "Allocated"
              << std::setw(12) << "IntFrag" << "\n";
    std::cout << std::string(56, '-') << "\n";

    for (size_t i = 0; i < procSize.size(); ++i) {
        bool allocated = true;
        for (int f : res.pageTable[i]) {
            if (f == -1) {
                allocated = false;
                break;
            }
        }

        std::cout << std::left << std::setw(8) << (i + 1)
                  << std::setw(12) << procSize[i]
                  << std::setw(12) << res.pagesNeeded[i];

        if (!allocated) {
            std::cout << std::setw(12) << "NO"
                      << std::setw(12) << "-";
        } else {
            std::cout << std::setw(12) << "YES"
                      << std::setw(12) << res.internal[i];
        }
        std::cout << "\n";
    }
}

static void printPageTables(const PagingResult& res) {
    std::cout << "\nPage Tables (page -> frame):\n";
    for (size_t i = 0; i < res.pageTable.size(); ++i) {
        bool allocated = true;
        for (int f : res.pageTable[i]) {
            if (f == -1) {
                allocated = false;
                break;
            }
        }
        std::cout << "P" << (i + 1) << ": ";
        if (!allocated) {
            std::cout << "NOT ALLOCATED\n";
            continue;
        }
        for (size_t p = 0; p < res.pageTable[i].size(); ++p) {
            if (p) std::cout << ", ";
            std::cout << p << "->" << res.pageTable[i][p];
        }
        std::cout << "\n";
    }
}

void runPagingAllocation() {
    ui::printSection("Memory Management - Non-Contiguous (Paging)");

    int memorySize = ui::readInt("Enter total physical memory size (1-8192): ", 1, 8192);
    int pageSize = ui::readInt("Enter page/frame size (1-1024): ", 1, 1024);
    while (pageSize > memorySize) {
        std::cout << "Page size must be <= memory size.\n";
        pageSize = ui::readInt("Enter page/frame size (1-1024): ", 1, 1024);
    }

    int numProcs = ui::readInt("Enter number of processes (1-20): ", 1, 20);
    std::vector<int> procSize(numProcs);
    for (int i = 0; i < numProcs; ++i) {
        procSize[i] = ui::readInt("Process " + std::to_string(i + 1) + " size (1-8192): ", 1, 8192);
    }

    PagingResult res = pagingAllocate(memorySize, pageSize, procSize);

    std::cout << "\nTotal Frames: " << res.totalFrames << "\n";
    std::cout << "Free Frames: " << res.freeFrames << "\n";
    std::cout << "System Remainder (wasted): " << res.systemRemainder << "\n";

    printPagingSummary(procSize, res);
    printPageTables(res);

    std::cout << "\nTotal Internal Fragmentation (incl. remainder): " << res.totalInternal << "\n";
    std::cout << "Unallocated Processes: " << res.unallocated << "\n";
}

