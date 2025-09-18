#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include "Geometry.h"

#ifdef ARDUINO
#include "esp32-hal-ledc.h"
#endif

class ServoController {
public:
    ServoController(int xPin, int yPin, int laserPin);
    
    void begin();
    void moveTo(const Point& position);
    void setLaser(bool enabled);
    
    Point getCurrentPosition() const { return currentPosition; }
    
    // Hardware limits and calibration
    void setLimits(int xMin, int xMax, int yMin, int yMax);
    void calibrate(); // Move to known positions for calibration
    
    // Direct servo control (for manual adjustments)
    void adjustX(int delta);
    void adjustY(int delta);
    
private:
    // Hardware pins
    int servoXPin, servoYPin, laserPin;
    
    // PWM channels
    int servoXChannel, servoYChannel;
    
    // Current state
    Point currentPosition;
    Point minLimits, maxLimits;
    
    // PWM parameters (from original code)
    static const int pwmFrequency = 50; // 50Hz for servos
    static const int pwmResolution = 16; // 16-bit resolution
    static const int pwmMinDuty = 3277; // For ~1ms pulse with 16-bit res at 50Hz
    static const int pwmMaxDuty = 6554; // For ~2ms pulse with 16-bit res at 50Hz
    
    void moveServo(int channel, int angle);
    int constrainToLimits(int value, int min, int max);
    void updateCurrentPosition(const Point& newPos);
};

#endif // SERVO_CONTROLLER_H