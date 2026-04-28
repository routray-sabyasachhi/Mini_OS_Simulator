# MiniOS Simulator

## Description

MiniOS Simulator is a Qt-based desktop application built to demonstrate important operating system concepts through an interactive graphical interface. It combines multiple OS topics in one project so users can enter data, run algorithms, and observe how the system behaves under different conditions.

The project is designed for learning, demonstration, and experimentation. Instead of showing only final output, the simulator presents visual and animated results so users can better understand how each algorithm works step by step.

## Features

- interactive GUI built with Qt
- separate modules for major operating system topics
- user-editable input data for simulations
- animated execution flow in result views
- global OS runtime timer in the application header
- light mode and dark mode support
- default values provided in every module for quick testing

## Modules

- `CPU Scheduling`
  Simulates FCFS, SJF Non-Preemptive, SJF Preemptive (SRTF), Priority, and Round Robin. It shows process metrics and an animated Gantt chart.

- `Comparison Dashboard`
  Allows the user to enter custom process data and compare multiple CPU scheduling algorithms on the same workload.

- `Memory Management`
  Demonstrates contiguous memory allocation methods such as First Fit, Best Fit, and Next Fit, along with paging-based non-contiguous allocation.

- `Page Replacement`
  Simulates FIFO, LRU, and Optimal page replacement using a user-defined page reference string.

- `Disk Scheduling`
  Demonstrates FCFS, SSTF, SCAN, and C-SCAN disk head scheduling algorithms with animated output.

## Explanation

This project helps users understand how operating system algorithms behave in practice. Each module accepts input from the user, processes that input using the selected algorithm, and displays the result in a more visual form. The comparison and animation features make it easier to study differences between algorithms instead of only reading numerical output.

By combining CPU scheduling, memory management, page replacement, and disk scheduling in one simulator, the project gives a broader picture of how different operating system components work together.

## Technologies Used

- `C++`
- `Qt 6 Widgets`
- `CMake`
- `MinGW 64-bit toolchain`

## How to Compile

Run this from the project root:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt_mingw64.ps1
```

## How to Run

After the build finishes, run:

```powershell
build_qt_mingw64\bin\MiniOSSimulator.exe
```
