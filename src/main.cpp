#include <Arduino.h>
#include <SPI.h>
// #include <LittleFS.h>
#include <SPIFFS.h>

#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

#include "gb.h"
#include "game_pack.h"
#include "ppu.h"

#include <iomanip>

#define SCREEN_WIDTH 480

// static gb::GBConsole emulator;
gb::GBConsole* emulator = nullptr;
static std::string gameName = "Tetris V1.1.gb";
static constexpr u8 textFont = 2;

void setup()
{
  delay(1000);
  Serial.begin(115200);
  Dabble.begin("FestBoy_ESP32"); // Set bluetooth name of your device
  // while(!Serial) { ; }

  if(!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  emulator = new gb::GBConsole();
  Ref<gb::GamePak> cartridge = std::make_shared<gb::GamePak>(gameName);

  emulator->insertCartridge(cartridge);
  emulator->reset();
  
  emulator->getPPU().printTextToDisplay(gameName, 1, 1, textFont);
}

void loop()
{
  // Serial.printf("CPU speed is %d MHz\n", getCpuFrequencyMhz());
  // Serial.printf("Current running thread id is %d\n", xPortGetCoreID());
  // Serial.println("Executing loop");
  u32 startTime = millis();

  emulator->controllerState.buttons |= 0xF;
  emulator->controllerState.dpad |= 0xF;

  Dabble.processInput(); // This function is used to refresh data obtained from smartphone. Hence calling this function is mandatory in order to get data properly from your mobile.
  
  if (GamePad.isUpPressed())
  {
    emulator->controllerState.dpad &= ~0x4;
    // Serial.println("UP pressed!");
  }

  if (GamePad.isDownPressed())
  {
    emulator->controllerState.dpad &= ~0x8;
    // Serial.println("DOWN pressed!");
  }

  if (GamePad.isLeftPressed())
  {
    emulator->controllerState.dpad &= ~0x2;
    // Serial.println("LEFT pressed!");
  }

  if (GamePad.isRightPressed())
  {
    emulator->controllerState.dpad &= ~0x1;
    // Serial.println("RIGHT pressed!");
  }

  // if (GamePad.isSquarePressed())
  // {
  //   Serial.println("Square");
  // }

  if (GamePad.isCirclePressed())
  {
    emulator->controllerState.buttons &= ~0x1;
    // Serial.println("A pressed!");
  }

  if (GamePad.isCrossPressed())
  {
    emulator->controllerState.buttons &= ~0x2;
    // Serial.println("B pressed!");
  }

  // if (GamePad.isTrianglePressed())
  // {
  //   Serial.println("Triangle");
  // }

  if (GamePad.isStartPressed())
  {
    emulator->controllerState.buttons &= ~0x8;
    // Serial.println("START pressed!");
  }

  if (GamePad.isSelectPressed())
  {
    emulator->controllerState.buttons &= ~0x4;
    // Serial.println("SELECT pressed!");
  }

  do
  {
    emulator->clock();
  } while (!emulator->getPPU().frameCompleted);

  // Serial.println("Frame finished");

  emulator->getPPU().frameCompleted = false;

  emulator->getPPU().drawFrameToDisplay();

  u32 endTime = millis();

  // Serial.printf("Elapsed time %dms\n", endTime - startTime);
  u16 elapsedTime = endTime - startTime;
  std::stringstream stream;
  stream << "Frame time: " << elapsedTime << "ms - FPS: " << std::fixed << std::setprecision(2) << (1000.f / elapsedTime);
  emulator->getPPU().printTextToDisplay(stream.str(), SCREEN_WIDTH - stream.str().size(), 1, textFont, TR_DATUM);
}