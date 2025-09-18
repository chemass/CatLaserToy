#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Include source files directly for testing (no Arduino dependencies)
#include "../Geometry.h"
#include "../Geometry.cpp"

TEST_CASE("Point construction and basic operations") {
    // Test default constructor
    Point p1;
    CHECK_EQ(p1.x, 0);
    CHECK_EQ(p1.y, 0);
    
    // Test parameterized constructor
    Point p2(10, 20);
    CHECK_EQ(p2.x, 10);
    CHECK_EQ(p2.y, 20);
    
    // Test equality
    Point p3(10, 20);
    CHECK(p2.equals(p3));
    CHECK(!p1.equals(p2));
}

TEST_CASE("Point distance calculations") {
    Point p1(0, 0);
    Point p2(3, 4);
    
    // Distance squared should be 3² + 4² = 25
    CHECK_EQ(p1.distanceSquared(p2), 25);
    CHECK_EQ(p2.distanceSquared(p1), 25);
    
    // Self distance should be 0
    CHECK_EQ(p1.distanceSquared(p1), 0);
}

TEST_CASE("NormalizedPoint construction and clamping") {
    // Test default constructor
    NormalizedPoint np1;
    CHECK_EQ(np1.u, 0.5f);
    CHECK_EQ(np1.v, 0.5f);
    
    // Test parameterized constructor
    NormalizedPoint np2(0.3f, 0.7f);
    CHECK_EQ(np2.u, 0.3f);
    CHECK_EQ(np2.v, 0.7f);
    
    // Test clamping - values within range should remain unchanged
    NormalizedPoint np3(0.2f, 0.8f);
    np3.clamp();
    CHECK_EQ(np3.u, 0.2f);
    CHECK_EQ(np3.v, 0.8f);
    
    // Test clamping - values outside range should be clamped
    NormalizedPoint np4(-0.5f, 1.5f);
    np4.clamp();
    CHECK_EQ(np4.u, 0.0f);
    CHECK_EQ(np4.v, 1.0f);
}

TEST_CASE("Rectangle construction and methods") {
    // Test default constructor
    Rectangle rect1;
    CHECK_EQ(rect1.topLeft.x, 0);
    CHECK_EQ(rect1.topLeft.y, 0);
    CHECK_EQ(rect1.bottomRight.x, 100);
    CHECK_EQ(rect1.bottomRight.y, 100);
    
    // Test width and height
    CHECK_EQ(rect1.width(), 100);
    CHECK_EQ(rect1.height(), 100);
    
    // Test center calculation
    Point center = rect1.center();
    CHECK_EQ(center.x, 50);
    CHECK_EQ(center.y, 50);
    
    // Test custom rectangle
    Rectangle rect2(Point(10, 20), Point(30, 50));
    CHECK_EQ(rect2.width(), 20);
    CHECK_EQ(rect2.height(), 30);
    Point center2 = rect2.center();
    CHECK_EQ(center2.x, 20);
    CHECK_EQ(center2.y, 35);
}

TEST_CASE("Quadrilateral construction and center") {
    // Test default constructor (creates a square)
    Quadrilateral quad1;
    CHECK(quad1.corners[0].equals(Point(0, 0)));
    CHECK(quad1.corners[1].equals(Point(100, 0)));
    CHECK(quad1.corners[2].equals(Point(100, 100)));
    CHECK(quad1.corners[3].equals(Point(0, 100)));
    
    Point center1 = quad1.center();
    CHECK_EQ(center1.x, 50);
    CHECK_EQ(center1.y, 50);
    
    // Test custom quadrilateral
    Point corners[4] = {Point(0, 0), Point(20, 0), Point(20, 10), Point(0, 10)};
    Quadrilateral quad2(corners);
    Point center2 = quad2.center();
    CHECK_EQ(center2.x, 10);
    CHECK_EQ(center2.y, 5);
}

TEST_CASE("Geometry utility functions - constrainToBounds") {
    // Point within bounds should remain unchanged
    Point p1 = Geometry::constrainToBounds(Point(50, 60), 0, 100, 0, 100);
    CHECK_EQ(p1.x, 50);
    CHECK_EQ(p1.y, 60);
    
    // Point outside bounds should be constrained
    Point p2 = Geometry::constrainToBounds(Point(-10, 150), 0, 100, 0, 100);
    CHECK_EQ(p2.x, 0);
    CHECK_EQ(p2.y, 100);
    
    Point p3 = Geometry::constrainToBounds(Point(150, -10), 0, 100, 0, 100);
    CHECK_EQ(p3.x, 100);
    CHECK_EQ(p3.y, 0);
}

