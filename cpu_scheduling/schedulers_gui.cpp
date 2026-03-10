#include "schedulers_gui.h"

#include <algorithm>
#include <queue>

ScheduleResult scheduleFCFS(const std::vector<Process>& procs) {
    int n = static_cast<int>(procs.size());
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
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt,
                          computeAverages(waiting, turnaround, response)};
    return result;
}

ScheduleResult scheduleSJFNonPreemptive(const std::vector<Process>& procs) {
    int n = static_cast<int>(procs.size());
    std::vector<int> completion(n, 0), waiting(n, 0), turnaround(n, 0), response(n, 0);
    std::vector<GanttEntry> gantt;

    int time = 0;
    int completed = 0;
    std::vector<bool> done(n, false);

    while (completed < n) {
        int chosen = -1;
        int minBurst = 1e9;
        for (int i = 0; i < n; ++i) {
            if (!done[i] && procs[i].arrival <= time) {
                if (chosen == -1 || procs[i].burst < minBurst ||
                    (procs[i].burst == minBurst && procs[i].arrival < procs[chosen].arrival)) {
                    minBurst = procs[i].burst;
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
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt,
                          computeAverages(waiting, turnaround, response)};
    return result;
}

ScheduleResult scheduleSJFPreemptive(const std::vector<Process>& procs) {
    int n = static_cast<int>(procs.size());
    std::vector<int> remaining(n), completion(n, 0), waiting(n, 0), turnaround(n, 0), response(n, -1);
    for (int i = 0; i < n; ++i) remaining[i] = procs[i].burst;

    int time = 0;
    int completed = 0;
    std::vector<GanttEntry> gantt;
    int current = -1;
    int segmentStart = 0;

    while (completed < n) {
        int chosen = -1;
        int minRemaining = 1e9;
        for (int i = 0; i < n; ++i) {
            if (procs[i].arrival <= time && remaining[i] > 0) {
                if (chosen == -1 || remaining[i] < minRemaining ||
                    (remaining[i] == minRemaining && procs[i].arrival < procs[chosen].arrival)) {
                    minRemaining = remaining[i];
                    chosen = i;
                }
            }
        }

        if (chosen == -1) {
            if (current != -1) {
                gantt.push_back({procs[current].pid, segmentStart, time});
                current = -1;
            }
            int nextArrival = 1e9;
            for (int i = 0; i < n; ++i) {
                if (remaining[i] > 0) nextArrival = std::min(nextArrival, procs[i].arrival);
            }
            gantt.push_back({-1, time, nextArrival});
            time = nextArrival;
            continue;
        }

        if (current != chosen) {
            if (current != -1) {
                gantt.push_back({procs[current].pid, segmentStart, time});
            }
            current = chosen;
            segmentStart = time;
            if (response[chosen] == -1) response[chosen] = time - procs[chosen].arrival;
        }

        remaining[chosen]--;
        time++;

        if (remaining[chosen] == 0) {
            completion[chosen] = time;
            turnaround[chosen] = completion[chosen] - procs[chosen].arrival;
            waiting[chosen] = turnaround[chosen] - procs[chosen].burst;
            completed++;
            gantt.push_back({procs[chosen].pid, segmentStart, time});
            current = -1;
        }
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt,
                          computeAverages(waiting, turnaround, response)};
    return result;
}

ScheduleResult schedulePriority(const std::vector<Process>& procs) {
    int n = static_cast<int>(procs.size());
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
                if (chosen == -1 || procs[i].priority < bestPriority ||
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
    }

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt,
                          computeAverages(waiting, turnaround, response)};
    return result;
}

ScheduleResult scheduleRoundRobin(const std::vector<Process>& procs, int quantum) {
    int n = static_cast<int>(procs.size());
    if (n == 0) return {};

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

    time = std::max(time, procs[order[0]].arrival);
    pushArrivals();

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

    ScheduleResult result{procs, completion, waiting, turnaround, response, gantt,
                          computeAverages(waiting, turnaround, response)};
    return result;
}
