#include <iostream>
#include <string>
#include <vector>

#include "cpu_scheduling/cpu_utils.h"

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printTitle(const std::string& title);
    void printSection(const std::string& title);
    void waitForEnter();
    std::string color(const std::string& text, const std::string& code);
}

// CPU scheduling functions
ScheduleResult runFCFS();
ScheduleResult runSJFNonPreemptive();
ScheduleResult runSJFPreemptive();
ScheduleResult runPriorityScheduling();
ScheduleResult runRoundRobin();

// Memory management (allocation strategies)
void runFirstFit();
void runBestFit();
void runNextFit();
void runPagingAllocation();

// Page replacement
void runFIFOPageReplacement();
void runLRUPageReplacement();
void runOptimalPageReplacement();

// Disk scheduling
void runFCFSDisk();
void runSSTFDisk();
void runSCANDisk();
void runCSCAN();

static void showScheduleResult(const ScheduleResult& result) {
    printProcessTable(result.processes, result.completion, result.waiting, result.turnaround, result.response);
    printMetrics(result.metrics);
}

static void cpuSchedulingMenu() {
    while (true) {
        ui::printTitle("CPU Scheduling");
        std::cout << "1. FCFS\n";
        std::cout << "2. SJF Non-Preemptive\n";
        std::cout << "3. SJF Preemptive (SRTF)\n";
        std::cout << "4. Priority Scheduling\n";
        std::cout << "5. Round Robin\n";
        std::cout << "6. Back\n";

        int choice = ui::readInt("Select option: ", 1, 6);
        if (choice == 6) return;

        ScheduleResult result;
        switch (choice) {
            case 1: result = runFCFS(); break;
            case 2: result = runSJFNonPreemptive(); break;
            case 3: result = runSJFPreemptive(); break;
            case 4: result = runPriorityScheduling(); break;
            case 5: result = runRoundRobin(); break;
        }

        showScheduleResult(result);
        ui::waitForEnter();
    }
}

static void contiguousMemoryMenu() {
    while (true) {
        ui::printTitle("Memory Management - Contiguous Allocation");
        std::cout << "1. First Fit\n";
        std::cout << "2. Best Fit\n";
        std::cout << "3. Next Fit\n";
        std::cout << "4. Back\n";
        int choice = ui::readInt("Select option: ", 1, 4);
        if (choice == 4) return;
        if (choice == 1) runFirstFit();
        if (choice == 2) runBestFit();
        if (choice == 3) runNextFit();
        ui::waitForEnter();
    }
}

static void nonContiguousMemoryMenu() {
    while (true) {
        ui::printTitle("Memory Management - Non-Contiguous Allocation");
        std::cout << "1. Paging Allocation\n";
        std::cout << "2. Back\n";
        int choice = ui::readInt("Select option: ", 1, 2);
        if (choice == 2) return;
        if (choice == 1) runPagingAllocation();
        ui::waitForEnter();
    }
}

static void memoryManagementMenu() {
    while (true) {
        ui::printTitle("Memory Management");
        std::cout << "1. Contiguous Allocation\n";
        std::cout << "2. Non-Contiguous Allocation\n";
        std::cout << "3. Back\n";
        int choice = ui::readInt("Select option: ", 1, 3);
        if (choice == 3) return;
        if (choice == 1) contiguousMemoryMenu();
        if (choice == 2) nonContiguousMemoryMenu();
    }
}

static void pageReplacementMenu() {
    while (true) {
        ui::printTitle("Page Replacement");
        std::cout << "1. FIFO\n";
        std::cout << "2. LRU\n";
        std::cout << "3. Optimal\n";
        std::cout << "4. Back\n";
        int choice = ui::readInt("Select option: ", 1, 4);
        if (choice == 4) return;
        if (choice == 1) runFIFOPageReplacement();
        if (choice == 2) runLRUPageReplacement();
        if (choice == 3) runOptimalPageReplacement();
        ui::waitForEnter();
    }
}

static void diskSchedulingMenu() {
    while (true) {
        ui::printTitle("Disk Scheduling");
        std::cout << "1. FCFS\n";
        std::cout << "2. SSTF\n";
        std::cout << "3. SCAN\n";
        std::cout << "4. C-SCAN\n";
        std::cout << "5. Back\n";
        int choice = ui::readInt("Select option: ", 1, 5);
        if (choice == 5) return;
        if (choice == 1) runFCFSDisk();
        if (choice == 2) runSSTFDisk();
        if (choice == 3) runSCANDisk();
        if (choice == 4) runCSCAN();
        ui::waitForEnter();
    }
}

int main() {
    while (true) {
        ui::printTitle("Mini Operating System Simulator");
        std::cout << "Developer: SABYASACHHI ROUTRAY\n";
        std::cout << ui::color("Major Modules", "32") << "\n";
        std::cout << "1. CPU Scheduling\n";
        std::cout << "2. Memory Management\n";
        std::cout << "3. Page Replacement\n";
        std::cout << "4. Disk Scheduling\n";
        std::cout << "5. Exit\n";

        int choice = ui::readInt("Select option: ", 1, 5);
        if (choice == 5) {
            std::cout << "\nExiting...\n";
            break;
        }
        if (choice == 1) cpuSchedulingMenu();
        if (choice == 2) memoryManagementMenu();
        if (choice == 3) pageReplacementMenu();
        if (choice == 4) diskSchedulingMenu();
    }
    return 0;
}
