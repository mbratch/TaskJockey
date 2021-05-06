#ifndef _TaskJockey_h
#define _TaskJockey_h

#include <Arduino.h>

typedef uint8_t taskId_t;

class TaskJockey {
    public:
        TaskJockey(void);
        ~TaskJockey();

        taskId_t addTask(void (*handler)(taskId_t), void *args, uint16_t interval, uint16_t offsetStart = 0, int8_t iterations = -1);
        void pauseTask(taskId_t);
        void resumeTask(taskId_t);
        void resetTaskTimer(taskId_t);
        void killAllTasks(void);
        void killTask(taskId_t);
        void *getTaskArgs(taskId_t);
        uint16_t getTaskInterval(taskId_t);
        void setTaskInterval(taskId_t, uint16_t);
        int8_t getTaskIterationsRemaining(taskId_t);
        uint32_t getTaskLastRunTime(taskId_t);
        void runTasks(void);

    private:
        enum taskState_t {
            taskActive,             // Active (runs when scheduled)
            taskPaused,             // Paused (present but not running)
            taskDead                // To be deleted at earliest convenience
        };

        struct taskItem_t {
            taskId_t taskId;        // Unique taskId (1-255); 0 is a "null" task Id representing "no task"
            void (*handler)(taskId_t);  // Task function
            taskState_t state;      // Task state
            void *args;             // pointer to arguments
            uint32_t runInterval;   // run in specified millisecond intervals
            uint32_t interval;      // current run interval
            int8_t iteration;       // Iterations remaining to run (-1 is infinite)
            uint32_t lastRunTime;   // Last run time (ms)
            uint32_t elapsedTime;   // Time elapsed since last run time
            taskItem_t *nextTask;   // Next task (for linked list table of tasks)
        };

        void deleteDeadTasks(void); // Delete all dead tasks
        uint32_t timeNow(void);     // Get current time
        taskId_t newTaskId(void);   // Get a new (currently unused) task Id
        taskItem_t *findTask(taskId_t);  // Find the given task id in the task table

        uint32_t lastTime_;         // last time the scheduler ran
        taskId_t nextTaskId_;       // next assumed available task Id
        taskItem_t *taskTable_;     // Table of known tasks
};

#endif