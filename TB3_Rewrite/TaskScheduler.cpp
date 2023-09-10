#include "TaskScheduler.h"

void TaskScheduler::addTask(TaskFunction func, uint32_t nextMicros) {
    if (taskCount < MAX_TASKS) {
        Task newTask;
        newTask.function = func;
        newTask.nextRunTime = nextMicros;
        
        uint8_t pos = taskCount;
        while (pos > 0 && tasks[pos-1].nextRunTime > newTask.nextRunTime) {
            tasks[pos] = tasks[pos-1];
            pos--;
        }
        tasks[pos] = newTask;
        taskCount++;
    }
}

void TaskScheduler::addTask(TaskFunction func) {
    if (taskCount < MAX_TASKS) {
        Task newTask;
        newTask.function = func;
        newTask.nextRunTime = micros();
        
        uint8_t pos = taskCount;
        while (pos > 0 && tasks[pos-1].nextRunTime > newTask.nextRunTime) {
            tasks[pos] = tasks[pos-1];
            pos--;
        }
        tasks[pos] = newTask;
        taskCount++;
    }
}

void TaskScheduler::runTasks() {
    uint32_t currentTime = micros();
    uint8_t i = 0;
    while (i < taskCount) {
        if ((int32_t)(currentTime - tasks[i].nextRunTime) >= 0) {
            uint32_t delay = tasks[i].function();
            if (delay == 0) {
                removeTask(i);
            } else {
                tasks[i].nextRunTime = delay;
                Task updatedTask = tasks[i];
                removeTask(i);
                addTask(updatedTask.function, updatedTask.nextRunTime);
            }
        } else {
            break;
        }
        i++;
    }
}

void TaskScheduler::removeTask(uint8_t pos) {
    for (uint8_t j = pos; j < taskCount-1; j++) {
        tasks[j] = tasks[j+1];
    }
    taskCount--;
}
