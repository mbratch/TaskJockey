# TaskJockey
## Overview
The `TaskJockey` library for Arduino or other base-level loop systems is a light-weight, cooperative task manager that encapsulates the method of tracking timed task iterations using `millis()` (or other function that returns milliseconds since startup) in your `loop()` function. The idea is to instantiate the task jockey, add tasks (functions) that you want to call periodically at various periods, then just call the task jockey in every call to `loop()` to run your tasks rather than handling the tasks individually with their own timing variables and delays.

Using `TaskJockey`, you can:
* Configure a task to run periodically and indefinitely, and include a pointer to static or global data
* Configure a task to run periodically but for a limited number of times or just once at a period of time in the future
* Chain a sequence of tasks to execute at prescribed intervals, one after the other, and run that chain periodically (achieved by allowing tasks to add other tasks)

`TaskJockey` is useful for multiple tasks that are timed to run periodically with a granularity of milliseconds (_e.g._, you want to run a particular task every 2ms, or 125ms, or 5s). It is not designed to handle short cycles of periodic execution less than that. The tasks themselves are each expected to execute in less than 1ms on each call.

See the section below on "Using TaskJockey" for more details.

## Software Details
### Constructor
Defining a `TaskJockey` is very simple. There are no arguments:
```c++
TaskJockey jockey;     // Define a task jockey instance
```
Generally, you only need one task jockey, but you could define more than one if, for some reason, it worked better with your code structure. Each `TaskJockey` instance would have its own schedule you would need to run.

### TaskJockey Methods
```c++
taskId_t addTask(void (*handler)(taskId_t, void *), void *pArgs, uint16_t interval,
                 int16_t offsetStart = -1, int8_t iterations = -1);
```
The arguments for this method are:
* `handler` - The name of a function you have declared which returns `void` and accepts a `taskId_t` argument. `TaskJockey` will call the function at regular, predefined intervals defined by the subsequent parameters. When `TaskJockey` calls the handler, it will pass the task id that has been assigned to that handler as an argument which may be used by the handler to "kill itself" (see `killTask` below) removing it from the task jockey and ending its execution cycle.
* `pArgs` - A pointer to static or global arguments to be passed to the task every time it is executed.
* `interval` - The periodic interval according to which you want `handler` to be called. This is in milliseconds and can be as small as 1ms. If an interval of 0 is passed, `addTask` will return immediately and do nothing. Normally, `handler` will be called every `interval` milliseconds indefinitely unless indicated otherwise by the other arguments, or until the task is killed.
* `offsetStart` - Normally, after `addTask` has been called for a `handler`, `TaskJockey` will run the handler for the first time only after `interval` milliseconds have transpired (`offsetStart` is `interval`). If you want `TaskJockey` to run `handler` starting at a different initial time in the future, set `offsetStart` to the number of milliseconds in the future in which you want it to be first run. If `offsetStart` is negative, it will default to `interval` milliseconds.
* `iterations` - Defines the number of iterations that you wish to run `handler`. The value defaults to -1 which means it will run indefinitely. A value of 0 will do nothing (the task will not run at all: it will be scheduled but immediately killed). A positive value will run the `handler` that many times at the specified `interval` and then `TaskJockey` will automatically kill it.

`addTask` returns a unique task Id (type `taskId_t`) for the task added which can be used in calls to other `TaskJockey` methods. A value of 0 indicates a null task, meaning that the call failed. This would only happen if you exceed the total allowed number of tasks, which is 255.
```c++
void pauseTask(taskId_t taskId);
```
Pauses the task with the given `taskId`. The task remains in this state in `TaskJockey` and no longer runs until it is subsequently resumed or it killed. When a task is paused, it is in a suspended state with respect to the time interval it is programmed for (see `resumeTask`).

```c++
void resumeTask(taskId_t taskId);
```
Resumes a previously paused task. When a task that was previously paused is resumed, the state of its interval is resumed from its last active count. For example, if a task is scheduled to run every 1000 ms (1 second) and it is paused half way through a cycle (at 500 ms), when it is later resumed, it will next execute 500 ms after it has been resumed.

```c++
void resetTaskTimer(taskId_t taskId);
```
Resets the given tasks interval timer to the current time. After resetting the task's timer, it will not execute again until `interval` milliseconds have passed (as long as the task is active).

