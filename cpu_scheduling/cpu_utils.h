#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

struct Process {
    int pid;
    int arrival;
    int burst;
    int priority;
};

struct GanttEntry {
    int pid; // -1 for idle
    int start;
    int end;
};

struct ScheduleMetrics {
    double avg_waiting;
    double avg_turnaround;
    double avg_response;
};

struct ScheduleResult {
    std::vector<Process> processes;
    std::vector<int> completion;
    std::vector<int> waiting;
    std::vector<int> turnaround;
    std::vector<int> response;
    std::vector<GanttEntry> gantt;
    ScheduleMetrics metrics;
};


inline int sumVector(const std::vector<int>& v) {
    return std::accumulate(v.begin(), v.end(), 0);
}

inline ScheduleMetrics computeAverages(const std::vector<int>& waiting,
                                      const std::vector<int>& turnaround,
                                      const std::vector<int>& response) {
    ScheduleMetrics m{};
    if (!waiting.empty()) {
        m.avg_waiting = static_cast<double>(sumVector(waiting)) / waiting.size();
        m.avg_turnaround = static_cast<double>(sumVector(turnaround)) / turnaround.size();
        m.avg_response = static_cast<double>(sumVector(response)) / response.size();
    }
    return m;
}

inline void printProcessTable(const std::vector<Process>& procs,
                              const std::vector<int>& completion,
                              const std::vector<int>& waiting,
                              const std::vector<int>& turnaround,
                              const std::vector<int>& response) {
    std::cout << "\n";
    std::cout << std::left << std::setw(6) << "PID"
              << std::setw(10) << "ARR"
              << std::setw(10) << "BURST"
              << std::setw(10) << "PRIO"
              << std::setw(12) << "COMP"
              << std::setw(12) << "WAIT"
              << std::setw(12) << "TAT"
              << std::setw(12) << "RESP" << "\n";
    std::cout << std::string(84, '-') << "\n";
    for (size_t i = 0; i < procs.size(); ++i) {
        std::cout << std::left << std::setw(6) << procs[i].pid
                  << std::setw(10) << procs[i].arrival
                  << std::setw(10) << procs[i].burst
                  << std::setw(10) << procs[i].priority
                  << std::setw(12) << completion[i]
                  << std::setw(12) << waiting[i]
                  << std::setw(12) << turnaround[i]
                  << std::setw(12) << response[i] << "\n";
    }
}

inline void printMetrics(const ScheduleMetrics& m) {
    std::cout << "\nAverages: "
              << "Waiting=" << std::fixed << std::setprecision(2) << m.avg_waiting
              << ", Turnaround=" << m.avg_turnaround
              << ", Response=" << m.avg_response << "\n";
}

#endif
