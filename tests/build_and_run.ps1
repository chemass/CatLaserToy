# PowerShell script to build and run tests on Windows
# Usage: .\build_and_run.ps1

param(
    [switch]$Clean,
    [switch]$Coverage,
    [switch]$Profile,
    [string]$TestSuite = "all"
)

Write-Host "Cat Laser Toy - Test Builder for Windows" -ForegroundColor Green
Write-Host "=======================================" -ForegroundColor Green

# Check if g++ is available
try {
    $gccVersion = & g++ --version 2>$null
    Write-Host "Found G++: $($gccVersion[0])" -ForegroundColor Green
} catch {
    Write-Host "Error: g++ not found. Please install MinGW-w64 or MSYS2." -ForegroundColor Red
    Write-Host "Download from: https://www.msys2.org/" -ForegroundColor Yellow
    exit 1
}

# Compiler settings
$CXX = "g++"
$CXXFLAGS = "-std=c++11 -Wall -Wextra -O2 -g"
$INCLUDES = "-I. -I.."

if ($Coverage) {
    $CXXFLAGS += " --coverage"
}

if ($Profile) {
    $CXXFLAGS += " -pg"
}

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    Remove-Item -Path "*.exe" -ErrorAction SilentlyContinue
    Remove-Item -Path "*.o" -ErrorAction SilentlyContinue
    Remove-Item -Path "*.gcda" -ErrorAction SilentlyContinue
    Remove-Item -Path "*.gcno" -ErrorAction SilentlyContinue
    Write-Host "Clean completed." -ForegroundColor Green
    if ($TestSuite -eq "clean") { exit 0 }
}

# Build function
function Build-Test {
    param($TestName, $Sources)
    
    Write-Host "Building $TestName..." -ForegroundColor Yellow
    $Command = "$CXX $CXXFLAGS $INCLUDES -o $TestName.exe $Sources"
    Write-Host "  Command: $Command" -ForegroundColor Gray
    
    $Output = & cmd /c $Command '2>&1'
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed for $TestName" -ForegroundColor Red
        Write-Host $Output -ForegroundColor Red
        return $false
    } else {
        Write-Host "  ✓ Build successful" -ForegroundColor Green
        return $true
    }
}

# Run test function
function Run-Test {
    param($TestName)
    
    if (Test-Path "$TestName.exe") {
        Write-Host "Running $TestName..." -ForegroundColor Yellow
        & ".\$TestName.exe"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ Tests passed" -ForegroundColor Green
            return $true
        } else {
            Write-Host "  ✗ Tests failed" -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "  ✗ Test executable not found: $TestName.exe" -ForegroundColor Red
        return $false
    }
}

# Test definitions
$Tests = @{
    "geometry" = @{
        "sources" = "../Geometry.cpp test_geometry.cpp"
        "name" = "test_geometry"
    }
    "patterns" = @{
        "sources" = "../PatternGenerator.cpp test_patterns.cpp"
        "name" = "test_patterns"
    }
    "motion" = @{
        "sources" = "../MotionPlanner.cpp ../MotionExecutor.cpp test_motion_system.cpp"
        "name" = "test_motion_system"
    }
}

$BuildSuccess = $true
$TestSuccess = $true

# Build and run tests based on TestSuite parameter
switch ($TestSuite.ToLower()) {
    "geometry" {
        $BuildSuccess = Build-Test $Tests.geometry.name $Tests.geometry.sources
        if ($BuildSuccess) { $TestSuccess = Run-Test $Tests.geometry.name }
    }
    "patterns" {
        $BuildSuccess = Build-Test $Tests.patterns.name $Tests.patterns.sources
        if ($BuildSuccess) { $TestSuccess = Run-Test $Tests.patterns.name }
    }
    "motion" {
        $BuildSuccess = Build-Test $Tests.motion.name $Tests.motion.sources
        if ($BuildSuccess) { $TestSuccess = Run-Test $Tests.motion.name }
    }
    "all" {
        foreach ($test in $Tests.GetEnumerator()) {
            $result = Build-Test $test.Value.name $test.Value.sources
            $BuildSuccess = $BuildSuccess -and $result
            
            if ($result) {
                $result = Run-Test $test.Value.name
                $TestSuccess = $TestSuccess -and $result
            }
        }
    }
    default {
        Write-Host "Unknown test suite: $TestSuite" -ForegroundColor Red
        Write-Host "Available options: all, geometry, patterns, motion" -ForegroundColor Yellow
        exit 1
    }
}

# Coverage report
if ($Coverage -and $BuildSuccess -and $TestSuccess) {
    Write-Host "Generating coverage report..." -ForegroundColor Yellow
    try {
        & gcov ../Geometry.cpp ../PatternGenerator.cpp ../MotionPlanner.cpp ../MotionExecutor.cpp
        Write-Host "  ✓ Coverage report generated" -ForegroundColor Green
    } catch {
        Write-Host "  Warning: gcov not available for coverage analysis" -ForegroundColor Yellow
    }
}

# Summary
Write-Host "`n=======================================" -ForegroundColor Green
if ($BuildSuccess -and $TestSuccess) {
    Write-Host "All tests completed successfully! ✓" -ForegroundColor Green
    exit 0
} else {
    if (-not $BuildSuccess) {
        Write-Host "Build failed! ✗" -ForegroundColor Red
    }
    if (-not $TestSuccess) {
        Write-Host "Tests failed! ✗" -ForegroundColor Red
    }
    exit 1
}