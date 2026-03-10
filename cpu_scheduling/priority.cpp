#include <algorithm>
#include <iostream>
#include <vector>

#include "cpu_utils.h"

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

ScheduleResult runPriorityScheduling() {
    ui::printSection("Priority Scheduling (Non-Preemptive)");
    int n = ui::readInt("Enter number of processes (1-10): ", 1, 10);

    std::vector<Process> procs(n);
    for (int i = 0; i < n; ++i) {
        procs[i].pid = i + 1;
        procs[i].arrival = ui::readInt("Arrival time P" + std::to_string(i + 1) + " (0-50): ", 0, 50);
        procs[i].burst = ui::readInt("Burst time P" + std::to_string(i + 1) + " (1-50): ", 1, 50);
        procs[i].priority = ui::readInt("Priority P" + std::to_string(i + 1) + " (1-10, lower is higher): ", 1, 10);
    }

    std::vector<int> completion(n, 0), waiting(n, 0), turnaround(n, 0), response(n, 0);
    std::vector<GanttEntry> gantt;

    int time = 0;
    int completed = 0;
    std::vector<bool> done(n, false);

    while (completed < n) {
        int chosen = -1;
        int bestPriority = 1e9;
        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival <= time) {
                if (procs[i].priority < bestPriority ||
                    (procs[i].priority == bestPriority && procs[i].arrival < procs[chosen].arrival)) {
                    bestPriority = procs[i].priority;
                    chosen = i;
                }
            }
        }

        if (chosen == -1) {
            int nextArrival = 1e9;
            for (int i = 0; i < n; ++i) {
                if (!done[i]) nextArrival = std::min(nextArrival, procs[i].arrival);
            }
            gantt.push_back({-1, time, nextArrival});
            time = nextArrival;
            continue;
        }

        int start = time;
        response[chosen] = start - procs[chosen].arrival;
        time += procs[chosen].burst;
        completion[chosen] = time;
        turnaround[chosen] = completion[chosen] - procs[chosen].arrival;
        waiting[chosen] = turnaround[chosen] - procs[chosen].burst;
        done[chosen] = true;
        completed++;
        gantt.push_back({procs[chosen].pid, start, time});

        std::cout << "Step: P" << procs[chosen].pid << " runs from " << start << " to " << time << "\n";
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt, computeAverages(waiting, turnaround, response)};
    return result;
}
