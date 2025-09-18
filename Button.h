#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
private:
    uint8_t pin;
    bool lastReading;
    bool state;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;
    bool isPullUp;

public:
    Button(uint8_t buttonPin, bool pullUp = true, unsigned long delay = 50);
    
    void update();
    bool isPressed();
    bool isReleased();
    bool wasPressed();
    bool wasReleased();
    bool getState();
    
    // For long press detection
    bool isLongPressed(unsigned long longPressTime = 1500);
};

#endif