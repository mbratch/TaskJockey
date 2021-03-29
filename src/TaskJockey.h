#ifndef _TaskJockey_h
#define _TaskJockey_h

#include <Arduino.h>

typedef uint8_t taskId_t;

class TaskJockey {
    public:
        TaskJockey(void);
        ~TaskJockey();

        taskId_t addTask(void (*handler)(taskId_t), void *args, uint16_t interval, bool runImmediate = false, int8_t iterations = -1);
        void pauseTask(taskId_t taskId);
        void resumeTask(taskId_t taskId);
        void resetTaskTimer(taskId_t taskId);
        void killAllTasks(void);
        void killTask(taskId_t taskId);
        void *getTaskArgs(taskId_t taskId);
        int8_t getTaskIterationsRemaining(taskId_t taskId);
        uint32_t getTaskLastRunTime(taskId_t taskId);
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
            uint32_t interval;      // run in specified millisecond intervals
            int8_t iteration;       // Iterations remaining to run (-1 is infinite)
            uint32_t lastRunTime;   // Last run time (ms)
            uint32_t elapsedTime;   // Time elapsed since last run time
            taskItem_t *nextTask;   // Next task (for linked list table of tasks)
        };

        void deleteDeadTasks(void); // Delete all dead tasks
        uint32_t timeNow(void);     // Get current time
        taskId_t newTaskId(void);   // Get a new (currently unused) task Id
        taskItem_t *findTask(taskId_t taskId);  // Find the given task id in the task table

        uint32_t lastTime_;         // last time the scheduler ran
        taskId_t nextTaskId_;       // next assumed available task Id
        taskItem_t *taskTable_;     // Table of known tasks
};

#endif