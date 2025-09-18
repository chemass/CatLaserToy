#include "MotionPlanner.h"
#include "ServoController.h"
#include <Arduino.h>
#include <cstring>

// MotionSequence implementation
int MotionSequence::generateCommands(
    PatternGenerator& pattern,
    MotionCommand* commands,
    int maxCommands,
    const MotionConfig& cfg
) {
    // Generate normalized pattern points
    const int MAX_PATTERN_POINTS = 200;
    NormalizedPoint patternPoints[MAX_PATTERN_POINTS];
    
    int patternCount = pattern.generatePattern(patternPoints, MAX_PATTERN_POINTS);
    if (patternCount == 0) return 0;
    
    int commandCount = 0;
    
    // Convert pattern points to motion commands
    for (int i = 0; i < patternCount && commandCount < maxCommands; i++) {
        // Map normalized point to boundary
        Point target = Geometry::mapToQuadrilateral(patternPoints[i], boundary);
        
        // Create motion command
        commands[commandCount] = MotionCommand(target, cfg.pointDuration_ms, cfg.laserEnabled);
        commandCount++;
        
        // Add pause if configured
        if (cfg.pauseDuration_ms > 0 && commandCount < maxCommands) {
            commands[commandCount] = MotionCommand(target, cfg.pauseDuration_ms, false);
            commandCount++;
        }
    }
    
    // Return to center if configured
    if (cfg.returnToCenter && commandCount < maxCommands) {
        Point center = boundary.center();
        commands[commandCount] = MotionCommand(center, 200, false);
        commandCount++;
    }
    
    return commandCount;
}

int MotionSequence::generateTransition(
    const Point& start,
    const Point& end,
    MotionCommand* commands,
    int maxCommands,
    unsigned long totalDuration_ms
) {
    if (maxCommands <= 0) return 0;
    
    const int steps = 10; // Number of interpolation steps
    unsigned long stepDuration = totalDuration_ms / steps;
    int commandCount = 0;
    
    for (int i = 0; i <= steps && commandCount < maxCommands; i++) {
        float t = (float)i / (float)steps;
        Point interpolated = Geometry::lerp(start, end, t);
        
        commands[commandCount] = MotionCommand(interpolated, stepDuration, false);
        commandCount++;
    }
    
    return commandCount;
}

// MotionPlanner implementation
MotionPlanner::MotionPlanner() : sequencer(Quadrilateral()), currentPattern(nullptr) {
    // Initialize with default boundary
}

MotionPlanner::~MotionPlanner() {
    // Patterns are stack allocated, no cleanup needed
}

int MotionPlanner::executePattern(
    PatternGenerator& pattern,
    const Quadrilateral& boundary,
    MotionCommand* commands,
    int maxCommands,
    const MotionConfig& config
) {
    sequencer.setBoundary(boundary);
    return sequencer.generateCommands(pattern, commands, maxCommands, config);
}

// Legacy compatibility method (enhanced to use new architecture)
void MotionPlanner::generateZigzagPattern(
    const Point boundary[4], 
    MotionCommand* commands,
    int& commandCount,
    int maxCommands,
    int numZigzags, 
    int pointsPerLine,
    unsigned long pointDuration
) {
    // Configure zigzag pattern
    zigzagPattern.setLines(numZigzags);
    zigzagPattern.setPointsPerLine(pointsPerLine);
    
    // Configure motion
    MotionConfig config;
    config.pointDuration_ms = pointDuration;
    config.laserEnabled = true;
    config.returnToCenter = true;
    
    // Set boundary
    Quadrilateral quad(boundary);
    
    // Generate commands
    commandCount = executePattern(zigzagPattern, quad, commands, maxCommands, config);
}

void MotionPlanner::generateSmoothPath(
    const Point& start, 
    const Point& end, 
    MotionCommand* commands,
    int& commandCount,
    int maxCommands,
    unsigned long totalDuration,
    int steps
) {
    commandCount = 0;
    unsigned long stepDuration = totalDuration / steps;
    
    for (int i = 0; i <= steps && commandCount < maxCommands; i++) {
        float t = (float)i / (float)steps;
        
        Point interpolated;
        interpolated.x = start.x + (int)((end.x - start.x) * t);
        interpolated.y = start.y + (int)((end.y - start.y) * t);
        
        commands[commandCount] = MotionCommand(interpolated, stepDuration, true);
        commandCount++;
    }
}

Point MotionPlanner::mapToBoundary(float u, float v, const Point boundary[4]) {
    return bilinearInterpolate(u, v, boundary);
}

Point MotionPlanner::bilinearInterpolate(float u, float v, const Point corners[4]) {
    float invU = 1.0 - u;
    float invV = 1.0 - v;
    
    Point result;
    
    // Bilinear interpolation formula
    // Assumes corners are in order: [0]=top-left, [1]=top-right, [2]=bottom-right, [3]=bottom-left
    result.x = (int)(corners[0].x * invU * invV +
                     corners[1].x * u * invV +
                     corners[2].x * u * v +
                     corners[3].x * invU * v);
                
    result.y = (int)(corners[0].y * invU * invV +
                     corners[1].y * u * invV +
                     corners[2].y * u * v +
                     corners[3].y * invU * v);
    
    return result;
}