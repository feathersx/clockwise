#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Clockface
#include <MarioFace.h>
#include <PacmanFace.h>
#include <Pockdexface.h>
#include <CustomizeFace.h>

//  Commons
#include <WiFiController.h>
#include <CWDateTime.h>
#include <CWPreferences.h>
#include <CWWebServer.h>
#include <StatusController.h>

#define MIN_BRIGHT_DISPLAY_ON 4
#define MIN_BRIGHT_DISPLAY_OFF 0

#define ESP32_LED_BUILTIN 2

MatrixPanel_I2S_DMA *dma_display = nullptr;

MarioFace *marioFace;
PacManface *pacManface;
PockDexFace *pockDexFace;
CustomizeFace *customizeFace;

WiFiController wifi;
CWDateTime cwDateTime;

bool autoBrightEnabled;
long autoBrightMillis = 0;
uint8_t currentBrightSlot = -1;
unsigned long lastMillis = 0;

void displaySetup(uint8_t swapRGB, uint8_t displayBright, uint8_t displayRotation, bool clockPhase)
{
  HUB75_I2S_CFG mxconfig(64, 64, 1);

  // uint8_t* rgb=[27,13,25,14,26,12];

  int rgbArray[6][6] = {
      {27, 13, 25, 14, 26, 12},
      {27, 13, 26, 12, 25, 14},
      {25, 14, 27, 13, 26, 12},
      {26, 12, 25, 14, 27, 13},
      {26, 12, 27, 13, 25, 14},
      {26, 12, 26, 12, 27, 13},
  };
  mxconfig.gpio.r1 = rgbArray[swapRGB][0];
  mxconfig.gpio.r2 = rgbArray[swapRGB][1];
  mxconfig.gpio.g1 = rgbArray[swapRGB][2];
  mxconfig.gpio.g2 = rgbArray[swapRGB][3];
  mxconfig.gpio.b1 = rgbArray[swapRGB][4];
  mxconfig.gpio.b2 = rgbArray[swapRGB][5];

  // if (swapRGB = 1)
  // {
  //   mxconfig.gpio.r1 = 27;
  //   mxconfig.gpio.r2 = 13;
  //   mxconfig.gpio.g1 = 25;
  //   mxconfig.gpio.g2 = 14;
  //   mxconfig.gpio.b1 = 26;
  //   mxconfig.gpio.b2 = 12;
  // }
  // else if (swapRGB = 2)
  // {
  //   mxconfig.gpio.r1 = 27;
  //   mxconfig.gpio.r2 = 13;
  //   mxconfig.gpio.g1 = 25;
  //   mxconfig.gpio.g2 = 14;
  //   mxconfig.gpio.b1 = 26;
  //   mxconfig.gpio.b2 = 12;
  // }

  mxconfig.gpio.e = 18;
  mxconfig.clkphase = clockPhase;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(displayBright);
  dma_display->clearScreen();
  dma_display->setRotation(displayRotation);
}

void automaticBrightControl()
{
  if (autoBrightEnabled)
  {
    if (millis() - autoBrightMillis > 3000)
    {
      int16_t currentValue = analogRead(ClockwiseParams::getInstance()->ldrPin);

      uint16_t ldrMin = ClockwiseParams::getInstance()->autoBrightMin;
      uint16_t ldrMax = ClockwiseParams::getInstance()->autoBrightMax;

      const uint8_t minBright = (currentValue < ldrMin ? MIN_BRIGHT_DISPLAY_OFF : MIN_BRIGHT_DISPLAY_ON);
      uint8_t maxBright = ClockwiseParams::getInstance()->displayBright;

      uint8_t slots = 10; // 10 slots
      uint8_t mapLDR = map(currentValue > ldrMax ? ldrMax : currentValue, ldrMin, ldrMax, 1, slots);
      uint8_t mapBright = map(mapLDR, 1, slots, minBright, maxBright);

      // Serial.printf("LDR: %d, mapLDR: %d, Bright: %d\n", currentValue, mapLDR, mapBright);
      if (abs(currentBrightSlot - mapLDR) >= 2 || mapBright == 0)
      {
        dma_display->setBrightness8(mapBright);
        currentBrightSlot = mapLDR;
        //  Serial.printf("setBrightness: %d , Update currentBrightSlot to %d\n", mapBright, mapLDR);
      }
      autoBrightMillis = millis();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(ESP32_LED_BUILTIN, OUTPUT);

  StatusController::getInstance()->blink_led(5, 100);

  ClockwiseParams::getInstance()->load();

  pinMode(ClockwiseParams::getInstance()->ldrPin, INPUT);

  displaySetup(ClockwiseParams::getInstance()->swapRGB, ClockwiseParams::getInstance()->displayBright,
               ClockwiseParams::getInstance()->displayRotation, ClockwiseParams::getInstance()->clockPhase);
  if (ClockwiseParams::getInstance()->themeName.equals("mario"))
  {
    marioFace = new MarioFace(dma_display);
  }
  else if (ClockwiseParams::getInstance()->themeName.equals("pacman"))
  {
    pacManface = new PacManface(dma_display);
  }
  else if (ClockwiseParams::getInstance()->themeName.equals("pockdex"))
  {
    pockDexFace = new PockDexFace(dma_display);
  }
  else
  {
    customizeFace = new CustomizeFace(dma_display);
  }

  autoBrightEnabled = (ClockwiseParams::getInstance()->autoBrightMax > 0);

  StatusController::getInstance()->clockwiseLogo();
  delay(1000);

  StatusController::getInstance()->wifiConnecting();
  if (wifi.begin())
  {
    StatusController::getInstance()->ntpConnecting();
    cwDateTime.begin(ClockwiseParams::getInstance()->timeZone.c_str(),
                     ClockwiseParams::getInstance()->use24hFormat,
                     ClockwiseParams::getInstance()->ntpServer.c_str(),
                     ClockwiseParams::getInstance()->manualPosix.c_str());

    if (ClockwiseParams::getInstance()->themeName.equals("mario"))
    {
      marioFace->setup(&cwDateTime);
    }
    else if (ClockwiseParams::getInstance()->themeName.equals("pacman"))
    {
      pacManface->setup(&cwDateTime);
    }
    else if (ClockwiseParams::getInstance()->themeName.equals("pockdex"))
    {
      pockDexFace->setup(&cwDateTime);
    }
    else
    {
      customizeFace->setup(&cwDateTime);
    }
  }
}

void loop()
{
  wifi.handleImprovWiFi();

  if (wifi.isConnected())
  {
    ClockwiseWebServer::getInstance()->handleHttpRequest();
    ezt::events();
  }

  if (wifi.connectionSucessfulOnce)
  {
    if (ClockwiseParams::getInstance()->themeName.equals("mario"))
    {
      marioFace->update();
    }
    else if (ClockwiseParams::getInstance()->themeName.equals("pacman"))
    {
      pacManface->update();
    }
    else if (ClockwiseParams::getInstance()->themeName.equals("pockdex"))
    {
      pockDexFace->update();
    }
    else
    {
      customizeFace->update();
    }
  }

  automaticBrightControl();
}
