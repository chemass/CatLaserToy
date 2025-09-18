// MockServoController.h - Mock implementation for testing
#ifndef MOCK_SERVO_CONTROLLER_H
#define MOCK_SERVO_CONTROLLER_H

#include <vector>
#include <string>
#include "../Geometry.h"

// Mock servo controller that logs commands instead of controlling hardware
// Matches ServoController interface for testing
class MockServoController {
public:
    struct LoggedCommand {
        std::string command;
        float value1;
        float value2;
        unsigned long timestamp;
    };

    MockServoController() : laser_on_(false), current_x_(90), current_y_(90) {}
    MockServoController(int xPin, int yPin, int laserPin) : laser_on_(false), current_x_(90), current_y_(90) {}

    void begin() {
        log_command("begin", 0, 0);
    }

    void moveTo(const Point& position) {
        current_x_ = position.x;
        current_y_ = position.y;
        log_command("moveTo", position.x, position.y);
    }

    void setLaser(bool enabled) {
        laser_on_ = enabled;
        log_command("setLaser", enabled ? 1.0f : 0.0f, 0);
    }

    Point getCurrentPosition() const {
        return Point(current_x_, current_y_);
    }

    // Hardware limits and calibration (mock implementations)
    void setLimits(int xMin, int xMax, int yMin, int yMax) {
        log_command("setLimits", (float)xMin, (float)xMax);
    }

    void calibrate() {
        log_command("calibrate", 0, 0);
    }

    // Direct servo control (mock implementations)
    void adjustX(int delta) {
        current_x_ += delta;
        log_command("adjustX", (float)delta, 0);
    }

    void adjustY(int delta) {
        current_y_ += delta;
        log_command("adjustY", 0, (float)delta);
    }

    // Test helper methods
    const std::vector<LoggedCommand>& getCommandLog() const { return command_log_; }

    void clearLog() { command_log_.clear(); }

    size_t getCommandCount() const { return command_log_.size(); }

    LoggedCommand getLastCommand() const {
        return command_log_.empty() ? LoggedCommand{} : command_log_.back();
    }

    bool hasCommand(const std::string& cmd) const {
        for (const auto& logged : command_log_) {
            if (logged.command == cmd) return true;
        }
        return false;
    }

    float getCurrentX() const { return current_x_; }
    float getCurrentY() const { return current_y_; }
    bool isLaserOn() const { return laser_on_; }

private:
    bool laser_on_;
    float current_x_;
    float current_y_;
    std::vector<LoggedCommand> command_log_;

    void log_command(const std::string& cmd, float val1, float val2) {
        command_log_.push_back({cmd, val1, val2, 0}); // timestamp not critical for tests
    }
};

#endif // MOCK_SERVO_CONTROLLER_H