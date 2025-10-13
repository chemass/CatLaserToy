// MockServoController.h - Mock implementation for testing
#ifndef MOCK_SERVO_CONTROLLER_H
#define MOCK_SERVO_CONTROLLER_H

#include "../Geometry.h"

// Mock implementation of ServoController for testing
class MockServoController {
private:
    bool laser_on_;
    int current_x_, current_y_;
    int x_min_, x_max_, y_min_, y_max_;

public:
    MockServoController(int /* xPin */, int /* yPin */, int /* laserPin */) 
        : laser_on_(false), current_x_(90), current_y_(90), 
          x_min_(0), x_max_(180), y_min_(0), y_max_(180) {}
    
    void begin() {}
    
    void moveTo(const Point& point) {
        current_x_ = static_cast<int>(point.x);
        current_y_ = static_cast<int>(point.y);
        
        // Enforce limits
        if (current_x_ < x_min_) current_x_ = x_min_;
        if (current_x_ > x_max_) current_x_ = x_max_;
        if (current_y_ < y_min_) current_y_ = y_min_;
        if (current_y_ > y_max_) current_y_ = y_max_;
    }
    
    void setLaser(bool on) {
        laser_on_ = on;
    }
    
    Point getCurrentPosition() const {
        return Point(static_cast<float>(current_x_), static_cast<float>(current_y_));
    }
    
    bool isLaserOn() const {
        return laser_on_;
    }
    
    void setLimits(int xMin, int xMax, int /* yMin */, int /* yMax */) {
        x_min_ = xMin;
        x_max_ = xMax;
        // Suppressing unused parameter warnings by commenting them out
    }
};

// Global mock instance for testing
extern MockServoController servoController;

#endif