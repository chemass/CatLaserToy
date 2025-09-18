#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\PatternGenerator.cpp"
#include "PatternGenerator.h"
#include <cstdlib>
#include <cmath>

// Zigzag pattern implementation
int ZigzagPattern::generatePattern(NormalizedPoint* points, int maxPoints) {
    int pointCount = 0;
    
    for (int line = 0; line < numLines && pointCount < maxPoints; line++) {
        float v = (float)line / (float)(numLines - 1); // Vertical position
        
        // Alternate direction each line
        bool rightToLeft = (line % 2 == 1);
        
        for (int point = 0; point < pointsPerLine && pointCount < maxPoints; point++) {
            float u;
            if (rightToLeft) {
                u = 1.0f - ((float)point / (float)(pointsPerLine - 1));
            } else {
                u = (float)point / (float)(pointsPerLine - 1);
            }
            
            points[pointCount] = NormalizedPoint(u, v);
            pointCount++;
        }
    }
    
    return pointCount;
}

// Spiral pattern implementation
int SpiralPattern::generatePattern(NormalizedPoint* points, int maxPoints) {
    int pointCount = 0;
    const float PI = 3.14159265359f;
    
    for (int i = 0; i < totalPoints && pointCount < maxPoints; i++) {
        float t = (float)i / (float)(totalPoints - 1);
        float angle = t * spiralTightness * 2.0f * PI;
        float radius = t * maxRadius;
        
        points[pointCount] = Geometry::polarToCartesian(radius, angle);
        pointCount++;
    }
    
    return pointCount;
}

// Random walk pattern implementation
int RandomWalkPattern::generatePattern(NormalizedPoint* points, int maxPoints) {
    int pointCount = 0;
    currentPos = NormalizedPoint(0.5f, 0.5f); // Start at center
    
    for (int i = 0; i < totalPoints && pointCount < maxPoints; i++) {
        points[pointCount] = currentPos;
        pointCount++;
        
        // Random step
        float angle = ((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159265359f;
        float stepX = cos(angle) * stepSize;
        float stepY = sin(angle) * stepSize;
        
        currentPos.u += stepX;
        currentPos.v += stepY;
        currentPos.clamp();
    }
    
    return pointCount;
}

// Figure-8 pattern implementation
int Figure8Pattern::generatePattern(NormalizedPoint* points, int maxPoints) {
    int pointCount = 0;
    const float PI = 3.14159265359f;
    
    for (int i = 0; i < totalPoints && pointCount < maxPoints; i++) {
        float t = ((float)i / (float)(totalPoints - 1)) * 2.0f * PI;
        
        // Parametric equations for figure-8 (lemniscate)
        float denominator = 1.0f + sin(t) * sin(t);
        float x = (cos(t) / denominator) * scale;
        float y = (sin(t) * cos(t) / denominator) * scale;
        
        points[pointCount] = NormalizedPoint(0.5f + x, 0.5f + y);
        points[pointCount].clamp();
        pointCount++;
    }
    
    return pointCount;
}

// Perimeter pattern implementation  
int PerimeterPattern::generatePattern(NormalizedPoint* points, int maxPoints) {
    int pointCount = 0;
    
    if (totalPoints <= 0) return 0;
    
    // Divide points among 4 sides
    int pointsPerSide = totalPoints / 4;
    int remainder = totalPoints % 4;
    
    // Top edge (left to right)
    for (int i = 0; i < pointsPerSide && pointCount < maxPoints; i++) {
        float u = (float)i / (float)(pointsPerSide - 1);
        points[pointCount] = NormalizedPoint(u, 0.0f);
        pointCount++;
    }
    
    // Right edge (top to bottom)
    for (int i = 0; i < pointsPerSide && pointCount < maxPoints; i++) {
        float v = (float)i / (float)(pointsPerSide - 1);
        points[pointCount] = NormalizedPoint(1.0f, v);
        pointCount++;
    }
    
    // Bottom edge (right to left)
    for (int i = 0; i < pointsPerSide && pointCount < maxPoints; i++) {
        float u = 1.0f - ((float)i / (float)(pointsPerSide - 1));
        points[pointCount] = NormalizedPoint(u, 1.0f);
        pointCount++;
    }
    
    // Left edge (bottom to top)
    for (int i = 0; i < pointsPerSide && pointCount < maxPoints; i++) {
        float v = 1.0f - ((float)i / (float)(pointsPerSide - 1));
        points[pointCount] = NormalizedPoint(0.0f, v);
        pointCount++;
    }
    
    // Add remainder points to first side
    for (int i = 0; i < remainder && pointCount < maxPoints; i++) {
        float u = (float)i / (float)remainder;
        points[pointCount] = NormalizedPoint(u, 0.0f);
        pointCount++;
    }
    
    return pointCount;
}