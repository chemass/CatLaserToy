#!/bin/bash
# Linux/macOS test runner script
# Usage: ./run_tests.sh [test_suite]

set -e  # Exit on error

echo "Cat Laser Toy - Test Runner for Linux/macOS"
echo "============================================"

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "Error: make not found. Please install build tools."
    exit 1
fi

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install g++ compiler."
    exit 1
fi

# Default to running all tests
TEST_SUITE=${1:-"all"}

case $TEST_SUITE in
    "geometry")
        echo "Running geometry tests..."
        make test-geometry
        ;;
    "patterns")
        echo "Running pattern tests..."
        make test-patterns
        ;;
    "motion")
        echo "Running motion system tests..."
        make test-motion
        ;;
    "all")
        echo "Running all tests..."
        make test
        ;;
    "clean")
        echo "Cleaning build artifacts..."
        make clean
        ;;
    "coverage")
        echo "Running tests with coverage analysis..."
        make coverage
        ;;
    "valgrind")
        echo "Running tests with memory leak detection..."
        make valgrind
        ;;
    "help")
        echo "Available commands:"
        echo "  ./run_tests.sh [geometry|patterns|motion|all|clean|coverage|valgrind|help]"
        echo ""
        echo "  geometry  - Run only geometry tests"
        echo "  patterns  - Run only pattern tests"
        echo "  motion    - Run only motion system tests"
        echo "  all       - Run all tests (default)"
        echo "  clean     - Clean build artifacts"
        echo "  coverage  - Run tests with coverage analysis"
        echo "  valgrind  - Run tests with memory leak detection"
        echo "  help      - Show this help message"
        ;;
    *)
        echo "Unknown test suite: $TEST_SUITE"
        echo "Use './run_tests.sh help' for available options"
        exit 1
        ;;
esac

echo "Test execution completed!"