#ifndef MOTION_PLANNER_H
#define MOTION_PLANNER_H

#include "Geometry.h"
#include "PatternGenerator.h"

struct MotionCommand {
    Point target;
    unsigned long duration_ms;
    bool laser_on;
    
    // Computed properties that read from target
    float targetX() const { return target.x; }
    float targetY() const { return target.y; }
    
    // Setters that update target
    void targetX(float value) { target.x = value; }
    void targetY(float value) { target.y = value; }
    
    MotionCommand() : target(), duration_ms(50), laser_on(false) {}
    MotionCommand(Point targetPoint, unsigned long durationMs, bool laserOn) : target(targetPoint), duration_ms(durationMs), laser_on(laserOn) {}
    MotionCommand(float x, float y, unsigned long durationMs, bool laserOn) : target(x, y), duration_ms(durationMs), laser_on(laserOn) {}
};

// Configuration for motion sequences
struct MotionConfig {
    unsigned long pointDuration_ms = 50;
    unsigned long pauseDuration_ms = 0;
    bool laserEnabled = true;
    bool returnToCenter = false;
    
    MotionConfig() = default;
};

// Motion sequence generator - converts patterns to commands
class MotionSequence {
private:
    Quadrilateral boundary;
    
public:
    MotionSequence() : boundary() {}
    MotionSequence(const Quadrilateral& bound) : boundary(bound) {}
    
    void setBoundary(const Quadrilateral& bound) { boundary = bound; }
    
    int generateCommands(
        PatternGenerator& pattern,
        MotionCommand* commands,
        int maxCommands,
        const MotionConfig& config = MotionConfig()
    );
    
    int generateTransition(
        const Point& start,
        const Point& end,
        MotionCommand* commands,
        int maxCommands,
        unsigned long totalDuration_ms = 500
    );
};

// Motion planner - high-level interface for motion sequences
class MotionPlanner {
private:
    MotionSequence sequencer;
    PatternGenerator* currentPattern;
    
    // Built-in pattern generators
    ZigzagPattern zigzagPattern;
    SpiralPattern spiralPattern;
    RandomWalkPattern randomWalkPattern;
    Figure8Pattern figure8Pattern;
    PerimeterPattern perimeterPattern;
    
public:
    MotionPlanner();
    ~MotionPlanner();
    
    // Pattern execution
    int executePattern(
        PatternGenerator& pattern,
        const Quadrilateral& boundary,
        MotionCommand* commands,
        int maxCommands,
        const MotionConfig& config = MotionConfig()
    );
    
    // Legacy compatibility for existing code
    void generateZigzagPattern(
        const Point boundary[4], 
        MotionCommand* commands,
        int& commandCount,
        int maxCommands,
        int numZigzags = 8, 
        int pointsPerLine = 10,
        unsigned long pointDuration = 50
    );
    
    // Generate smooth interpolated path between two points
    void generateSmoothPath(
        const Point& start, 
        const Point& end, 
        MotionCommand* commands,
        int& commandCount,
        int maxCommands,
        unsigned long totalDuration = 1000,
        int steps = 20
    );
    
    // Map normalized coordinates to boundary quadrilateral
    Point mapToBoundary(float u, float v, const Point boundary[4]);
    
    // Direct pattern access
    ZigzagPattern& getZigzagPattern() { return zigzagPattern; }
    SpiralPattern& getSpiralPattern() { return spiralPattern; }
    RandomWalkPattern& getRandomWalkPattern() { return randomWalkPattern; }
    Figure8Pattern& getFigure8Pattern() { return figure8Pattern; }
    PerimeterPattern& getPerimeterPattern() { return perimeterPattern; }
    
private:
    Point bilinearInterpolate(float u, float v, const Point corners[4]);
};

#endif // MOTION_PLANNER_H