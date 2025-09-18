#line 1 "C:\\Users\\utscs\\source\\repos\\CatLaserToy\\MotionExecutor.cpp"
#include "MotionExecutor.h"
#include <Arduino.h>

MotionExecutor::MotionExecutor(ServoController& controller) 
    : servoController(controller), queueHead(0), queueTail(0), queueSize(0),
      commandStartTime(0), executing(false) {
}

void MotionExecutor::queueCommands(const MotionCommand* commands, int commandCount) {
    for (int i = 0; i < commandCount && !isQueueFull(); i++) {
        enqueue(commands[i]);
    }
    
    Serial.print("Queued ");
    Serial.print(commandCount);
    Serial.print(" commands. Queue size: ");
    Serial.println(queueSize);
}

bool MotionExecutor::executeNext() {
    // If currently executing a command, check if it's complete
    if (executing) {
        if (isCommandComplete()) {
            executing = false;
            Serial.println("Command completed");
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
    
    Serial.print("Immediate move to X: ");
    Serial.print(target.x);
    Serial.print(", Y: ");
    Serial.print(target.y);
    Serial.print(", Laser: ");
    Serial.println(laserOn ? "ON" : "OFF");
}

void MotionExecutor::clearQueue() {
    queueHead = 0;
    queueTail = 0;
    queueSize = 0;
    Serial.println("Motion queue cleared");
}

void MotionExecutor::stop() {
    executing = false;
    clearQueue();
    servoController.setLaser(false); // Turn off laser for safety
    Serial.println("Motion executor stopped");
}

void MotionExecutor::startCommand(const MotionCommand& cmd) {
    currentCommand = cmd;
    commandStartTime = millis();
    executing = true;
    
    // Execute the movement
    servoController.moveTo(cmd.target);
    servoController.setLaser(cmd.laser_on);
    
    Serial.print("Executing command: X=");
    Serial.print(cmd.target.x);
    Serial.print(", Y=");
    Serial.print(cmd.target.y);
    Serial.print(", Duration=");
    Serial.print(cmd.duration_ms);
    Serial.print("ms, Laser=");
    Serial.println(cmd.laser_on ? "ON" : "OFF");
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
        Serial.println("Warning: Motion queue is full, command dropped");
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