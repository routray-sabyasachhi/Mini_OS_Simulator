# Mini Operating System Simulator

MiniOS-Simulator is a C++ project that demonstrates key Operating System algorithms with step-by-step output and computed metrics. It ships in two variants for Windows:
- Console app (`main.cpp`)
- Qt GUI app (`main_qt.cpp`)

Developer: SABYASACHHI ROUTRAY

**Algorithms Included**
- CPU Scheduling: FCFS, SJF (Non-Preemptive), SJF (Preemptive / SRTF), Priority (Non-Preemptive), Round Robin
- Memory Management:
  - Contiguous Allocation: First Fit, Best Fit, Next Fit
  - Non-Contiguous Allocation: Paging Allocation
- Page Replacement: FIFO, LRU, Optimal
- Disk Scheduling: FCFS, SSTF, SCAN, C-SCAN

**Project Layout**
- `main.cpp`: console entry point
- `main_qt.cpp`: Qt GUI entry point
- `utils/input_helper_console.cpp`: console-only UI helpers (no Qt)
- `utils/input_helper_qt.cpp`: Qt UI helpers (Qt Widgets)
- `cpu_scheduling/`, `page_replacement/`, `disk_scheduling/`: algorithm implementations
- `memory_management/contiguous/`: contiguous allocation (First/Best/Next Fit)
- `memory_management/non_contiguous/`: non-contiguous allocation (Paging)
- `utils/`: shared utilities

**Software Requirements (Windows Only)**
- C++17 compiler: MSVC (Visual Studio) or MinGW-w64 (g++)
- CMake 3.16+ (required for the Qt GUI build)
- Qt 6 (Widgets module) for the GUI variant

**Download Links (Official)**
```
Visual Studio Community: https://visualstudio.microsoft.com/vs/community/
MinGW-w64: https://www.mingw-w64.org/
LLVM/Clang: https://llvm.org/
CMake: https://cmake.org/download/
Qt (Qt Online Installer): https://www.qt.io/download
```

**Install Prerequisites (Windows)**
- Install a C++ compiler (Visual Studio or MinGW-w64).
- Install CMake.
- Install Qt 6 (ensure the Qt 6 Widgets component is selected).

**Build And Run (Console App - Windows)**
From the project root:
```
g++ main.cpp cpu_scheduling/*.cpp memory_management/contiguous/*.cpp memory_management/non_contiguous/*.cpp page_replacement/*.cpp disk_scheduling/*.cpp utils/input_helper_console.cpp -o MiniOS
```
Run:
```
.\MiniOS.exe
```
Note: Do not compile `utils/input_helper_qt.cpp` for the console build. It requires Qt headers and will cause `QInputDialog` not found errors.

**Build And Run (Qt GUI App - Windows)**
From the project root:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
Run:
```
.\build\Release\MiniOSSimulator.exe
```

**Build And Run (Qt GUI App - Windows, MinGW 64-bit Qt Kit)**
If you installed the Qt *MinGW 64-bit* kit (e.g. `C:\Qt\6.10.2\mingw_64`) and hit a Qt6 “not compatible (64bit)” CMake error, build using the repo script (it forces the correct MinGW toolchain):
```
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt_mingw64.ps1
```
Optional (pick a specific Qt version folder):
```
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt_mingw64.ps1 -QtVersion 6.10.2
```
If you previously configured a build folder with the wrong compiler, rebuild clean:
```
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt_mingw64.ps1 -QtVersion 6.10.2 -Clean
```

**If CMake Cannot Find Qt**
Point CMake to your Qt installation, then reconfigure:
- Windows example:
```
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.y\msvc2019_64"
```

**Notes**
- The console UI uses ANSI colors. If colors do not appear, enable ANSI color support in your terminal.
- All modules print step-by-step simulation output and computed metrics.
