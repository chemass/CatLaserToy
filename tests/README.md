# Cat Laser Toy - Test Suite

This directory contains comprehensive unit tests for the Cat Laser Toy ESP32 project. The tests are designed to run without hardware dependencies, enabling fast development and continuous integration.

## Test Architecture

### Testing Framework
- **doctest.h**: Lightweight, single-header C++ testing framework
- **MockServoController.h**: Hardware abstraction mock for testing motion systems
- Cross-platform support (Windows, Linux, macOS)

### Test Files
- `test_geometry.cpp` - Point operations, coordinate transformations, boundary mapping
- `test_patterns.cpp` - All 5 pattern generators with various configurations  
- `test_motion_system.cpp` - Motion planning and execution with hardware mocks

## Quick Start

### Windows (PowerShell)
```powershell
# Install prerequisites (if needed)
# Download and install MSYS2 from https://www.msys2.org/
# Then: pacman -S mingw-w64-x86_64-gcc

# Run all tests
.\build_and_run.ps1

# Run specific test suite
.\build_and_run.ps1 -TestSuite geometry
.\build_and_run.ps1 -TestSuite patterns
.\build_and_run.ps1 -TestSuite motion

# Clean build artifacts
.\build_and_run.ps1 -Clean
```

### Linux/macOS (Make)
```bash
# Install prerequisites
sudo apt-get install build-essential g++ make  # Ubuntu/Debian
# or
brew install gcc make  # macOS with Homebrew

# Run all tests
make test

# Run specific tests
make test-geometry
make test-patterns  
make test-motion

# Clean build artifacts
make clean

# Advanced testing
make valgrind    # Memory leak detection (Linux only)
make coverage    # Code coverage analysis
make profile     # Performance profiling
```

## Test Coverage

### Geometry System (test_geometry.cpp)
- ✅ Point construction and operations
- ✅ NormalizedPoint clamping and validation
- ✅ Rectangle and Quadrilateral geometry
- ✅ Coordinate space transformations
- ✅ Bilinear interpolation for boundary mapping
- ✅ Distance and utility calculations

### Pattern Generators (test_patterns.cpp)
- ✅ ZigzagPattern - Line-based scanning patterns
- ✅ SpiralPattern - Concentric circular motion
- ✅ RandomWalkPattern - Unpredictable movement
- ✅ Figure8Pattern - Smooth infinity loops
- ✅ PerimeterPattern - Boundary tracing
- ✅ Configuration and reset functionality
- ✅ Edge cases and error handling

### Motion System (test_motion_system.cpp)
- ✅ MotionPlanner boundary setup and validation
- ✅ MotionExecutor command queueing
- ✅ Non-blocking execution and timing
- ✅ Emergency stop functionality
- ✅ Queue overflow protection
- ✅ Integration testing with mocks
- ✅ Hardware abstraction validation

## Test Structure

Each test follows the **AAA pattern**:
```cpp
TEST_CASE("Feature description") {
    // ARRANGE: Setup test data
    ZigzagPattern pattern(3, 5);
    
    // ACT: Execute the functionality
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    // ASSERT: Verify the results
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
}
```

## Benefits of This Testing Approach

### 🚀 **Fast Feedback**
- Tests complete in milliseconds
- No hardware setup required
- Immediate validation during development

### 🔧 **Hardware Independence**
- Mock objects simulate ESP32 peripherals
- Tests run on any development machine
- Consistent results across environments

### 📊 **Comprehensive Coverage**
- Unit tests for individual components
- Integration tests for system interactions
- Edge case and error condition validation

### 🔄 **Continuous Integration Ready**
- Automated testing in GitHub Actions
- Cross-platform compatibility
- Easy integration with CI/CD pipelines

## Advanced Features

### Memory Leak Detection (Linux)
```bash
make valgrind
```
Uses Valgrind to detect memory leaks and invalid memory access.

### Code Coverage Analysis
```bash
make coverage
```
Generates coverage reports showing which code paths are tested.

### Performance Profiling
```bash
make profile
```
Creates performance profiles to identify bottlenecks.

## Integration with CI/CD

The tests are integrated with GitHub Actions for automated testing:
- ✅ Build verification for ESP32 target
- ✅ Cross-platform test execution
- ✅ Code quality checks and analysis
- ✅ Automated reporting and notifications

## Troubleshooting

### Common Issues

**Build Errors on Windows:**
- Install MSYS2 and MinGW-w64 toolchain
- Ensure g++ is in PATH
- Run from PowerShell as Administrator if needed

**Test Failures:**
- Check that source files haven't been modified incompatibly
- Verify all required headers are included
- Review test output for specific assertion failures

**Performance Issues:**
- Use optimized builds for performance testing
- Consider compiler flags: `-O2` or `-O3`
- Profile specific test cases if needed

### Getting Help

1. Check test output for specific failure details
2. Review the main project documentation in `../TESTING.md`
3. Examine source code and test expectations
4. Run individual test suites to isolate issues

## Future Enhancements

- **Property-based testing** with random input generation
- **Mutation testing** to verify test quality
- **Visual regression testing** for pattern validation
- **Hardware-in-the-loop testing** with real ESP32 devices

---

**Happy Testing! 🧪✨**

*This test suite ensures reliable, maintainable code for the Cat Laser Toy project while enabling rapid development and confident refactoring.*