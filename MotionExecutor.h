#ifndef MOTION_EXECUTOR_H
#define MOTION_EXECUTOR_H

#include "MotionPlanner.h"
#include "ServoController.h"

class MotionExecutor {
public:
    static const int MAX_QUEUE_SIZE = 200;
    
    MotionExecutor(ServoController& controller);
    
    // Queue a sequence of commands for execution
    void queueCommands(const MotionCommand* commands, int commandCount);
    
    // Execute next command if ready (call in loop())
    bool executeNext();
    
    // Immediate movement (bypasses queue)
    void moveImmediate(const Point& target, bool laserOn = false);
    
    // Status queries
    bool isBusy() const { return queueSize > 0 || executing; }
    int getQueueSize() const { return queueSize; }
    void clearQueue();
    
    // Emergency stop
    void stop();
    
private:
    ServoController& servoController;
    
    // Command queue (using array instead of std::queue for Arduino compatibility)
    MotionCommand commandQueue[MAX_QUEUE_SIZE];
    int queueHead;
    int queueTail;
    int queueSize;
    
    // Execution state
    unsigned long commandStartTime;
    bool executing;
    MotionCommand currentCommand;
    
    void startCommand(const MotionCommand& cmd);
    bool isCommandComplete();
    void enqueue(const MotionCommand& cmd);
    MotionCommand dequeue();
    bool isQueueEmpty() const { return queueSize == 0; }
    bool isQueueFull() const { return queueSize >= MAX_QUEUE_SIZE; }
};

#endif // MOTION_EXECUTOR_H