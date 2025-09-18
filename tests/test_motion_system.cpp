#include "doctest.h"
#include "MockServoController.h"

// Include source files for testing
#include "../MotionPlanner.h"
#include "../MotionPlanner.cpp"
#include "../MotionExecutor.h"
#include "../MotionExecutor.cpp"

TEST_CASE("MotionPlanner boundary setup and validation") {
    MotionPlanner planner;
    MockServoController mockServo;
    
    // Test default boundary
    CHECK(planner.hasBoundary());
    
    // Test setting custom boundary
    Point corners[4] = {
        Point(10, 10),   // Top-left
        Point(170, 20),  // Top-right
        Point(160, 170), // Bottom-right
        Point(20, 160)   // Bottom-left
    };
    
    planner.setBoundary(corners);
    CHECK(planner.hasBoundary());
}

TEST_CASE("MotionPlanner pattern execution") {
    MotionPlanner planner;
    MockServoController mockServo;
    
    // Clear any previous commands
    mockServo.clearLog();
    
    // Test zigzag pattern
    bool success = planner.executeZigzagPattern(3, 5);
    CHECK(success);
    
    // Should generate some motion commands
    // Note: Since we're testing the planner in isolation, 
    // we need to verify it can generate valid motion sequences
}

TEST_CASE("MotionExecutor command queueing") {
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    mockServo.clearLog();
    
    // Create some test commands
    MotionCommand cmd1;
    cmd1.targetX = 90;
    cmd1.targetY = 45;
    cmd1.laser = true;
    cmd1.duration_ms = 100;
    
    MotionCommand cmd2;
    cmd2.targetX = 45;
    cmd2.targetY = 90;
    cmd2.laser = false;
    cmd2.duration_ms = 150;
    
    MotionCommand commands[2] = {cmd1, cmd2};
    
    // Queue the commands
    bool queued = executor.queueCommands(commands, 2);
    CHECK(queued);
    
    // Executor should report as busy
    CHECK(executor.isBusy());
    
    // Should be able to get queue status
    CHECK_GT(executor.getQueueSize(), 0);
}

TEST_CASE("MotionExecutor immediate movement") {
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    mockServo.clearLog();
    
    // Test immediate movement
    executor.moveImmediate(100, 80, true);
    
    // Should have logged the movement command
    CHECK_GT(mockServo.getCommandCount(), 0);
    CHECK(mockServo.hasCommand("setPosition"));
    CHECK(mockServo.hasCommand("setLaser"));
    
    auto lastCmd = mockServo.getLastCommand();
    CHECK_EQ(lastCmd.value1, 1.0f); // Laser should be on
}

TEST_CASE("MotionExecutor stop functionality") {
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    // Queue some commands first
    MotionCommand cmd;
    cmd.targetX = 90;
    cmd.targetY = 90;
    cmd.laser = true;
    cmd.duration_ms = 1000;
    
    executor.queueCommands(&cmd, 1);
    CHECK(executor.isBusy());
    
    mockServo.clearLog();
    
    // Stop execution
    executor.stop();
    
    // Should no longer be busy
    CHECK(!executor.isBusy());
    CHECK_EQ(executor.getQueueSize(), 0);
    
    // Should have turned off laser
    CHECK(mockServo.hasCommand("setLaser"));
    auto lastCmd = mockServo.getLastCommand();
    CHECK_EQ(lastCmd.value1, 0.0f); // Laser should be off
}

TEST_CASE("MotionExecutor queue overflow protection") {
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    // Try to queue more commands than the limit
    std::vector<MotionCommand> commands(250); // More than MAX_QUEUE_SIZE
    
    for (size_t i = 0; i < commands.size(); i++) {
        commands[i].targetX = 90;
        commands[i].targetY = 90;
        commands[i].laser = false;
        commands[i].duration_ms = 10;
    }
    
    // Should reject the oversized queue
    bool queued = executor.queueCommands(commands.data(), commands.size());
    CHECK(!queued);
    
    // Queue should remain empty or manageable
    CHECK_LT(executor.getQueueSize(), 250);
}

TEST_CASE("Integration test - MotionPlanner with MotionExecutor") {
    MotionPlanner planner;
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    mockServo.clearLog();
    
    // Set up a simple boundary
    Point corners[4] = {
        Point(45, 45),   // Top-left
        Point(135, 45),  // Top-right
        Point(135, 135), // Bottom-right
        Point(45, 135)   // Bottom-left
    };
    planner.setBoundary(corners);
    
    // Execute a simple pattern
    bool success = planner.executeZigzagPattern(2, 3);
    CHECK(success);
    
    // The motion planner should have generated some valid commands
    // (Note: In a real integration, the planner would queue commands to the executor)
}

TEST_CASE("MotionExecutor timing and updates") {
    MockServoController mockServo;
    MotionExecutor executor(&mockServo);
    
    // Create a command with specific timing
    MotionCommand cmd;
    cmd.targetX = 120;
    cmd.targetY = 60;
    cmd.laser = true;
    cmd.duration_ms = 50;
    
    executor.queueCommands(&cmd, 1);
    
    // Simulate time passage with update calls
    // (In real system, this would be called from main loop)
    executor.update();
    
    // After sufficient updates/time, command should complete
    // (This is a simplified test - real timing would use millis())
}

TEST_CASE("MockServoController logging functionality") {
    MockServoController mockServo;
    
    // Test basic operations are logged
    mockServo.begin();
    mockServo.setPosition(90, 45);
    mockServo.setLaser(true);
    
    CHECK_EQ(mockServo.getCommandCount(), 3);
    CHECK(mockServo.hasCommand("begin"));
    CHECK(mockServo.hasCommand("setPosition"));
    CHECK(mockServo.hasCommand("setLaser"));
    
    // Test state tracking
    CHECK_EQ(mockServo.getCurrentX(), 90);
    CHECK_EQ(mockServo.getCurrentY(), 45);
    CHECK(mockServo.isLaserOn());
    
    // Test log clearing
    mockServo.clearLog();
    CHECK_EQ(mockServo.getCommandCount(), 0);
    
    // State should persist after log clear
    CHECK_EQ(mockServo.getCurrentX(), 90);
    CHECK_EQ(mockServo.getCurrentY(), 45);
    CHECK(mockServo.isLaserOn());
}