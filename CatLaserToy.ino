#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#include "esp32-hal-ledc.h"
#include "Button.h"
#include "Geometry.h"
#include "StorageManager.h"
#include "MotionPlanner.h"
#include "MotionExecutor.h"
#include "PatternGenerator.h"
#include "ServoController.h"
#include "WebInterface.h"
#include <WiFi.h>

const char* ssid = "WeLoveBella";
const char* password = "BellaIsTheBest";
const char* hostname = "cattoy"; // mDNS hostname - access via http://cattoy.local

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Setup mode state for webpage
float lastPointerX = 0.5, lastPointerY = 0.5;

// Create storage manager instance
StorageManager storage;

// Create motion control system
ServoController servoController(16, 17, 4); // X pin, Y pin, laser pin
MotionPlanner motionPlanner;
MotionExecutor motionExecutor(servoController);

// Define pins for button (servo pins now handled by ServoController)
const int buttonPin = 18; // Main button pin
const int ledPin = 2;     // ESP32 internal LED

// Create button objects
Button mainButton(buttonPin);

// Laser state
bool laserActive = false;

// Timer for auto-disable
unsigned long laserStartTime = 0;

// Travel limits
int xMin = 0, xMax = 180;
int yMin = 0, yMax = 180;

// Current positions (now managed by ServoController, but kept for encoder tracking)
int currentX = 90; // Start at midpoint
int currentY = 90; // Start at midpoint

bool isZigging = false;

// Point storage
Point storedPoints[StorageManager::MAX_POINTS];
int currentPointIndex = 0;
bool inPointStorageMode = false;
unsigned long lastLedToggleTime = 0;
bool ledState = false;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);  
    
    // Initialize EEPROM
    storage.begin();

    // Initialize servo controller
    servoController.begin();
    servoController.setLimits(xMin, xMax, yMin, yMax);

    // Set LED pin
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);    // LED off by default
    
    // Load stored points from EEPROM
    if (!storage.loadPoints(storedPoints)) {
        Serial.println("No stored points found. Entering setup mode.");
        inPointStorageMode = true;
        currentPointIndex = 0;
    } else {
        Serial.println("Stored points loaded from EEPROM:");
        for (int i = 0; i < StorageManager::MAX_POINTS; i++) {
            Serial.print("Point ");
            Serial.print(i + 1);
            Serial.print(": X=");
            Serial.print(storedPoints[i].x);
            Serial.print(", Y=");
            Serial.println(storedPoints[i].y);
        }
    }

    // Move servos to initial positions using ServoController
    Point initialPos(currentX, currentY);
    servoController.moveTo(initialPos);

    // Setup WiFi
    WiFi.setHostname(hostname);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }
    
    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Setup mDNS
    if (!MDNS.begin(hostname)) {
        Serial.println("Error setting up mDNS responder!");
    } else {
        Serial.printf("mDNS responder started. Access via: http://%s.local\n", hostname);
        // Add service to mDNS-SD
        MDNS.addService("http", "tcp", 80);
    }

    // Configure WebSocket for stability
    ws.onEvent(onWsEvent);
    ws.setAuthentication("", ""); // No authentication
    ws.enable(true);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.begin();
}

// Forward declarations for functions used in onWsEvent
void sendCurrentPosition(AsyncWebSocketClient *client);
void notifySetupMode(AsyncWebSocketClient *client);
void enterPointStorageMode();
void performZigzagPattern();
void performSpiralPattern();
void performRandomPattern();
void performFigure8Pattern();
void performPerimeterPattern();

// Helper function to enable laser before movement if not already enabled
void enableLaserForMovement() {
    if (!laserActive) {
        laserActive = true;
        laserStartTime = millis();
        servoController.setLaser(true);
        Serial.println("Laser auto-enabled for movement.");
        
        // Notify all WebSocket clients about laser state change
        ws.textAll("{\"type\":\"laser-state\",\"active\":true}");
    }
}

