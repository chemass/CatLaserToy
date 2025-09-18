// MockServoController.h - Mock implementation for testing
#ifndef MOCK_SERVO_CONTROLLER_H
#define MOCK_SERVO_CONTROLLER_H

#include <vector>
#include <string>

// Mock servo controller that logs commands instead of controlling hardware
class MockServoController {
public:
    struct LoggedCommand {
        std::string command;
        float value1;
        float value2;
        unsigned long timestamp;
    };
    
    MockServoController() : laser_on_(false), current_x_(90), current_y_(90) {}
    
    void begin() {
        log_command("begin", 0, 0);
    }
    
    void setPosition(float x, float y) {
        current_x_ = x;
        current_y_ = y;
        log_command("setPosition", x, y);
    }
    
    void setLaser(bool on) {
        laser_on_ = on;
        log_command("setLaser", on ? 1.0f : 0.0f, 0);
    }
    
    float getCurrentX() const { return current_x_; }
    float getCurrentY() const { return current_y_; }
    bool isLaserOn() const { return laser_on_; }
    
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