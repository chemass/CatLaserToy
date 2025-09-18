# Testing Dependencies and Setup Guide

## Overview

The Cat Laser Toy automated testing system is designed to work with minimal external dependencies, using only standard C++11 libraries and common build tools.

## Required Dependencies

### Core Requirements

**C++ Compiler (C++11 or later)**:
- **Linux**: `g++` (GCC 4.8+ or Clang 3.3+)
- **macOS**: `g++` (via Xcode Command Line Tools or Homebrew)
- **Windows**: MinGW-w64, MSYS2, or Visual Studio

**Build System**:
- **Linux/macOS**: `make` (usually pre-installed)
- **Windows**: `make` (via MSYS2/MinGW) or PowerShell scripts

### Standard Library Dependencies

The tests use only standard C++11 libraries (no external dependencies):

```cpp
#include <iostream>     // Console I/O for test output
#include <vector>       // Dynamic arrays for test data
#include <string>       // String handling
#include <sstream>      // String stream operations
#include <functional>   // Function objects for test callbacks
#include <cmath>        // Math functions (geometry calculations)
#include <cstdlib>      // Standard library utilities
```

## Installation Instructions

### Ubuntu/Debian Linux
```bash
sudo apt-get update
sudo apt-get install -y build-essential g++ make
```

### CentOS/RHEL/Fedora
```bash
# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install gcc-c++

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc-c++
```

### macOS
```bash
# Option 1: Xcode Command Line Tools
xcode-select --install

# Option 2: Homebrew
brew install gcc make
```

### Windows

**Option 1: MSYS2 (Recommended)**
1. Download and install MSYS2 from https://www.msys2.org/
2. Open MSYS2 terminal and run:
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
```
3. Add `C:\msys64\mingw64\bin` to your PATH

**Option 2: MinGW-w64**
1. Download from https://www.mingw-w64.org/
2. Install and add to PATH
3. Verify with `g++ --version`

**Option 3: Visual Studio**
1. Install Visual Studio with C++ support
2. Use Developer Command Prompt
3. May require adaptation of build scripts

## Testing the Installation

### Quick Verification
```bash
cd tests
make test-basic    # Run simple C++11 compatibility test
```

### Full Test Suite
```bash
cd tests
make test         # Run all tests
```

## Common Issues and Solutions

### Compilation Errors

**Error**: `g++: command not found`
```bash
# Solution: Install build tools (see installation instructions above)
```

**Error**: `make: command not found` (Windows)
```bash
# Solution: Use PowerShell script instead
.\build_and_run.ps1
```

**Error**: C++11 features not supported
```bash
# Solution: Update compiler or add explicit C++11 flag
g++ -std=c++11 ...
```

### Library Issues

**Error**: `#include <functional>` not found
```bash
# Solution: Update compiler to C++11 compatible version
# GCC 4.8+, Clang 3.3+, or Visual Studio 2013+
```

**Error**: `auto` keyword not recognized
```bash
# Solution: Ensure C++11 mode is enabled
# The Makefile already includes -std=c++11
```

### Platform-Specific Issues

**Windows**: PowerShell execution policy
```powershell
# Solution: Allow script execution
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

**macOS**: Command Line Tools not installed
```bash
# Solution: Install Xcode Command Line Tools
xcode-select --install
```

**Linux**: Missing development packages
```bash
# Solution: Install development group
sudo apt-get install build-essential
```

## No External Libraries Required

### Why No External Dependencies?

The testing framework is **completely self-contained**:

- ✅ **No external test frameworks** (e.g., Google Test, Catch2)
- ✅ **No package managers** (e.g., vcpkg, Conan)
- ✅ **No build system generators** (e.g., CMake, though it could be added)
- ✅ **No Arduino libraries** for unit tests

### Benefits of This Approach

1. **Simple Setup**: Works immediately on any C++11 system
2. **Fast CI/CD**: No dependency download time
3. **Portable**: Runs everywhere C++11 is supported
4. **Reliable**: No external dependency version conflicts
5. **Educational**: Shows how testing frameworks work internally

## Advanced Tools (Optional)

### Memory Analysis (Linux Only)
```bash
# Install Valgrind for memory leak detection
sudo apt-get install valgrind

# Run memory tests
cd tests
make valgrind
```

### Code Coverage (GCC Only)
```bash
# Generate coverage reports
cd tests
make coverage

# View coverage files
ls *.gcov
```

### Performance Profiling (GCC Only)
```bash
# Generate performance profiles
cd tests
make profile

# View profile data
gprof test_geometry gmon.out
```

## Continuous Integration

The GitHub Actions workflow automatically:
- ✅ Installs all required dependencies
- ✅ Compiles tests on Ubuntu and Windows
- ✅ Runs full test suite
- ✅ Verifies ESP32 Arduino compilation
- ✅ Performs code quality checks

No manual dependency management needed for CI!

## Architecture Benefits

This dependency-minimal approach provides:

### 🚀 **Fast Development**
- Immediate test execution
- No dependency resolution delays
- Works on resource-constrained systems

### 🔧 **Maximum Compatibility**
- Runs on embedded development boards
- Works with older compilers
- Compatible with restricted environments

### 📦 **Easy Distribution**
- Single repository contains everything
- No external package management
- Works offline

---

**Summary**: The testing system requires only a C++11 compiler and make, both of which are standard tools available on all major platforms. No external libraries, frameworks, or package managers are needed! 🧪✨