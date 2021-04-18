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
 * This example program demonstrates using TaskJuggler for chained and
 * self-killing tasks. This program assumes you have a button hooked up to
 * go HIGH on digital I/O pin 3 when the button is pressed. Every 5 seconds,
 * The program will blink the built-in LED 3 times quickly (125ms pulses).
 * It will do this indefinitely until the button is pressed, after which it
 * will stop.
 *
 * See the included README.md for additional details.
 *
 *  https://github.com/mbratch/TaskJuggler
 */
 /*************************************************************************/
#include <TaskJockey.h>

TaskJockey jockey;

volatile bool buttonPressed = false;

void ledFlashOff(taskId_t taskId) {
  digitalWrite(*(int8_t *)jockey.getTaskArgs(taskId), LOW);
}

void ledFlashOn(taskId_t taskId) {
  uint8_t *pLedPin = (uint8_t *)jockey.getTaskArgs(taskId);
  digitalWrite(*pLedPin, HIGH);
  jockey.addTask(ledFlashOff, pLedPin, 125, -1, 1); // Turn off the LED once after 125 ms
}

void ledFlashSequence(taskId_t taskId) {
  if (buttonPressed)
    jockey.killTask(taskId);
  else
    jockey.addTask(ledFlashOn, jockey.getTaskArgs(taskId), 250, 0, 3); // Run the LED flash ON 3 times, once very 250ms
}

// Simplistic button press without debounce
void buttonPress(void) {
  // NOTE: do not call TaskJuggler from an interrupt handler
  buttonPressed = true;
}

void setup() {
  static uint8_t ledPin = LED_BUILTIN;

  const byte ButtonIn = 3;      // Pin 3 goes high when button is pressed

  // Set up a button press interrupt
  pinMode(ButtonIn, INPUT);
  attachInterrupt(digitalPinToInterrupt(ButtonIn), buttonPress, RISING);

  pinMode(ledPin, OUTPUT);

  jockey.addTask(ledFlashSequence, &ledPin, 5000); // Execute the LED flash sequence every 5 seconds

  // Serial port for tracing/debugging
  Serial.begin(115200);
}

void loop() {
  jockey.runTasks();
}
