#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>

// Point structure for 2D coordinates
struct Point {
    int x;
    int y;
    
    // Constructor
    Point() : x(0), y(0) {}
    Point(int x_val, int y_val) : x(x_val), y(y_val) {}
    
    // Utility methods
    bool equals(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    // Distance calculation (returns squared distance to avoid floating point)
    long distanceSquared(const Point& other) const {
        long dx = x - other.x;
        long dy = y - other.y;
        return dx * dx + dy * dy;
    }
};

// Normalized point for coordinate-space independent calculations
struct NormalizedPoint {
    float u; // 0.0 to 1.0
    float v; // 0.0 to 1.0
    
    NormalizedPoint() : u(0.5f), v(0.5f) {}
    NormalizedPoint(float u_val, float v_val) : u(u_val), v(v_val) {}
    
    // Clamp to valid range
    void clamp() {
        u = (u < 0.0f) ? 0.0f : (u > 1.0f) ? 1.0f : u;
        v = (v < 0.0f) ? 0.0f : (v > 1.0f) ? 1.0f : v;
    }
};

// Rectangle for boundary definition
struct Rectangle {
    Point topLeft;
    Point bottomRight;
    
    Rectangle() : topLeft(0, 0), bottomRight(100, 100) {}
    Rectangle(Point tl, Point br) : topLeft(tl), bottomRight(br) {}
    
    int width() const { return bottomRight.x - topLeft.x; }
    int height() const { return bottomRight.y - topLeft.y; }
    Point center() const { 
        return Point(topLeft.x + width()/2, topLeft.y + height()/2); 
    }
};

// Quadrilateral for arbitrary 4-point boundaries
struct Quadrilateral {
    Point corners[4]; // [0]=TL, [1]=TR, [2]=BR, [3]=BL (clockwise from top-left)
    
    Quadrilateral() {
        corners[0] = Point(0, 0);   // Top-left
        corners[1] = Point(100, 0); // Top-right  
        corners[2] = Point(100, 100); // Bottom-right
        corners[3] = Point(0, 100); // Bottom-left
    }
    
    Quadrilateral(const Point quad[4]) {
        for (int i = 0; i < 4; i++) {
            corners[i] = quad[i];
        }
    }
    
    Point center() const {
        int centerX = (corners[0].x + corners[1].x + corners[2].x + corners[3].x) / 4;
        int centerY = (corners[0].y + corners[1].y + corners[2].y + corners[3].y) / 4;
        return Point(centerX, centerY);
    }
};

// Geometric utility functions
namespace Geometry {
    // Basic point operations
    Point constrainToBounds(const Point& point, int xMin, int xMax, int yMin, int yMax);
    Point lerp(const Point& start, const Point& end, float t);
    bool isWithinBounds(const Point& point, int xMin, int xMax, int yMin, int yMax);
    
    // Coordinate space transformations
    Point mapToRectangle(const NormalizedPoint& normalized, const Rectangle& rect);
    Point mapToQuadrilateral(const NormalizedPoint& normalized, const Quadrilateral& quad);
    NormalizedPoint mapFromRectangle(const Point& point, const Rectangle& rect);
    
    // Bilinear interpolation for quadrilateral mapping
    Point bilinearInterpolate(float u, float v, const Quadrilateral& quad);
    
    // Distance and angle calculations
    float distance(const Point& a, const Point& b);
    float angle(const Point& from, const Point& to); // Returns angle in radians
    
    // Pattern utility functions
    NormalizedPoint polarToCartesian(float radius, float angle, const NormalizedPoint& center = NormalizedPoint(0.5f, 0.5f));
    void rotatePoint(NormalizedPoint& point, float angle, const NormalizedPoint& pivot = NormalizedPoint(0.5f, 0.5f));
}

#endif // GEOMETRY_H