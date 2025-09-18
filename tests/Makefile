# Makefile for Cat Laser Toy Tests
# Cross-platform build system for unit tests

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -g
INCLUDES = -I. -I..
LDFLAGS = 

# Source files
GEOMETRY_SOURCES = ../Geometry.cpp test_geometry.cpp
PATTERN_SOURCES = ../PatternGenerator.cpp test_patterns.cpp  
MOTION_SOURCES = ../MotionPlanner.cpp ../MotionExecutor.cpp test_motion_system.cpp

# Test executables
GEOMETRY_TEST = test_geometry
PATTERN_TEST = test_patterns
MOTION_TEST = test_motion_system

# Default target
all: $(GEOMETRY_TEST) $(PATTERN_TEST) $(MOTION_TEST)

# Individual test targets
$(GEOMETRY_TEST): $(GEOMETRY_SOURCES) doctest.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(GEOMETRY_SOURCES) $(LDFLAGS)

$(PATTERN_TEST): $(PATTERN_SOURCES) doctest.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(PATTERN_SOURCES) $(LDFLAGS)

$(MOTION_TEST): $(MOTION_SOURCES) doctest.h MockServoController.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(MOTION_SOURCES) $(LDFLAGS)

# Run all tests
test: all
	@echo "Running Geometry Tests..."
	./$(GEOMETRY_TEST)
	@echo "\nRunning Pattern Tests..."
	./$(PATTERN_TEST)
	@echo "\nRunning Motion System Tests..."
	./$(MOTION_TEST)
	@echo "\nAll tests completed!"

# Individual test runs
test-geometry: $(GEOMETRY_TEST)
	./$(GEOMETRY_TEST)

test-patterns: $(PATTERN_TEST)
	./$(PATTERN_TEST)

test-motion: $(MOTION_TEST)
	./$(MOTION_TEST)

# Clean build artifacts
clean:
	rm -f $(GEOMETRY_TEST) $(PATTERN_TEST) $(MOTION_TEST)
	rm -f *.o *.exe

# Memory leak detection (Linux only)
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all ./$(GEOMETRY_TEST)
	valgrind --leak-check=full --show-leak-kinds=all ./$(PATTERN_TEST)
	valgrind --leak-check=full --show-leak-kinds=all ./$(MOTION_TEST)

# Coverage analysis (requires gcov)
coverage: CXXFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: all
	./$(GEOMETRY_TEST)
	./$(PATTERN_TEST)
	./$(MOTION_TEST)
	gcov $(GEOMETRY_SOURCES) $(PATTERN_SOURCES) $(MOTION_SOURCES)

# Performance profiling (requires gprof)
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: all
	./$(GEOMETRY_TEST)
	./$(PATTERN_TEST)
	./$(MOTION_TEST)
	gprof $(GEOMETRY_TEST) gmon.out > geometry_profile.txt
	gprof $(PATTERN_TEST) gmon.out > pattern_profile.txt
	gprof $(MOTION_TEST) gmon.out > motion_profile.txt

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Build all test executables"
	@echo "  test        - Build and run all tests"
	@echo "  test-geometry - Run only geometry tests"
	@echo "  test-patterns - Run only pattern tests"
	@echo "  test-motion - Run only motion system tests"
	@echo "  clean       - Remove build artifacts"
	@echo "  valgrind    - Run tests with memory leak detection (Linux)"
	@echo "  coverage    - Generate code coverage report"
	@echo "  profile     - Generate performance profiles"
	@echo "  help        - Show this help message"

# Phony targets
.PHONY: all test test-geometry test-patterns test-motion clean valgrind coverage profile help

# Windows-specific adjustments
ifeq ($(OS),Windows_NT)
    # Windows uses .exe extension
    GEOMETRY_TEST := $(GEOMETRY_TEST).exe
    PATTERN_TEST := $(PATTERN_TEST).exe
    MOTION_TEST := $(MOTION_TEST).exe
    
    # Windows doesn't have valgrind
    valgrind:
	@echo "Valgrind not available on Windows. Use Visual Studio diagnostics instead."
endif