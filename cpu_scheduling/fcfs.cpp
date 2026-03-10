#include <algorithm>
#include <iostream>
#include <vector>

#include "cpu_utils.h"

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

ScheduleResult runFCFS() {
    ui::printSection("FCFS Scheduling");
    int n = ui::readInt("Enter number of processes (1-10): ", 1, 10);

    std::vector<Process> procs(n);
    for (int i = 0; i < n; ++i) {
        procs[i].pid = i + 1;
        procs[i].arrival = ui::readInt("Arrival time P" + std::to_string(i + 1) + " (0-50): ", 0, 50);
        procs[i].burst = ui::readInt("Burst time P" + std::to_string(i + 1) + " (1-50): ", 1, 50);
        procs[i].priority = 0;
    }

    std::vector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        if (procs[a].arrival == procs[b].arrival) return procs[a].pid < procs[b].pid;
        return procs[a].arrival < procs[b].arrival;
    });

    std::vector<int> completion(n, 0), waiting(n, 0), turnaround(n, 0), response(n, 0);
    std::vector<GanttEntry> gantt;

    int time = 0;
    for (int k = 0; k < n; ++k) {
        int i = idx[k];
        if (time < procs[i].arrival) {
            gantt.push_back({-1, time, procs[i].arrival});
            time = procs[i].arrival;
        }
        int start = time;
        response[i] = start - procs[i].arrival;
        time += procs[i].burst;
        completion[i] = time;
        turnaround[i] = completion[i] - procs[i].arrival;
        waiting[i] = turnaround[i] - procs[i].burst;
        gantt.push_back({procs[i].pid, start, time});

        std::cout << "Step: P" << procs[i].pid << " runs from " << start << " to " << time << "\n";
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt, computeAverages(waiting, turnaround, response)};
    return result;
}
