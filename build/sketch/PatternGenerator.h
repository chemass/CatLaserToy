#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\PatternGenerator.h"
#ifndef PATTERN_GENERATOR_H
#define PATTERN_GENERATOR_H

#include "Geometry.h"
#include <vector>

// Abstract base class for pattern generators
class PatternGenerator {
public:
    virtual ~PatternGenerator() {}
    
    // Generate a sequence of normalized points (0-1 coordinate space)
    // Returns number of points generated
    virtual int generatePattern(NormalizedPoint* points, int maxPoints) = 0;
    
    // Reset pattern to beginning
    virtual void reset() = 0;
    
    // Get pattern name for debugging
    virtual const char* getName() const = 0;
};

// Zigzag pattern generator
class ZigzagPattern : public PatternGenerator {
private:
    int numLines;
    int pointsPerLine;
    bool currentReversed;
    
public:
    ZigzagPattern(int lines = 8, int pointsPerLine = 10) 
        : numLines(lines), pointsPerLine(pointsPerLine), currentReversed(false) {}
    
    int generatePattern(NormalizedPoint* points, int maxPoints) override;
    void reset() override { currentReversed = false; }
    const char* getName() const override { return "Zigzag"; }
    
    // Configuration
    void setLines(int lines) { numLines = lines; }
    void setPointsPerLine(int points) { pointsPerLine = points; }
};

// Spiral pattern generator
class SpiralPattern : public PatternGenerator {
private:
    float maxRadius;
    int totalPoints;
    float spiralTightness;
    
public:
    SpiralPattern(float radius = 0.4f, int points = 100, float tightness = 2.0f)
        : maxRadius(radius), totalPoints(points), spiralTightness(tightness) {}
        
    int generatePattern(NormalizedPoint* points, int maxPoints) override;
    void reset() override {}
    const char* getName() const override { return "Spiral"; }
    
    // Configuration
    void setRadius(float radius) { maxRadius = radius; }
    void setPoints(int points) { totalPoints = points; }
    void setTightness(float tightness) { spiralTightness = tightness; }
};

// Random walk pattern generator
class RandomWalkPattern : public PatternGenerator {
private:
    int totalPoints;
    float stepSize;
    NormalizedPoint currentPos;
    
public:
    RandomWalkPattern(int points = 50, float step = 0.1f)
        : totalPoints(points), stepSize(step), currentPos(0.5f, 0.5f) {}
        
    int generatePattern(NormalizedPoint* points, int maxPoints) override;
    void reset() override { currentPos = NormalizedPoint(0.5f, 0.5f); }
    const char* getName() const override { return "RandomWalk"; }
    
    // Configuration
    void setPoints(int points) { totalPoints = points; }
    void setStepSize(float step) { stepSize = step; }
};

// Figure-8 pattern generator
class Figure8Pattern : public PatternGenerator {
private:
    int totalPoints;
    float scale;
    
public:
    Figure8Pattern(int points = 80, float scale = 0.3f)
        : totalPoints(points), scale(scale) {}
        
    int generatePattern(NormalizedPoint* points, int maxPoints) override;
    void reset() override {}
    const char* getName() const override { return "Figure8"; }
    
    // Configuration
    void setPoints(int points) { totalPoints = points; }
    void setScale(float s) { scale = s; }
};

// Perimeter pattern generator (traces boundary)
class PerimeterPattern : public PatternGenerator {
private:
    int totalPoints;
    
public:
    PerimeterPattern(int points = 40) : totalPoints(points) {}
    
    int generatePattern(NormalizedPoint* points, int maxPoints) override;
    void reset() override {}
    const char* getName() const override { return "Perimeter"; }
    
    // Configuration
    void setPoints(int points) { totalPoints = points; }
};

#endif // PATTERN_GENERATOR_H