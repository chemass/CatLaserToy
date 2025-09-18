#include "ServoController.h"
#include <Arduino.h>

ServoController::ServoController(int xPin, int yPin, int laserPin) 
    : servoXPin(xPin), servoYPin(yPin), laserPin(laserPin),
      servoXChannel(0), servoYChannel(1),
      currentPosition(90, 90), // Start at midpoint
      minLimits(0, 0), maxLimits(180, 180) {
}

void ServoController::begin() {
    // Set laser pin as output
    pinMode(laserPin, OUTPUT);
    digitalWrite(laserPin, HIGH); // Laser off by default (inverted logic)
    
    // Initialize PWM for servos using ESP32 LEDC
    ledcAttachChannel(servoXPin, pwmFrequency, pwmResolution, servoXChannel);
    ledcAttachChannel(servoYPin, pwmFrequency, pwmResolution, servoYChannel);
    
    // Move to initial position
    moveTo(currentPosition);
    
    Serial.println("ServoController initialized");
    Serial.print("Initial position - X: ");
    Serial.print(currentPosition.x);
    Serial.print(", Y: ");
    Serial.println(currentPosition.y);
}

void ServoController::moveTo(const Point& position) {
    // Constrain to limits
    Point constrainedPos;
    constrainedPos.x = constrainToLimits(position.x, minLimits.x, maxLimits.x);
    constrainedPos.y = constrainToLimits(position.y, minLimits.y, maxLimits.y);
    
    // Move servos
    moveServo(servoXChannel, constrainedPos.x);
    moveServo(servoYChannel, constrainedPos.y);
    
    // Update current position
    updateCurrentPosition(constrainedPos);
}

void ServoController::setLaser(bool enabled) {
    // Note: Inverted logic - HIGH = OFF, LOW = ON
    digitalWrite(laserPin, enabled ? LOW : HIGH);
}

void ServoController::setLimits(int xMin, int xMax, int yMin, int yMax) {
    minLimits.x = xMin;
    minLimits.y = yMin;
    maxLimits.x = xMax;
    maxLimits.y = yMax;
    
    Serial.print("Servo limits set - X: ");
    Serial.print(xMin);
    Serial.print("-");
    Serial.print(xMax);
    Serial.print(", Y: ");
    Serial.print(yMin);
    Serial.print("-");
    Serial.println(yMax);
}

void ServoController::calibrate() {
    Serial.println("Starting servo calibration...");
    
    // Move to corners to verify range
    Point corners[] = {
        {minLimits.x, minLimits.y}, // Min corner
        {maxLimits.x, minLimits.y}, // Max X, Min Y
        {maxLimits.x, maxLimits.y}, // Max corner
        {minLimits.x, maxLimits.y}  // Min X, Max Y
    };
    
    for (int i = 0; i < 4; i++) {
        Serial.print("Moving to corner ");
        Serial.print(i + 1);
        Serial.print(": X=");
        Serial.print(corners[i].x);
        Serial.print(", Y=");
        Serial.println(corners[i].y);
        
        moveTo(corners[i]);
        delay(1000); // Hold position
    }
    
    // Return to center
    Point center((minLimits.x + maxLimits.x) / 2, (minLimits.y + maxLimits.y) / 2);
    moveTo(center);
    
    Serial.println("Calibration complete");
}

void ServoController::adjustX(int delta) {
    Point newPos = currentPosition;
    newPos.x += delta;
    moveTo(newPos);
}

void ServoController::adjustY(int delta) {
    Point newPos = currentPosition;
    newPos.y += delta;
    moveTo(newPos);
}

void ServoController::moveServo(int channel, int angle) {
    angle = constrainToLimits(angle, 0, 180); // Constrain angle to valid servo range
    int dutyCycle = map(angle, 0, 180, pwmMinDuty, pwmMaxDuty);
    ledcWriteChannel(channel, dutyCycle);
}

int ServoController::constrainToLimits(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void ServoController::updateCurrentPosition(const Point& newPos) {
    currentPosition = newPos;
}