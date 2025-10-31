#include "CanDataHandler.h"
#include "DisplayHandler.h"
#include "EncoderHandler.h"
#include "StateManager.h"

#include <Arduino.h>
#include <Chrono.h>
#include <utility>
#include <vector>

// Uncomment for debugging without CAN connection
#define DEBUG 1

// HW specific definitions
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10

#define ENC_B 15
#define ENC_1 16
#define ENC_2 17

#define SCREEN_WIDTH 240
#define SCREEEN_HEIGHT 240

const int kCanBaud = 500000;          // Baud rate of the CAN bus
const int kDoubleClickSpeed = 300;    // Max time between clicks for double click in ms
const int kDisplayRefreshPeriod = 10; // Time between refresh in ms

CanDataHandler canDataHandler;
DisplayHandler displayHandler(TFT_RST, TFT_DC, TFT_CS, SCREEEN_HEIGHT, SCREEN_WIDTH);
EncoderHandler encoderHandler(ENC_1, ENC_2, ENC_B, kDoubleClickSpeed);

StateManager stateManager(encoderHandler, canDataHandler, displayHandler);

Chrono displayRefreshTimer;

void setup()
{
  Serial.begin(115200);

  displayHandler.displayStartupScreen();

  canDataHandler.initCan(kCanBaud);

  delay(500);
  displayHandler.clearScreen();
}

void loop()
{
  stateManager.poll();

  if (displayRefreshTimer.hasPassed(kDisplayRefreshPeriod))
  {
    stateManager.serveData();
    displayRefreshTimer.restart();
  }
}