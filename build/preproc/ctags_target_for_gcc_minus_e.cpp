# 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino"
# 2 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 3 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 4 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2

# 6 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 7 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 8 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 9 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 10 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 11 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 12 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 13 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 14 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2
# 15 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 2

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
const int ledPin = 2; // ESP32 internal LED

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
    Serial0.begin(115200);

    // Initialize EEPROM
    storage.begin();

    // Initialize servo controller
    servoController.begin();
    servoController.setLimits(xMin, xMax, yMin, yMax);

    // Set LED pin
    pinMode(ledPin, 0x03);
    digitalWrite(ledPin, 0x0); // LED off by default

    // Load stored points from EEPROM
    if (!storage.loadPoints(storedPoints)) {
        Serial0.println("No stored points found. Entering setup mode.");
        inPointStorageMode = true;
        currentPointIndex = 0;
    } else {
        Serial0.println("Stored points loaded from EEPROM:");
        for (int i = 0; i < StorageManager::MAX_POINTS; i++) {
            Serial0.print("Point ");
            Serial0.print(i + 1);
            Serial0.print(": X=");
            Serial0.print(storedPoints[i].x);
            Serial0.print(", Y=");
            Serial0.println(storedPoints[i].y);
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
    Serial0.print(".");
    }

    Serial0.println("\nConnected to WiFi");
    Serial0.print("IP address: ");
    Serial0.println(WiFi.localIP());

    // Setup mDNS
    if (!MDNS.begin(hostname)) {
        Serial0.println("Error setting up mDNS responder!");
    } else {
        Serial0.printf("mDNS responder started. Access via: http://%s.local\n", hostname);
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
        Serial0.print("WebSocket client connected from IP: ");
        Serial0.println(client->remoteIP());
        notifySetupMode(client);
    } else if (type == WS_EVT_DISCONNECT) {
        Serial0.print("WebSocket client disconnected from IP: ");
        Serial0.println(client->remoteIP());
    } else if (type == WS_EVT_ERROR) {
        Serial0.print("WebSocket error: ");
        Serial0.println((char*)data);
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len) {
            data[len] = 0;
            String msg = (char *)data;
            Serial0.print("WS received: ");
            Serial0.println(msg);

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
                        }
                        return;
                    } else if (typeVal == "laser-toggle" || typeVal == "toggle-laser") {
                        laserActive = !laserActive;
                        servoController.setLaser(laserActive);
                        if (laserActive) {
                            laserStartTime = millis();
                        }
                        client->text("{\"laser\":" + String(laserActive ? "true" : "false") + "}");
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
                        servoController.adjustY(-2);
                        return;
                    } else if (typeVal == "move-down") {
                        servoController.adjustY(2);
                        return;
                    } else if (typeVal == "move-left") {
                        servoController.adjustX(-2);
                        return;
                    } else if (typeVal == "move-right") {
                        servoController.adjustX(2);
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
                            MotionCommand commands[200];
                            MotionConfig config;
                            config.pointDuration_ms = 50;
                            config.laserEnabled = true;
                            config.returnToCenter = false;
                            Quadrilateral 
# 304 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 3
                                         quad_t
# 304 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino"
                                             (storedPoints);
                            int cmdCount = motionPlanner.executePattern(*pattern, 
# 305 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino" 3
                                                                                 quad_t
# 305 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\CatLaserToy.ino"
                                                                                     , commands, 200, config);
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
        Serial0.println("WebSocket client connected");
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
    Serial0.println("Sent position: " + positionMsg);
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

    // Initial status report
    Serial0.println("System initialized.");
    Serial0.print("Current X: ");
    Serial0.println(currentX);
    Serial0.print("Current Y: ");
    Serial0.println(currentY);
}

void loop() {
    // Execute any queued motion commands
    motionExecutor.executeNext();

    // Check if pattern has completed
    if (isZigging && !motionExecutor.isBusy()) {
        isZigging = false;
        ws.textAll("pattern-complete");
        Serial0.println("Pattern completed.");
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
                Serial0.println("Laser activated.");
            } else {
                Serial0.println("Laser deactivated.");
                servoController.setLaser(false); // Turn off laser immediately
            }
        }
    }

    // Auto-disable laser after 5 minutes if active
    if (laserActive) {
        servoController.setLaser(true); // Turn on laser

        // Auto-disable after 5 minutes (300,000 ms)
        if (millis() - laserStartTime > 300000) {
            laserActive = false;
            servoController.setLaser(false); // Turn off laser
            Serial0.println("Laser auto-disabled after timeout.");
        }
    } else {
        servoController.setLaser(false); // Turn off laser
    }
}

// Function to enter point storage mode
void enterPointStorageMode() {
    inPointStorageMode = true;
    currentPointIndex = 0;
    Serial0.println("Entering point storage mode. Use the web interface to position the laser and store points.");
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
    const int numZigzags = 8; // Number of zigzag lines
    const int pointsPerLine = 10; // Points per zigzag line
    const int moveDelay = 50; // Delay between points in milliseconds

    Serial0.println("Starting zigzag pattern...");

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
    Serial0.print("Zigzag pattern queued with ");
    Serial0.print(commandCount);
    Serial0.println(" commands.");
}

// Function to perform spiral pattern within the defined boundary
void performSpiralPattern() {
    Serial0.println("Starting spiral pattern...");

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
    Serial0.print("Spiral pattern queued with ");
    Serial0.print(pointCount);
    Serial0.println(" commands.");
}

// Function to perform random walk pattern within the defined boundary
void performRandomPattern() {
    Serial0.println("Starting random walk pattern...");

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
    Serial0.print("Random walk pattern queued with ");
    Serial0.print(pointCount);
    Serial0.println(" commands.");
}

// Function to perform figure-8 pattern within the defined boundary
void performFigure8Pattern() {
    Serial0.println("Starting figure-8 pattern...");

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
    Serial0.print("Figure-8 pattern queued with ");
    Serial0.print(pointCount);
    Serial0.println(" commands.");
}

// Function to perform perimeter pattern within the defined boundary
void performPerimeterPattern() {
    Serial0.println("Starting perimeter pattern...");

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
    Serial0.print("Perimeter pattern queued with ");
    Serial0.print(pointCount);
    Serial0.println(" commands.");
}
