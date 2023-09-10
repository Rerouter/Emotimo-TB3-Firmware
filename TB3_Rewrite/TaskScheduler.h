#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <Arduino.h>

constexpr uint8_t MAX_TASKS = 10;

typedef uint32_t (*TaskFunction)();

struct Task {
    TaskFunction function;
    uint32_t nextRunTime;
};

class TaskScheduler {
public:
    TaskScheduler() : taskCount(0) {}

    // Add a task to the scheduler
    void addTask(TaskFunction func, uint32_t nextMicros);
    void addTask(TaskFunction func);

    // Execute due tasks
    void runTasks();

private:
    // Remove a task at a given position
    void removeTask(uint8_t pos);

    Task tasks[MAX_TASKS];
    uint8_t taskCount;
};

#endif // TASKSCHEDULER_H
