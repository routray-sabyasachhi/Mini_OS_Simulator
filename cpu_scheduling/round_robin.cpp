#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>

#include "cpu_utils.h"

namespace ui {
    int readInt(const std::string& prompt, int minValue, int maxValue);
    void printSection(const std::string& title);
}

ScheduleResult runRoundRobin() {
    ui::printSection("Round Robin Scheduling");
    int n = ui::readInt("Enter number of processes (1-10): ", 1, 10);
    int quantum = ui::readInt("Enter time quantum (1-10): ", 1, 10);

    std::vector<Process> procs(n);
    for (int i = 0; i < n; ++i) {
        procs[i].pid = i + 1;
        procs[i].arrival = ui::readInt("Arrival time P" + std::to_string(i + 1) + " (0-50): ", 0, 50);
        procs[i].burst = ui::readInt("Burst time P" + std::to_string(i + 1) + " (1-50): ", 1, 50);
        procs[i].priority = 0;
    }

    std::vector<int> remaining(n), completion(n, 0), waiting(n, 0), turnaround(n, 0), response(n, -1);
    for (int i = 0; i < n; ++i) remaining[i] = procs[i].burst;

    std::vector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        if (procs[a].arrival == procs[b].arrival) return procs[a].pid < procs[b].pid;
        return procs[a].arrival < procs[b].arrival;
    });

    std::queue<int> q;
    int time = 0;
    int nextIndex = 0;
    int completed = 0;
    std::vector<GanttEntry> gantt;

    auto pushArrivals = [&]() {
        while (nextIndex < n && procs[order[nextIndex]].arrival <= time) {
            q.push(order[nextIndex]);
            nextIndex++;
        }
    };

    if (order.size() > 0) {
        time = std::max(time, procs[order[0]].arrival);
        pushArrivals();
    }

    while (completed < n) {
        if (q.empty()) {
            int nextArrival = procs[order[nextIndex]].arrival;
            gantt.push_back({-1, time, nextArrival});
            time = nextArrival;
            pushArrivals();
            continue;
        }

        int i = q.front();
        q.pop();
        if (response[i] == -1) response[i] = time - procs[i].arrival;

        int run = std::min(quantum, remaining[i]);
        int start = time;
        time += run;
        remaining[i] -= run;
        gantt.push_back({procs[i].pid, start, time});

        std::cout << "Step: P" << procs[i].pid << " runs from " << start << " to " << time
                  << " (remaining " << remaining[i] << ")\n";

        pushArrivals();
        if (remaining[i] > 0) {
            q.push(i);
        } else {
            completion[i] = time;
            turnaround[i] = completion[i] - procs[i].arrival;
            waiting[i] = turnaround[i] - procs[i].burst;
            completed++;
        }
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt, computeAverages(waiting, turnaround, response)};
    return result;
}