// Helper function to broadcast current laser position to all clients
void broadcastCurrentPosition() {
    Point currentPos = servoController.getCurrentPosition();
    float x, y;
    
    if (inPointStorageMode) {
        // In setup mode, use last pointer position
        x = lastPointerX;
        y = lastPointerY;
    } else {
        // In normal mode, convert current servo position back to normalized coordinates
        if (storage.loadPoints(storedPoints)) {
            // Try to reverse map from boundary to normalized coordinates (approximate)
            x = (float)currentPos.x / 180.0;
            y = (float)currentPos.y / 180.0;
        } else {
            // No boundary set, use direct servo mapping
            x = (float)currentPos.x / 180.0;
            y = (float)currentPos.y / 180.0;
        }
    }
    
    // Broadcast position to all clients
    String positionMsg = "{\"type\":\"position\",\"x\":" + String(x, 3) + ",\"y\":" + String(y, 3) + "}";
    ws.textAll(positionMsg);
}

// Helper function to broadcast laser state to all clients
void broadcastLaserState() {
    String laserMsg = "{\"type\":\"laser-state\",\"active\":" + String(laserActive ? "true" : "false") + "}";
    ws.textAll(laserMsg);
}

// Helper to parse x/y from JSON string
bool parseXYFromJson(const String& msg, float& x, float& y) {
    int xIdx = msg.indexOf("\"x\"");
    int yIdx = msg.indexOf("\"y\"");
    if (xIdx != -1 && yIdx != -1) {
        int xColon = msg.indexOf(":", xIdx);
        int xEnd = msg.indexOf(",", xColon);
        if (xEnd == -1) xEnd = msg.indexOf("}", xColon);
        x = msg.substring(xColon + 1, xEnd).toFloat();
        int yColon = msg.indexOf(":", yIdx);
        int yEnd = msg.indexOf(",", yColon);
        if (yEnd == -1) yEnd = msg.indexOf("}", yColon);
        y = msg.substring(yColon + 1, yEnd).toFloat();
        // Clamp
        if (x < 0) x = 0; if (x > 1) x = 1;
        if (y < 0) y = 0; if (y > 1) y = 1;
        return true;
    }
    return false;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.print("WebSocket client connected from IP: ");
        Serial.println(client->remoteIP());
        notifySetupMode(client);
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.print("WebSocket client disconnected from IP: ");
        Serial.println(client->remoteIP());
    } else if (type == WS_EVT_ERROR) {
        Serial.print("WebSocket error: ");
        Serial.println((char*)data);
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len) {
            data[len] = 0;
            String msg = (char *)data;
            Serial.print("WS received: ");
            Serial.println(msg);

            // Try to parse as JSON for all commands
            if (msg.startsWith("{")) {
                // Simple JSON parsing for "type"
                int typeIdx = msg.indexOf("\"type\"");
                if (typeIdx >= 0) {
                    int colonIdx = msg.indexOf(":", typeIdx);
                    int quote1 = msg.indexOf('"', colonIdx);
                    int quote2 = msg.indexOf('"', quote1 + 1);
                    String typeVal = msg.substring(quote1 + 1, quote2);

                    // Handle move command: {"type":"move","x":...,"y":...}
                    if (typeVal == "move") {
                        float x, y;
                        if (parseXYFromJson(msg, x, y)) {
                            // Enable laser automatically for movement
                            enableLaserForMovement();
                            
                            float y_swapped = 1.0f - y;
                            Point mapped;
                            if (storage.loadPoints(storedPoints)) {
                                Quadrilateral boundary(storedPoints);
                                NormalizedPoint normPos(x, y_swapped);
                                mapped = Geometry::mapToQuadrilateral(normPos, boundary);
                            } else {
                                mapped = Point((int)(x * 180), (int)(y_swapped * 180));
                            }
                            motionExecutor.moveImmediate(mapped, laserActive);
                            lastPointerX = x;
                            lastPointerY = y_swapped;
                            
                            // Broadcast position update to all clients
                            broadcastCurrentPosition();
                        }
                        return;
                    } else if (typeVal == "setup-move") {
                        float x, y;
                        if (parseXYFromJson(msg, x, y)) {
                            float y_swapped = 1.0f - y;
                            Point mapped = Point((int)(x * 180), (int)(y_swapped * 180));
                            servoController.moveTo(mapped);
                            lastPointerX = x;
                            lastPointerY = y_swapped;
                            
                            // Broadcast position update to all clients
                            broadcastCurrentPosition();
                        }
                        return;
                    } else if (typeVal == "laser-toggle" || typeVal == "toggle-laser") {
                        laserActive = !laserActive;
                        servoController.setLaser(laserActive);
                        if (laserActive) {
                            laserStartTime = millis();
                        }
                        
                        // Broadcast laser state to all clients
                        broadcastLaserState();
                        return;
                    } else if (typeVal == "enter-setup") {
                        enterPointStorageMode();
                        client->text("{\"setup\":true}");
                        return;
                    } else if (typeVal == "store-point") {
                        if (inPointStorageMode && currentPointIndex < StorageManager::MAX_POINTS) {
                            storedPoints[currentPointIndex] = servoController.getCurrentPosition();
                            currentPointIndex++;
                            if (currentPointIndex >= StorageManager::MAX_POINTS) {
                                storage.savePoints(storedPoints);
                                inPointStorageMode = false;
                                client->text("{\"setup\":false}");
                            } else {
                                client->text("{\"storeNext\":" + String(currentPointIndex) + "}");
                            }
                        }
                        return;
                    } else if (typeVal == "move-up") {
                        enableLaserForMovement();
                        servoController.adjustY(-2);
                        broadcastCurrentPosition();
                        return;
                    } else if (typeVal == "move-down") {
                        enableLaserForMovement();
                        servoController.adjustY(2);
                        broadcastCurrentPosition();
                        return;
                    } else if (typeVal == "move-left") {
                        enableLaserForMovement();
                        servoController.adjustX(-2);
                        broadcastCurrentPosition();
                        return;
                    } else if (typeVal == "move-right") {
                        enableLaserForMovement();
                        servoController.adjustX(2);
                        broadcastCurrentPosition();
                        return;
                    } else if (typeVal == "stop-motion") {
                        motionExecutor.stop();
                        client->text("{\"motionStopped\":true}");
                        return;
                    } else if (typeVal == "get-position") {
                        sendCurrentPosition(client);
                        return;
                    }

                    // Pattern commands (require boundary and not busy)
                    if (typeVal.endsWith("-pattern")) {
                        // Only allow patterns if boundary is set
                        bool boundaryReady = storage.loadPoints(storedPoints);
                        if (!boundaryReady) {
                            client->text("{\"error\":\"Boundary not configured\"}");
                            return;
                        }
                        // Only allow one pattern at a time
                        if (motionExecutor.isBusy()) {
                            client->text("{\"error\":\"Pattern already running\"}");
                            return;
                        }
                        PatternGenerator* pattern = nullptr;
                        if (typeVal == "zigzag-pattern") {
                            pattern = new ZigzagPattern(8, 10);
                        } else if (typeVal == "spiral-pattern") {
                            pattern = new SpiralPattern(0.45f, 80, 1.0f);
                        } else if (typeVal == "random-pattern") {
                            pattern = new RandomWalkPattern(80, 0.08f);
                        } else if (typeVal == "figure8-pattern") {
                            pattern = new Figure8Pattern(80, 0.4f);
                        } else if (typeVal == "perimeter-pattern") {
                            pattern = new PerimeterPattern(80);
                        } else if (typeVal == "stop-pattern") {
                            motionExecutor.stop();
                            client->text("{\"pattern-stopped\":true}");
                            return;
                        }
                        if (pattern) {
                            // Enable laser automatically for pattern movement
                            enableLaserForMovement();
                            
                            MotionCommand commands[200];
                            MotionConfig config;
                            config.pointDuration_ms = 50;
                            config.laserEnabled = true;
                            config.returnToCenter = false;
                            Quadrilateral quad(storedPoints);
                            int cmdCount = motionPlanner.executePattern(*pattern, quad, commands, 200, config);
                            motionExecutor.queueCommands(commands, cmdCount);
                            client->text("{\"pattern-started\":\"" + typeVal + "\"}");
                            delete pattern;
                            return;
                        }
                    }
                }
            }
        }
    } else if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
        sendCurrentPosition(client);
        notifySetupMode(client);
    }
} // End of onWsEvent

