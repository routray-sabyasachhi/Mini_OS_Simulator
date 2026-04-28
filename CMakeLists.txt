cmake_minimum_required(VERSION 3.16)
project(MiniOSSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(MINIOS_CORE_SOURCES
    cpu_scheduling/schedulers_gui.cpp
    memory_management/contiguous/memory_algorithms.cpp
    memory_management/non_contiguous/paging_algorithms.cpp
    page_replacement/page_algorithms.cpp
    disk_scheduling/disk_algorithms.cpp
)

add_library(MiniOSCore STATIC ${MINIOS_CORE_SOURCES})
target_include_directories(MiniOSCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(MiniOSSimulator main_qt.cpp)
target_link_libraries(MiniOSSimulator PRIVATE MiniOSCore Qt6::Widgets)
target_include_directories(MiniOSSimulator PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
