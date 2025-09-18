#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\Button.cpp"
#include "Button.h"

Button::Button(uint8_t buttonPin, bool pullUp, unsigned long delay) {
    pin = buttonPin;
    isPullUp = pullUp;
    debounceDelay = delay;
    lastReading = isPullUp;
    state = isPullUp;
    lastState = isPullUp;
    lastDebounceTime = 0;
    
    // Configure pin
    pinMode(pin, isPullUp ? INPUT_PULLUP : INPUT);
}

void Button::update() {
    // Read the current state
    bool reading = digitalRead(pin);
    
    // If the reading changed, reset the debounce timer
    if (reading != lastReading) {
        lastDebounceTime = millis();
    }
    
    // If enough time has passed, update the actual state
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // Update state only if it's different
        if (reading != state) {
            state = reading;
        }
    }
    
    lastReading = reading;
}

bool Button::isPressed() {
    update();
    // For pullup, LOW means pressed
    return isPullUp ? (state == LOW) : (state == HIGH);
}

bool Button::isReleased() {
    update();
    // For pullup, HIGH means released
    return isPullUp ? (state == HIGH) : (state == LOW);
}

bool Button::wasPressed() {
    update();
    bool pressed = isPullUp ? (state == LOW && lastState == HIGH) 
                            : (state == HIGH && lastState == LOW);
    lastState = state;
    return pressed;
}

bool Button::wasReleased() {
    update();
    bool released = isPullUp ? (state == HIGH && lastState == LOW) 
                             : (state == LOW && lastState == HIGH);
    lastState = state;
    return released;
}

bool Button::getState() {
    update();
    return isPullUp ? (state == LOW) : (state == HIGH);
}

bool Button::isLongPressed(unsigned long longPressTime) {
    // If not pressed, return false immediately
    if (!isPressed()) {
        return false;
    }
    
    // Check if button has been pressed long enough
    unsigned long pressStartTime = millis();
    while (isPressed()) {
        if (millis() - pressStartTime > longPressTime) {
            return true;
        }
        delay(10); // Small delay to prevent CPU hogging
    }
    
    return false;
}