// Send current position to client
void sendCurrentPosition(AsyncWebSocketClient *client) {
    float x, y;
    
    if (inPointStorageMode) {
        // In setup mode, use last pointer position or default center
        x = lastPointerX;
        y = lastPointerY;
    } else {
        // In normal mode, convert current servo position back to normalized coordinates
        // If we have stored points, reverse map from servo to boundary coordinates
        if (currentPointIndex >= StorageManager::MAX_POINTS) {
            // Try to reverse map the current servo position to normalized coordinates
            // This is approximate since reverse mapping is complex
            x = (float)currentX / 180.0;
            y = (float)currentY / 180.0;
        } else {
            // No boundary set, use direct servo mapping
            x = (float)currentX / 180.0;
            y = (float)currentY / 180.0;
        }
    }
    
    // Send position as JSON
    String positionMsg = "{\"type\":\"position\",\"x\":" + String(x, 3) + ",\"y\":" + String(y, 3) + "}";
    client->text(positionMsg);
    Serial.println("Sent position: " + positionMsg);
}

// Notify clients of setup mode on connect
void notifySetupMode(AsyncWebSocketClient *client) {
    if (inPointStorageMode) {
        client->text("setup");
    } else {
        client->text("setup-done");
    }
    
    // Send current position
    sendCurrentPosition(client);
    
    // Send current laser state
    String laserMsg = "{\"type\":\"laser-state\",\"active\":" + String(laserActive ? "true" : "false") + "}";
    client->text(laserMsg);

    // Initial status report
    Serial.println("System initialized.");
    Serial.print("Current X: ");
    Serial.println(currentX);
    Serial.print("Current Y: ");
    Serial.println(currentY);
    Serial.print("Laser Active: ");
    Serial.println(laserActive ? "true" : "false");
}

