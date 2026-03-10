cmake_minimum_required(VERSION 3.16)
project(MiniOSSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(MiniOSSimulator
    main_qt.cpp
    utils/input_helper_qt.cpp
    cpu_scheduling/fcfs.cpp
    cpu_scheduling/sjf.cpp
    cpu_scheduling/priority.cpp
    cpu_scheduling/round_robin.cpp
    cpu_scheduling/schedulers_gui.cpp
    memory_management/first_fit.cpp
    memory_management/best_fit.cpp
    memory_management/next_fit.cpp
    memory_management/memory_algorithms.cpp
    page_replacement/fifo.cpp
    page_replacement/lru.cpp
    page_replacement/optimal.cpp
    page_replacement/page_algorithms.cpp
    disk_scheduling/fcfs_disk.cpp
    disk_scheduling/sstf.cpp
    disk_scheduling/scan.cpp
    disk_scheduling/cscan.cpp
    disk_scheduling/disk_algorithms.cpp
)

target_link_libraries(MiniOSSimulator PRIVATE Qt6::Widgets)
