# Toolchain helper for building MiniOSSimulator with Qt6 MinGW 64-bit on Windows.
#
# Usage (PowerShell):
#   cmake -S . -B build_qt_mingw64 -G "MinGW Makefiles" `
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/qt6-mingw64.cmake `
#     -DQT6_MINGW64_ROOT="C:\\Qt\\6.10.2\\mingw_64" `
#     -DQT6_TOOLS_MINGW64_BIN="C:\\Qt\\Tools\\mingw1310_64\\bin"
#
# Notes:
# - The compiler/toolset must match the Qt kit (MinGW 64-bit). Mixing 32-bit MinGW with Qt mingw_64 fails.

if(NOT WIN32)
  message(FATAL_ERROR "qt6-mingw64.cmake is intended for Windows MinGW builds.")
endif()

set(QT6_MINGW64_ROOT "" CACHE PATH "Qt6 MinGW 64-bit kit root (e.g. C:/Qt/6.10.2/mingw_64)")
set(QT6_TOOLS_MINGW64_BIN "" CACHE PATH "Qt Tools MinGW 64-bit bin dir (e.g. C:/Qt/Tools/mingw1310_64/bin)")

# In CMake's try_compile projects, only the compiler is needed. Those scratch projects
# don't automatically inherit arbitrary cache vars, so allow reading from environment.
if(NOT QT6_MINGW64_ROOT AND DEFINED ENV{QT6_MINGW64_ROOT})
  set(QT6_MINGW64_ROOT "$ENV{QT6_MINGW64_ROOT}")
endif()
if(NOT QT6_TOOLS_MINGW64_BIN AND DEFINED ENV{QT6_TOOLS_MINGW64_BIN})
  set(QT6_TOOLS_MINGW64_BIN "$ENV{QT6_TOOLS_MINGW64_BIN}")
endif()

if(QT6_MINGW64_ROOT AND EXISTS "${QT6_MINGW64_ROOT}/lib/cmake/Qt6/Qt6Config.cmake")
  set(CMAKE_PREFIX_PATH "${QT6_MINGW64_ROOT}" CACHE STRING "" FORCE)
endif()

if(QT6_TOOLS_MINGW64_BIN)
  if(EXISTS "${QT6_TOOLS_MINGW64_BIN}/g++.exe")
    set(CMAKE_CXX_COMPILER "${QT6_TOOLS_MINGW64_BIN}/g++.exe" CACHE FILEPATH "" FORCE)
  endif()
  if(EXISTS "${QT6_TOOLS_MINGW64_BIN}/gcc.exe")
    set(CMAKE_C_COMPILER "${QT6_TOOLS_MINGW64_BIN}/gcc.exe" CACHE FILEPATH "" FORCE)
  endif()
  if(EXISTS "${QT6_TOOLS_MINGW64_BIN}/mingw32-make.exe")
    set(CMAKE_MAKE_PROGRAM "${QT6_TOOLS_MINGW64_BIN}/mingw32-make.exe" CACHE FILEPATH "" FORCE)
  endif()
endif()