void loop() {
    // Execute any queued motion commands
    motionExecutor.executeNext();

    // Check if pattern has completed
    if (isZigging && !motionExecutor.isBusy()) {
        isZigging = false;
        ws.textAll("pattern-complete");
        Serial.println("Pattern completed.");
    }

    // Handle LED blinking for point storage mode
    if (inPointStorageMode) {
        unsigned long currentTime = millis();
        if (currentTime - lastLedToggleTime > 500) {
            ledState = !ledState;
            digitalWrite(ledPin, ledState);
            lastLedToggleTime = currentTime;
        }
    }

    // Check for main button press to toggle laser state
    if (mainButton.wasPressed()) {
        if (!inPointStorageMode) {
            laserActive = !laserActive;
            if (laserActive) {
                laserStartTime = millis(); // Record start time
                Serial.println("Laser activated.");
            } else {
                Serial.println("Laser deactivated.");
                servoController.setLaser(false); // Turn off laser immediately
            }
            
            // Broadcast laser state change to all clients
            broadcastLaserState();
        }
    }

    // Auto-disable laser after 30 seconds if active
    if (laserActive) {
        servoController.setLaser(true); // Turn on laser
        
        // Auto-disable after 30 seconds (30,000 ms)
        if (millis() - laserStartTime > 30000) {
            laserActive = false;
            servoController.setLaser(false); // Turn off laser
            Serial.println("Laser auto-disabled after timeout.");
            
            // Broadcast laser state change to all clients
            broadcastLaserState();
        }
    } else {
        servoController.setLaser(false); // Turn off laser
    }
}

// Function to enter point storage mode
void enterPointStorageMode() {
    inPointStorageMode = true;
    currentPointIndex = 0;
    Serial.println("Entering point storage mode. Use the web interface to position the laser and store points.");
    lastLedToggleTime = millis();
    ledState = true;
    digitalWrite(ledPin, ledState);
    // Notify all connected WebSocket clients
    ws.textAll("setup");
}

// Function to map a point from an arbitrary quadrilateral to a unit square
void mapPointToSquare(float u, float v, Point* result) {
    // Use boundary mapping with new geometry system
    Quadrilateral boundary(storedPoints);
    NormalizedPoint normPos(u, v);
    *result = Geometry::mapToQuadrilateral(normPos, boundary);
}
// Function to perform zigzag pattern within the defined boundary
void performZigzagPattern() {
    const int numZigzags = 8;       // Number of zigzag lines
    const int pointsPerLine = 10;   // Points per zigzag line
    const int moveDelay = 50;      // Delay between points in milliseconds
    
    Serial.println("Starting zigzag pattern...");
    
    // Create zigzag pattern generator with configuration
    ZigzagPattern zigzag(numZigzags, pointsPerLine);
    
    // Generate pattern points
    const int maxPoints = 200;
    NormalizedPoint patternPoints[maxPoints];
    int pointCount = zigzag.generatePattern(patternPoints, maxPoints);
    
    // Convert pattern points to motion commands
    MotionCommand commands[maxPoints];
    int commandCount = 0;
    motionPlanner.generateZigzagPattern(storedPoints, commands, commandCount, maxPoints, numZigzags, pointsPerLine, moveDelay);
    
    // Execute the sequence
    motionExecutor.queueCommands(commands, commandCount);
    
    isZigging = true; // Set flag to indicate zigzag pattern is being performed
    Serial.print("Zigzag pattern queued with ");
    Serial.print(commandCount);
    Serial.println(" commands.");
}

