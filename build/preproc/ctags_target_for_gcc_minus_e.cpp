# 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino"
// Define pins for servos, laser, button, and encoders
const int servoXPin = 16; // Servo for X-axis
const int servoYPin = 17; // Servo for Y-axis
const int laserPin = 4; // Laser control pin
const int buttonPin = 18; // Button pin
const int encoderXPinA = 25; // Rotary encoder X-axis pin A
const int encoderXPinB = 26; // Rotary encoder X-axis pin B
const int encoderXButton = 27; // Rotary encoder X-axis button
const int encoderYPinA = 32; // Rotary encoder Y-axis pin A
const int encoderYPinB = 33; // Rotary encoder Y-axis pin B
const int encoderYButton = 34; // Rotary encoder Y-axis button

// Laser state
bool laserActive = false;

// Timer for auto-disable
unsigned long laserStartTime = 0;

// Travel limits
int xMin = 0, xMax = 90;
int yMin = 0, yMax = 90;

// Current positions
int currentX = 90; // Start at midpoint
int currentY = 90; // Start at midpoint

// Servo PWM parameters
const int pwmFrequency = 50;
const int pwmResolution = 16;
const int pwmMinDuty = 3277; // For ~1ms pulse with 16-bit res at 50Hz
const int pwmMaxDuty = 6554; // For ~2ms pulse with 16-bit res at 50Hz

void setup() {
    // Initialize serial communication
    Serial0.begin(115200);

    // Set laser, button, and encoder pins
    pinMode(laserPin, 0x03);
    pinMode(buttonPin, 0x05);
    pinMode(encoderXPinA, 0x05);
    pinMode(encoderXPinB, 0x05);
    pinMode(encoderXButton, 0x05);
    pinMode(encoderYPinA, 0x05);
    pinMode(encoderYPinB, 0x05);
    pinMode(encoderYButton, 0x05);

    digitalWrite(laserPin, 0x1); // Laser off by default

    // Initialize PWM for servos
    analogWriteFrequency(servoXPin, pwmFrequency);
    analogWriteFrequency(servoYPin, pwmFrequency);
    analogWriteResolution(servoXPin, pwmResolution); // Should set 16-bit for Dlloydev lib
    analogWriteResolution(servoYPin, pwmResolution); // Should set 16-bit for Dlloydev lib

    Serial0.println("Testing servo X to 0 deg (1ms pulse expectation)");
    moveServo(servoXPin, 0); // Should use pwmMinDuty (3277)
    delay(2000);

    Serial0.println("Testing servo X to 90 deg (1.5ms pulse expectation)");
    moveServo(servoXPin, 90); // Should use midpoint duty
    delay(2000);

    Serial0.println("Testing servo X to 180 deg (2ms pulse expectation)");
    moveServo(servoXPin, 180); // Should use pwmMaxDuty (6554)
    delay(2000);

    // Move servos to initial positions
    moveServo(servoXPin, currentX);
    moveServo(servoYPin, currentY);

    // Initial status report
    Serial0.println("System initialized.");
    Serial0.print("Current X: ");
    Serial0.println(currentX);
    Serial0.print("Current Y: ");
    Serial0.println(currentY);
}

void loop() {
    // Check for button press to toggle laser state
    static bool lastButtonState = 0x1;
    bool currentButtonState = digitalRead(buttonPin);

    if (lastButtonState == 0x1 && currentButtonState == 0x0) {
        laserActive = !laserActive;
        if (laserActive) {
            laserStartTime = millis(); // Record start time
            Serial0.println("Laser activated.");
        } else {
            Serial0.println("Laser deactivated.");
        }
    }
    lastButtonState = currentButtonState;

    // If laser is active, allow manual control and check for auto-disable
    if (laserActive) {
        digitalWrite(laserPin, 0x0); // Turn on laser

        // Handle X-axis encoder
        static int lastXState = 0x1;
        int xState = digitalRead(encoderXPinA);
        if (xState != lastXState && xState == 0x0) {
            if (digitalRead(encoderXPinB) == 0x0) {
                currentX = max(xMin, currentX - 1); // Decrease position
            } else {
                currentX = min(xMax, currentX + 1); // Increase position
            }
            moveServo(servoXPin, currentX);
            Serial0.print("X-axis moved to: ");
            Serial0.println(currentX);
        }
        lastXState = xState;

        // Handle Y-axis encoder
        static int lastYState = 0x1;
        int yState = digitalRead(encoderYPinA);
        if (yState != lastYState && yState == 0x0) {
            if (digitalRead(encoderYPinB) == 0x0) {
                currentY = max(yMin, currentY - 5); // Decrease position
            } else {
                currentY = min(yMax, currentY + 5); // Increase position
            }
            moveServo(servoYPin, currentY);
            Serial0.print("Y-axis moved to: ");
            Serial0.println(currentY);
        }
        lastYState = yState;

        // Handle X-axis encoder button for setting limits
        static bool lastXButtonState = 0x1;
        bool xButtonState = digitalRead(encoderXButton);
        if (lastXButtonState == 0x1 && xButtonState == 0x0) {
            if (currentX < (xMax + xMin) / 2) {
                xMin = currentX; // Set minimum limit
                Serial0.print("X-axis minimum limit set to: ");
                Serial0.println(xMin);
            } else {
                xMax = currentX; // Set maximum limit
                Serial0.print("X-axis maximum limit set to: ");
                Serial0.println(xMax);
            }
        }
        lastXButtonState = xButtonState;

        // Handle Y-axis encoder button for setting limits
        static bool lastYButtonState = 0x1;
        bool yButtonState = digitalRead(encoderYButton);
        if (lastYButtonState == 0x1 && yButtonState == 0x0) {
            if (currentY < (yMax + yMin) / 2) {
                yMin = currentY; // Set minimum limit
                Serial0.print("Y-axis minimum limit set to: ");
                Serial0.println(yMin);
            } else {
                yMax = currentY; // Set maximum limit
                Serial0.print("Y-axis maximum limit set to: ");
                Serial0.println(yMax);
            }
        }
        lastYButtonState = yButtonState;

        // Auto-disable after 5 minutes (300,000 ms)
        if (millis() - laserStartTime > 300000) {
            laserActive = false;
            digitalWrite(laserPin, 0x1); // Turn off laser
            Serial0.println("Laser auto-disabled after timeout.");
        }
    } else {
        digitalWrite(laserPin, 0x1); // Turn off laser
    }
}

// Function to move a servo to a specific angle
void moveServo(int pin, int angle) {
    angle = ((angle) < (0) ? (0) : ((angle) > (180) ? (180) : (angle))); // Constrain angle to valid range
    int dutyCycle = map(angle, 0, 180, pwmMinDuty, pwmMaxDuty);
    Serial0.print("Setting duty cycle to: ");
    Serial0.println(dutyCycle);
    analogWrite(pin, dutyCycle);
}
