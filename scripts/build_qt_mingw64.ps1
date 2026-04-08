param(
  [string]$QtRoot = "C:\Qt",
  [string]$QtVersion = "",
  [string]$BuildDir = "build_qt_mingw64",
  [switch]$Clean,
  [string]$QtToolsRoot = ""
)

$ErrorActionPreference = "Stop"

function Resolve-QtKitRoot([string]$qtRoot, [string]$qtVersion) {
  if ($qtVersion -ne "") {
    $candidate = Join-Path $qtRoot $qtVersion
    if (Test-Path $candidate) { return $candidate }
    throw "Qt version folder not found: $candidate"
  }

  $dirs = Get-ChildItem -Path $qtRoot -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
    Sort-Object { [version]$_.Name } -Descending

  if (-not $dirs -or $dirs.Count -eq 0) {
    throw "No Qt versions found under $qtRoot (expected folders like 6.10.2)."
  }
  return $dirs[0].FullName
}

function Resolve-QtMingw64Root([string]$qtKitRoot) {
  $candidate = Join-Path $qtKitRoot "mingw_64"
  if (-not (Test-Path $candidate)) { throw "Qt MinGW kit not found: $candidate" }
  $qt6Config = Join-Path $candidate "lib\cmake\Qt6\Qt6Config.cmake"
  if (-not (Test-Path $qt6Config)) { throw "Qt6Config.cmake not found: $qt6Config" }
  return $candidate
}

function Resolve-QtToolsMingwBin([string]$qtRoot, [string]$explicitToolsRoot) {
  if ($explicitToolsRoot -ne "") {
    $bin = Join-Path $explicitToolsRoot "bin"
    if (-not (Test-Path (Join-Path $bin "g++.exe"))) { throw "g++.exe not found in $bin" }
    return $bin
  }

  $tools = Join-Path $qtRoot "Tools"
  if (-not (Test-Path $tools)) { return "" }

  $mingwDirs = Get-ChildItem -Path $tools -Directory -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -match '^mingw.*_64$' } |
    Sort-Object Name -Descending

  foreach ($d in $mingwDirs) {
    $bin = Join-Path $d.FullName "bin"
    if (Test-Path (Join-Path $bin "g++.exe")) { return $bin }
  }
  return ""
}

function Read-CacheValue([string]$cachePath, [string]$name) {
  $line = Select-String -Path $cachePath -Pattern ("^" + [regex]::Escape($name) + ":") -ErrorAction SilentlyContinue | Select-Object -First 1
  if (-not $line) { return "" }
  $parts = $line.Line.Split('=', 2)
  if ($parts.Count -ne 2) { return "" }
  return $parts[1]
}

$qtKit = Resolve-QtKitRoot -qtRoot $QtRoot -qtVersion $QtVersion
$qtMingw64 = Resolve-QtMingw64Root -qtKitRoot $qtKit
$toolsBin = Resolve-QtToolsMingwBin -qtRoot $QtRoot -explicitToolsRoot $QtToolsRoot

Write-Host "Qt kit: $qtKit"
Write-Host "Qt MinGW64: $qtMingw64"
if ($toolsBin -ne "") { Write-Host "Qt Tools MinGW bin: $toolsBin" } else { Write-Host "Qt Tools MinGW bin: (not found; using current PATH compiler)" }

# Make values available to CMake try_compile scratch projects (toolchain reads env vars too).
$env:QT6_MINGW64_ROOT = $qtMingw64
if ($toolsBin -ne "") { $env:QT6_TOOLS_MINGW64_BIN = $toolsBin }

if (Test-Path $BuildDir) {
  if ($Clean) {
    Write-Host "Cleaning build dir: $BuildDir"
    Remove-Item -Recurse -Force $BuildDir
  } else {
    $cache = Join-Path $BuildDir "CMakeCache.txt"
    if (Test-Path $cache) {
      $cachedCompiler = Read-CacheValue -cachePath $cache -name 'CMAKE_CXX_COMPILER:FILEPATH'
      if ($toolsBin -ne "") {
        $expected = (Join-Path $toolsBin 'g++.exe')
        if ($cachedCompiler -and ($cachedCompiler -ne $expected)) {
          Write-Host "Build dir '$BuildDir' was configured with a different compiler:"
          Write-Host "  cached:   $cachedCompiler"
          Write-Host "  expected: $expected"
          Write-Host "Re-run with -Clean or use a new -BuildDir."
          exit 2
        }
      }

      Write-Host "Reusing existing build dir: $BuildDir"
      & cmake --build $BuildDir
      if ($LASTEXITCODE -ne 0) { throw "CMake build failed with exit code $LASTEXITCODE" }
      Write-Host "Run: .\$BuildDir\MiniOSSimulator.exe"
      exit 0
    }
  }
}

$toolchain = Join-Path $PSScriptRoot "..\cmake\toolchains\qt6-mingw64.cmake"
$toolchain = (Resolve-Path $toolchain).Path

$args = @(
  "-S", ".", "-B", $BuildDir, "-G", "MinGW Makefiles",
  "-DCMAKE_BUILD_TYPE=Release",
  "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
  "-DQT6_MINGW64_ROOT=$qtMingw64"
)
if ($toolsBin -ne "") { $args += "-DQT6_TOOLS_MINGW64_BIN=$toolsBin" }

& cmake @args
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }

& cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) { throw "CMake build failed with exit code $LASTEXITCODE" }

Write-Host "Run: .\$BuildDir\MiniOSSimulator.exe"