```c++
void killAllTasks(void);
```
All tasks in the `TaskJockey` will be killed.

```c++
void killTask(taskId_t taskId);
```
Kill the specified task. A task is allowed to kill itself (which is actually a useful feature).

```c++
void *getTaskArgs(taskId_t taskId);
```
Get a pointer to the task argument(s). This is the same pointer to the static or global arguments that was passed at the time the task was added to `TaskJockey`.
```c++
uint16_t getTaskInterval(taskId_t taskId);
```
Gets the interval time (milliseconds) of the given task.
```c++
uint16_t setTaskInterval(taskId_t taskId, uint16_t interval);
```
Sets (changes) the interval time (milliseconds) of the given task.
```c++
int8_t getTaskIterationsRemaining(taskId_t taskId);
```
Gets the number of task iterations remaining. A value of -1 means the task will be runs indefinitely.
```c++
uint32_t getTaskLastRunTime(taskId_t taskId);
```
Gets the last run time in microseconds since the system was last initialized (that is, per `micros()` call)
```c++
void runTasks(void);
```
Check all of the scheduled tasks in the `TaskJockey` that are active (not paused) and run them if their interval has expired.
 
## Using TaskJockey
In the simplest case, you instantiate a `TaskJockey` variable, add tasks in your `setup()` that you wish to run periodically, then call the `runTasks()` method on ever iteration of `loop()`. `runTasks()` takes no arguments.

```c++
#include <TaskJockey.h>
...

TaskJockey jockey;   // TaskJugger instance
...

void task1(taskId_t taskId, void *pArgs) {
  // Do some things here
}

void task2(taskId_t taskId, void *pArgs) {
  // Do some things here, too
}

void task3(taskId_t taskId, void *pArgs) {
  // Do some other things
}

setup() {
  ...
  // Run task1 every 5 seconds, starting on the next 'jockey'
  jockey.addTask(task1, NULL, 5000, true);

  // Run task2 every 1 second starting 1 second from now
  jockey.addTask(task2, NULL, 1000);
  
  // Run task3 every 3 seconds, starting 3 seconds from now, a total of 5 times
  jockey.addTask(task3, NULL, 3000, false, 5);  
}

loop() {
  // you can do other work here, but don't consume milliseconds or more of time
  ...
  jockey.runTasks();
}
```
As mentioned in the overview, `TaskJockey` is a cooperative, base-level task scheduler. Each task must not hog time by calling `delay`. In addition, the `loop()` function should also avoid calling `delay` for `TaskJockey` to work effectively. The intent of `TaskJockey` is for you to be able to manage longer delays via discrete, timed tasks rather than with inline delays.

You may call `TaskJockey` methods from within a scheduled task. This allows you to do two things:
* You can chain tasks by calling `addTask` from within another task. Be mindful of iterations and intervals, since you can otherwise fill up the `TaskJockey` task table with this. When chaining tasks, you want to keep iterations low (perhaps just 1) and intervals low. Generally, you want your total chained task executions to complete within the interval duration of the "parent" task. If you continually add more tasks than are killed, you will eventually fill up the task table (which is approximately 255 tasks) and `TaskJockey` will quietly stop adding them to the table.
* A task can conditionally terminate itself by calling `killTask` (with its `taskId`) under those conditions within the execution of the task.

You may also add the same task multiple times with different settings if needed. Within the task itself, you can track which instance of the task you are executing by comparing the task ids.

You may have interrupt handlers in your code along side `TaskJockey`, but since the `TaskJockey` code is not re-entrant, it is not gauranteed to operate properly if you call it from an interrupt handler. If you need to add or kill a task as a result of an interrupt action, it's recommended that you simply set a flag and then call `TaskJockey` at base level as indicated by the flag. `TaskJockey` is designed as a base-level scheduler for tasks that are not real-time critical but only require timing that is on the order of milliseconds without precise timing requirements.

Simple coding examples are provided in the repository illustrating these concepts.

## Final Notes and License
If you decide to try this library and find it useful, please let me know. I am happy to accept defect reports, or suggestions for improvement.

This software is covered by the MIT license as detailed in [LICENSE.txt](LICENSE.txt).
