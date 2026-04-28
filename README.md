# MiniOS Simulator

## How to Compile

Run this from the project root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt_mingw64.ps1
```

## How to Run

After the build finishes, run:

```powershell
.\build_qt_mingw64\bin\MiniOSSimulator.exe
```

## Description

MiniOS Simulator is a Qt-based desktop application that demonstrates core operating system concepts through an interactive graphical interface. The project brings together multiple OS topics in one place so users can enter data, run algorithms, and observe how system behavior changes under different scheduling and memory conditions.

The simulator is designed as a learning and demonstration tool. Instead of showing only final answers, the GUI presents animated step-by-step output so users can follow how each algorithm works over time.

## Explanation

This project is organized into several operating system modules:

- `CPU Scheduling`: Simulates algorithms such as FCFS, SJF, SRTF, Priority, and Round Robin. It shows process timing, averages, and an animated Gantt chart.
- `Comparison Dashboard`: Lets the user enter process data and compare multiple CPU scheduling algorithms on the same workload.
- `Memory Management`: Demonstrates contiguous allocation methods like First Fit, Best Fit, and Next Fit, along with paging-based non-contiguous allocation.
- `Page Replacement`: Simulates FIFO, LRU, and Optimal page replacement using a user-provided reference string.
- `Disk Scheduling`: Demonstrates FCFS, SSTF, SCAN, and C-SCAN for disk head movement requests.

The GUI includes:

- customizable user input tables and controls for each module
- animated execution playback so results appear step by step
- a global OS runtime timer at the top of the application
- light mode and dark mode for easier viewing

Overall, the project helps users understand how major operating system algorithms work by combining theory, input-driven experimentation, and visual output in a single simulator.
