#ifndef SCHEDULERS_GUI_H
#define SCHEDULERS_GUI_H

#include <vector>

#include "cpu_utils.h"

ScheduleResult scheduleFCFS(const std::vector<Process>& procs);
ScheduleResult scheduleSJFNonPreemptive(const std::vector<Process>& procs);
ScheduleResult scheduleSJFPreemptive(const std::vector<Process>& procs);
ScheduleResult schedulePriority(const std::vector<Process>& procs);
ScheduleResult scheduleRoundRobin(const std::vector<Process>& procs, int quantum);

#endif