TEST_CASE("Geometry utility functions - isWithinBounds") {
    CHECK(Geometry::isWithinBounds(Point(50, 60), 0, 100, 0, 100));
    CHECK(Geometry::isWithinBounds(Point(0, 0), 0, 100, 0, 100));
    CHECK(Geometry::isWithinBounds(Point(100, 100), 0, 100, 0, 100));
    
    CHECK(!Geometry::isWithinBounds(Point(-1, 50), 0, 100, 0, 100));
    CHECK(!Geometry::isWithinBounds(Point(50, -1), 0, 100, 0, 100));
    CHECK(!Geometry::isWithinBounds(Point(101, 50), 0, 100, 0, 100));
    CHECK(!Geometry::isWithinBounds(Point(50, 101), 0, 100, 0, 100));
}

TEST_CASE("Geometry utility functions - lerp") {
    Point start(0, 0);
    Point end(100, 200);
    
    // t = 0 should return start point
    Point p1 = Geometry::lerp(start, end, 0.0f);
    CHECK_EQ(p1.x, 0);
    CHECK_EQ(p1.y, 0);
    
    // t = 1 should return end point
    Point p2 = Geometry::lerp(start, end, 1.0f);
    CHECK_EQ(p2.x, 100);
    CHECK_EQ(p2.y, 200);
    
    // t = 0.5 should return midpoint
    Point p3 = Geometry::lerp(start, end, 0.5f);
    CHECK_EQ(p3.x, 50);
    CHECK_EQ(p3.y, 100);
}

TEST_CASE("Coordinate space transformations - mapToRectangle") {
    Rectangle rect(Point(0, 0), Point(100, 200));
    
    // Normalized (0,0) should map to top-left
    Point p1 = Geometry::mapToRectangle(NormalizedPoint(0.0f, 0.0f), rect);
    CHECK_EQ(p1.x, 0);
    CHECK_EQ(p1.y, 0);
    
    // Normalized (1,1) should map to bottom-right
    Point p2 = Geometry::mapToRectangle(NormalizedPoint(1.0f, 1.0f), rect);
    CHECK_EQ(p2.x, 100);
    CHECK_EQ(p2.y, 200);
    
    // Normalized (0.5,0.5) should map to center
    Point p3 = Geometry::mapToRectangle(NormalizedPoint(0.5f, 0.5f), rect);
    CHECK_EQ(p3.x, 50);
    CHECK_EQ(p3.y, 100);
}

TEST_CASE("Coordinate space transformations - mapFromRectangle") {
    Rectangle rect(Point(0, 0), Point(100, 200));
    
    // Top-left should map to normalized (0,0)
    NormalizedPoint np1 = Geometry::mapFromRectangle(Point(0, 0), rect);
    CHECK_EQ(np1.u, 0.0f);
    CHECK_EQ(np1.v, 0.0f);
    
    // Bottom-right should map to normalized (1,1)
    NormalizedPoint np2 = Geometry::mapFromRectangle(Point(100, 200), rect);
    CHECK_EQ(np2.u, 1.0f);
    CHECK_EQ(np2.v, 1.0f);
    
    // Center should map to normalized (0.5,0.5)
    NormalizedPoint np3 = Geometry::mapFromRectangle(Point(50, 100), rect);
    CHECK_EQ(np3.u, 0.5f);
    CHECK_EQ(np3.v, 0.5f);
}

TEST_CASE("Distance calculation") {
    Point p1(0, 0);
    Point p2(3, 4);
    
    float dist = Geometry::distance(p1, p2);
    CHECK_GT(dist, 4.99f);  // Should be approximately 5.0
    CHECK_LT(dist, 5.01f);
    
    // Distance should be symmetric
    CHECK_EQ(Geometry::distance(p1, p2), Geometry::distance(p2, p1));
    
    // Self distance should be 0
    CHECK_EQ(Geometry::distance(p1, p1), 0.0f);
}

TEST_CASE("Bilinear interpolation") {
    // Create a simple rectangle quadrilateral
    Quadrilateral quad;
    quad.corners[0] = Point(0, 0);   // Top-left
    quad.corners[1] = Point(100, 0); // Top-right
    quad.corners[2] = Point(100, 100); // Bottom-right
    quad.corners[3] = Point(0, 100); // Bottom-left
    
    // Test corners
    Point p1 = Geometry::bilinearInterpolate(0.0f, 0.0f, quad);
    CHECK_EQ(p1.x, 0);
    CHECK_EQ(p1.y, 0);
    
    Point p2 = Geometry::bilinearInterpolate(1.0f, 0.0f, quad);
    CHECK_EQ(p2.x, 100);
    CHECK_EQ(p2.y, 0);
    
    Point p3 = Geometry::bilinearInterpolate(1.0f, 1.0f, quad);
    CHECK_EQ(p3.x, 100);
    CHECK_EQ(p3.y, 100);
    
    Point p4 = Geometry::bilinearInterpolate(0.0f, 1.0f, quad);
    CHECK_EQ(p4.x, 0);
    CHECK_EQ(p4.y, 100);
    
    // Test center
    Point center = Geometry::bilinearInterpolate(0.5f, 0.5f, quad);
    CHECK_EQ(center.x, 50);
    CHECK_EQ(center.y, 50);
}