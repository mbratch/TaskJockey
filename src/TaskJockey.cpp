#include "TaskJockey.h"

TaskJockey::TaskJockey()
{
    taskTable_ = NULL;
    nextTaskId_ = 1;
    lastTime_ = timeNow();
}

TaskJockey::~TaskJockey()
{
    taskItem_t *nextTask;

    for (taskItem_t *task = taskTable_; task != NULL; task = nextTask) {
        nextTask = task->nextTask;
        delete task;
    }
}

taskId_t TaskJockey::addTask(void (*handler)(taskId_t), void *args,
    uint16_t interval, uint16_t offsetStart, int8_t iterations)
{
    if (interval == 0)
        return 0;
        
    taskId_t taskId = newTaskId();

    if (taskId == 0)
        return 0;

    taskItem_t *task = new taskItem_t;

    task->taskId = taskId;
    task->handler = handler;
    task->args = args;
    task->interval = interval;
    task->iteration = iterations;
    task->state = taskActive;
    task->lastRunTime = timeNow() - task->interval + offsetStart;

    task->nextTask = NULL;

    // Add the task to the task table
    taskItem_t **ttp;

    for (ttp = &taskTable_; *ttp != NULL; ttp = &((*ttp)->nextTask))
        ;

    *ttp = task;

    return taskId;
}

void TaskJockey::pauseTask(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    // Disable the task only if it exists
    if (task != NULL && task->state == taskActive) {
        task->state = taskPaused;
        task->elapsedTime = timeNow() - task->lastRunTime;
    }
}

void TaskJockey::resumeTask(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    // Resume task only if it exists and is the head of a chain
    if (task != NULL && task->state == taskPaused) {
        task->lastRunTime = timeNow() - task->interval + task->elapsedTime;
        task->state = taskActive;
    }
}

void TaskJockey::resetTaskTimer(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    if (task != NULL) {
        task->lastRunTime = timeNow();
        task->elapsedTime = task->interval;
    }
}

void TaskJockey::killAllTasks(void)
{
    for (taskItem_t *task = taskTable_; task != NULL; task = task->nextTask) {
        task->state = taskDead;
    }
}

void TaskJockey::killTask(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    if (task != NULL)
        task->state = taskDead;
}

void *TaskJockey::getTaskArgs(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    return (task == NULL) ? NULL : task->args;
}

uint16_t TaskJockey::getTaskInterval(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    return (task == NULL) ? 0 : task->interval;
}

void TaskJockey::setTaskInterval(taskId_t taskId, uint16_t interval)
{
    taskItem_t *task = findTask(taskId);

    if (task != NULL)
        task->interval = interval;
}

int8_t TaskJockey::getTaskIterationsRemaining(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    return (task == NULL) ? 0 : task->iteration;
}

uint32_t TaskJockey::getTaskLastRunTime(taskId_t taskId)
{
    taskItem_t *task = findTask(taskId);

    return (task == NULL) ? 0 : task->lastRunTime;
}

void TaskJockey::deleteDeadTasks(void) {
    for (taskItem_t* task = taskTable_, *priorTask = NULL; task != NULL; ) {
        if (task->state == taskDead) {
            taskItem_t *thisTask = task;

            if (priorTask == NULL)
                task = taskTable_ = thisTask->nextTask;
            else
                task = priorTask->nextTask = thisTask->nextTask;

            delete thisTask;
        }
        else {
            priorTask = task;
            task = task->nextTask;
        }
    }
}

void TaskJockey::runTasks(void)
{
    uint32_t currentTime = timeNow();

    // Run active tasks & delete dead tasks
    for (taskItem_t *task = taskTable_; task != NULL; ) {
        taskItem_t *thisTask = task;
        task = task->nextTask;

        if (currentTime - thisTask->lastRunTime >= thisTask->interval &&
                 thisTask->iteration != 0 && thisTask->state == taskActive) {
            thisTask->handler(thisTask->taskId);
            thisTask->lastRunTime = currentTime;

            if (thisTask->iteration > 0)
                thisTask->iteration -= 1;
        }

        // Kill the task automatically when it's done
        if (thisTask->iteration == 0)
            thisTask->state = taskDead;
    }

    deleteDeadTasks();

    lastTime_ = currentTime;
}

uint32_t TaskJockey::timeNow(void)
{
    return millis();
}

taskId_t TaskJockey::newTaskId(void)
{
    bool idFound = true;
    taskId_t taskId = nextTaskId_;

    for (taskItem_t *task = taskTable_; task != NULL; task = task->nextTask) {
        if (taskId == task->taskId) {
            if (++taskId == 0)
                taskId = 1;

            // If every possible taskId has been tried (unlikely!) then
            // we're out of them
            if (taskId == nextTaskId_) {
                idFound = false;
                break;
            }

            task = taskTable_;
        }
    }

    if (idFound) {
        if ((nextTaskId_ = taskId + 1) == 0)
            nextTaskId_ = 1;

        return taskId;
    }

    return 0;
}
        
TaskJockey::taskItem_t *TaskJockey::findTask(taskId_t taskId)   // Find the given task id in the task table
{
    if (taskId == 0)
        return NULL;

    for (taskItem_t* task = taskTable_; task != NULL; task = task->nextTask) {
        if (task->taskId == taskId)
            return task;
    }

    return NULL;
};
