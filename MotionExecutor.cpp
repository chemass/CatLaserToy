#include "MotionExecutor.h"
#ifdef ARDUINO
#include <Arduino.h>
#endif

// Mock Arduino functions for testing
#ifndef ARDUINO
unsigned long millis() {
    static unsigned long mock_time = 0;
    return mock_time += 10; // Increment by 10ms each call for testing
}
#endif

MotionExecutor::MotionExecutor(ServoController& controller) 
    : servoController(controller), queueHead(0), queueTail(0), queueSize(0),
      commandStartTime(0), executing(false) {
}

void MotionExecutor::queueCommands(const MotionCommand* commands, int commandCount) {
    for (int i = 0; i < commandCount && !isQueueFull(); i++) {
        enqueue(commands[i]);
    }
    
#ifdef ARDUINO
    Serial.print("Queued ");
    Serial.print(commandCount);
    Serial.print(" commands. Queue size: ");
    Serial.println(queueSize);
#endif
}

bool MotionExecutor::executeNext() {
    // If currently executing a command, check if it's complete
    if (executing) {
        if (isCommandComplete()) {
            executing = false;
#ifdef ARDUINO
            Serial.println("Command completed");
#endif
        } else {
            return true; // Still executing
        }
    }
    
    // If not executing and queue has commands, start next command
    if (!executing && !isQueueEmpty()) {
        MotionCommand nextCommand = dequeue();
        startCommand(nextCommand);
        return true;
    }
    
    return false; // Nothing to execute
}

void MotionExecutor::moveImmediate(const Point& target, bool laserOn) {
    // Stop current execution and clear queue for immediate movement
    stop();
    
    // Execute immediately
    servoController.moveTo(target);
    servoController.setLaser(laserOn);
    
#ifdef ARDUINO
    Serial.print("Immediate move to X: ");
    Serial.print(target.x);
    Serial.print(", Y: ");
    Serial.print(target.y);
    Serial.print(", Laser: ");
    Serial.println(laserOn ? "ON" : "OFF");
#endif
}

void MotionExecutor::clearQueue() {
    queueHead = 0;
    queueTail = 0;
    queueSize = 0;
#ifdef ARDUINO
    Serial.println("Motion queue cleared");
#endif
}

void MotionExecutor::stop() {
    executing = false;
    clearQueue();
    servoController.setLaser(false); // Turn off laser for safety
#ifdef ARDUINO
    Serial.println("Motion executor stopped");
#endif
}

void MotionExecutor::startCommand(const MotionCommand& cmd) {
    currentCommand = cmd;
    commandStartTime = millis();
    executing = true;
    
    // Execute the movement
    servoController.moveTo(cmd.target);
    servoController.setLaser(cmd.laser_on);
    
#ifdef ARDUINO
    Serial.print("Executing command: X=");
    Serial.print(cmd.target.x);
    Serial.print(", Y=");
    Serial.print(cmd.target.y);
    Serial.print(", Duration=");
    Serial.print(cmd.duration_ms);
    Serial.print("ms, Laser=");
    Serial.println(cmd.laser_on ? "ON" : "OFF");
#endif
}

bool MotionExecutor::isCommandComplete() {
    return (millis() - commandStartTime) >= currentCommand.duration_ms;
}

void MotionExecutor::enqueue(const MotionCommand& cmd) {
    if (!isQueueFull()) {
        commandQueue[queueTail] = cmd;
        queueTail = (queueTail + 1) % MAX_QUEUE_SIZE;
        queueSize++;
    } else {
#ifdef ARDUINO
        Serial.println("Warning: Motion queue is full, command dropped");
#endif
    }
}

MotionCommand MotionExecutor::dequeue() {
    if (!isQueueEmpty()) {
        MotionCommand cmd = commandQueue[queueHead];
        queueHead = (queueHead + 1) % MAX_QUEUE_SIZE;
        queueSize--;
        return cmd;
    }
    
    // Return empty command if queue is empty
    return MotionCommand();
}