#include "doctest.h"

// Include source files directly for testing
#include "../PatternGenerator.h"
#include "../PatternGenerator.cpp"

TEST_CASE("ZigzagPattern basic functionality") {
    ZigzagPattern pattern(3, 5); // 3 lines, 5 points per line
    
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    // Should generate points for 3 lines with 5 points each
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
    
    // First point should be at the start
    CHECK_EQ(points[0].u, 0.0f);
    CHECK_EQ(points[0].v, 0.0f);
    
    // All points should be within normalized range
    for (int i = 0; i < numPoints; i++) {
        CHECK_GT(points[i].u, -0.001f);
        CHECK_LT(points[i].u, 1.001f);
        CHECK_GT(points[i].v, -0.001f);
        CHECK_LT(points[i].v, 1.001f);
    }
}

TEST_CASE("ZigzagPattern configuration") {
    ZigzagPattern pattern;
    
    // Test configuration methods
    pattern.setLines(5);
    pattern.setPointsPerLine(8);
    
    // Reset should work without issues
    pattern.reset();
    
    CHECK_EQ(pattern.getName(), std::string("Zigzag"));
}

TEST_CASE("SpiralPattern basic functionality") {
    SpiralPattern pattern(0.4f, 20, 2.0f); // radius=0.4, 20 points, tightness=2.0
    
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
    
    // All points should be within normalized range
    for (int i = 0; i < numPoints; i++) {
        CHECK_GT(points[i].u, -0.001f);
        CHECK_LT(points[i].u, 1.001f);
        CHECK_GT(points[i].v, -0.001f);
        CHECK_LT(points[i].v, 1.001f);
    }
    
    CHECK_EQ(pattern.getName(), std::string("Spiral"));
}

TEST_CASE("SpiralPattern configuration") {
    SpiralPattern pattern;
    
    pattern.setRadius(0.3f);
    pattern.setPoints(15);
    pattern.setTightness(1.5f);
    
    pattern.reset();
    
    // Should be able to generate pattern with new configuration
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    CHECK_GT(numPoints, 0);
}

TEST_CASE("RandomWalkPattern basic functionality") {
    RandomWalkPattern pattern(20, 0.1f); // 20 steps, step size 0.1
    
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
    
    // All points should be within normalized range (might be slightly outside due to random walk)
    for (int i = 0; i < numPoints; i++) {
        // Random walk might go slightly outside, but should mostly stay in bounds
        CHECK_GT(points[i].u, -0.2f);
        CHECK_LT(points[i].u, 1.2f);
        CHECK_GT(points[i].v, -0.2f);
        CHECK_LT(points[i].v, 1.2f);
    }
    
    CHECK_EQ(pattern.getName(), std::string("RandomWalk"));
}

TEST_CASE("Figure8Pattern basic functionality") {
    Figure8Pattern pattern(0.3f, 16); // radius=0.3, 16 points
    
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
    
    // All points should be within normalized range
    for (int i = 0; i < numPoints; i++) {
        CHECK_GT(points[i].u, -0.001f);
        CHECK_LT(points[i].u, 1.001f);
        CHECK_GT(points[i].v, -0.001f);
        CHECK_LT(points[i].v, 1.001f);
    }
    
    CHECK_EQ(pattern.getName(), std::string("Figure8"));
}

TEST_CASE("PerimeterPattern basic functionality") {
    PerimeterPattern pattern(20); // 20 points around perimeter
    
    NormalizedPoint points[50];
    int numPoints = pattern.generatePattern(points, 50);
    
    CHECK_GT(numPoints, 0);
    CHECK_LT(numPoints, 50);
    
    // All points should be on the boundary (0 or 1 for at least one coordinate)
    for (int i = 0; i < numPoints; i++) {
        bool onBoundary = (points[i].u <= 0.001f || points[i].u >= 0.999f ||
                          points[i].v <= 0.001f || points[i].v >= 0.999f);
        CHECK(onBoundary);
    }
    
    CHECK_EQ(pattern.getName(), std::string("Perimeter"));
}

TEST_CASE("Pattern reset functionality") {
    ZigzagPattern zigzag;
    SpiralPattern spiral;
    RandomWalkPattern randomWalk;
    Figure8Pattern figure8;
    PerimeterPattern perimeter;
    
    // Reset should not crash and should allow regeneration
    zigzag.reset();
    spiral.reset();
    randomWalk.reset();
    figure8.reset();
    perimeter.reset();
    
    // Should be able to generate patterns after reset
    NormalizedPoint points[20];
    CHECK_GT(zigzag.generatePattern(points, 20), 0);
    CHECK_GT(spiral.generatePattern(points, 20), 0);
    CHECK_GT(randomWalk.generatePattern(points, 20), 0);
    CHECK_GT(figure8.generatePattern(points, 20), 0);
    CHECK_GT(perimeter.generatePattern(points, 20), 0);
}

TEST_CASE("Pattern edge cases - zero points") {
    ZigzagPattern pattern;
    NormalizedPoint points[1];
    
    // Requesting 0 points should return 0
    int numPoints = pattern.generatePattern(points, 0);
    CHECK_EQ(numPoints, 0);
}

TEST_CASE("Pattern edge cases - minimal configuration") {
    // Test with minimal valid configurations
    ZigzagPattern zigzag(1, 1); // 1 line, 1 point
    SpiralPattern spiral(0.1f, 1, 0.1f); // minimal values
    
    NormalizedPoint points[10];
    
    CHECK_GT(zigzag.generatePattern(points, 10), 0);
    CHECK_GT(spiral.generatePattern(points, 10), 0);
}

TEST_CASE("Pattern deterministic behavior") {
    // Non-random patterns should generate same sequence
    ZigzagPattern pattern1(3, 4);
    ZigzagPattern pattern2(3, 4);
    
    NormalizedPoint points1[20], points2[20];
    
    int num1 = pattern1.generatePattern(points1, 20);
    int num2 = pattern2.generatePattern(points2, 20);
    
    CHECK_EQ(num1, num2);
    
    // Points should be identical
    for (int i = 0; i < num1; i++) {
        CHECK_EQ(points1[i].u, points2[i].u);
        CHECK_EQ(points1[i].v, points2[i].v);
    }
}