// Function to perform spiral pattern within the defined boundary
void performSpiralPattern() {
    Serial.println("Starting spiral pattern...");
    
    // Create spiral pattern generator with configuration
    SpiralPattern spiral(0.4f, 80, 2.0f); // radius, points, tightness
    
    // Generate pattern points
    const int maxPoints = 200;
    NormalizedPoint patternPoints[maxPoints];
    int pointCount = spiral.generatePattern(patternPoints, maxPoints);
    
    // Convert pattern points to motion commands
    MotionCommand commands[maxPoints];
    for (int i = 0; i < pointCount && i < maxPoints; i++) {
        Point mapped = motionPlanner.mapToBoundary(patternPoints[i].u, patternPoints[i].v, storedPoints);
        commands[i] = MotionCommand(mapped, 75, true);
    }
    
    motionExecutor.queueCommands(commands, pointCount);
    
    isZigging = true;
    Serial.print("Spiral pattern queued with ");
    Serial.print(pointCount);
    Serial.println(" commands.");
}

// Function to perform random walk pattern within the defined boundary
void performRandomPattern() {
    Serial.println("Starting random walk pattern...");
    
    // Create random walk pattern generator with configuration
    RandomWalkPattern random(50, 0.15f); // points, stepSize
    
    // Generate pattern points
    const int maxPoints = 200;
    NormalizedPoint patternPoints[maxPoints];
    int pointCount = random.generatePattern(patternPoints, maxPoints);
    
    // Convert pattern points to motion commands
    MotionCommand commands[maxPoints];
    for (int i = 0; i < pointCount && i < maxPoints; i++) {
        Point mapped = motionPlanner.mapToBoundary(patternPoints[i].u, patternPoints[i].v, storedPoints);
        commands[i] = MotionCommand(mapped, 100, true);
    }
    
    motionExecutor.queueCommands(commands, pointCount);
    
    isZigging = true;
    Serial.print("Random walk pattern queued with ");
    Serial.print(pointCount);
    Serial.println(" commands.");
}

// Function to perform figure-8 pattern within the defined boundary
void performFigure8Pattern() {
    Serial.println("Starting figure-8 pattern...");
    
    // Create figure-8 pattern generator with configuration
    Figure8Pattern figure8(64, 0.3f); // points, scale
    
    // Generate pattern points
    const int maxPoints = 200;
    NormalizedPoint patternPoints[maxPoints];
    int pointCount = figure8.generatePattern(patternPoints, maxPoints);
    
    // Convert pattern points to motion commands
    MotionCommand commands[maxPoints];
    for (int i = 0; i < pointCount && i < maxPoints; i++) {
        Point mapped = motionPlanner.mapToBoundary(patternPoints[i].u, patternPoints[i].v, storedPoints);
        commands[i] = MotionCommand(mapped, 60, true);
    }
    
    motionExecutor.queueCommands(commands, pointCount);
    
    isZigging = true;
    Serial.print("Figure-8 pattern queued with ");
    Serial.print(pointCount);
    Serial.println(" commands.");
}

// Function to perform perimeter pattern within the defined boundary
void performPerimeterPattern() {
    Serial.println("Starting perimeter pattern...");
    
    // Create perimeter pattern generator with configuration
    PerimeterPattern perimeter(60); // points - more points for smoother perimeter
    
    // Generate pattern points
    const int maxPoints = 200;
    NormalizedPoint patternPoints[maxPoints];
    int pointCount = perimeter.generatePattern(patternPoints, maxPoints);
    
    // Convert pattern points to motion commands
    MotionCommand commands[maxPoints];
    for (int i = 0; i < pointCount && i < maxPoints; i++) {
        Point mapped = motionPlanner.mapToBoundary(patternPoints[i].u, patternPoints[i].v, storedPoints);
        commands[i] = MotionCommand(mapped, 80, true);
    }
    
    motionExecutor.queueCommands(commands, pointCount);
    
    isZigging = true;
    Serial.print("Perimeter pattern queued with ");
    Serial.print(pointCount);
    Serial.println(" commands.");
}