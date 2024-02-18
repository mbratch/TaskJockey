/*************************************************************************/
/*!
 * S7Display Library - Example program
 *
 * @file: juggler_led.ino
 *
 * Copyright (c) 2021 Mark Bratcher
 *
 * License: MIT (per LICENSE.txt)
 *
 * This example program runs simple TaskJockey tests that display information
 * on the serial monitor.
 *
 * See the included README.md for additional details.
 *
 *  https://github.com/mbratch/TaskJuggler
 */
 /*************************************************************************/
#include <TaskJockey.h>

TaskJockey jockey;

uint32_t time;

void task5(taskId_t taskId) {
  Serial.print("Task 2 sub 3 sub 5 ");
  Serial.println(millis() - time);
}

void task3(taskId_t taskId) {
  Serial.print("Task 2 sub 3 ");
  Serial.println(millis() - time);
  jockey.addTask(task5, NULL, 500, 0, 3);
}

void task2(taskId_t taskId) {
  Serial.print("Task 2 ");
  Serial.println(millis() - time);
  jockey.addTask(task3, NULL, 2000, 0, 3);
}

void task4(taskId_t taskId) {
  Serial.print("Task 1 sub 4 ");
  Serial.println(millis() - time);
}

void task1(taskId_t taskId) {
  static uint8_t n = 8;

  // A counter is used to kill the task just for demonstration purposes
  // The better way to do this would be to use the 'iterations' parameter
  // when this task is created.
  if (n == 0)
    jockey.killTask(taskId);
  else {
    Serial.print("task 1 ");
    Serial.println(millis() - time);
    jockey.addTask(task4, NULL, 1000, 1000, 3);
    n -= 1;
  }
}

void setup() {
  Serial.begin(115200);
  jockey.addTask(task1, NULL, 5000);
  jockey.addTask(task2, NULL, 10000);
  time = millis();
}

void loop() {
  jockey.runTasks();
}
