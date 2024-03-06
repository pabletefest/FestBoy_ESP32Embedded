// /*
//   Sketch to show how a 4-bit Sprite is created, how to draw pixels
//   and text within the Sprite and then push the Sprite onto
//   the display screen.

//   The advantage of 4-bit sprites is:
//     1. Small memory footprint
//     2. Any set of 16 colours can be specified
//     3. Colours can be changed without redrawing in Sprite
//     4. Simple animations like flashing text can be achieved
//        by colour palette cycling and pushing sprite to TFT:
//        https://en.wikipedia.org/wiki/Color_cycling

//   Example for library:
//   https://github.com/Bodmer/TFT_eSPI

//   A Sprite is notionally an invisible graphics screen that is
//   kept in the processors RAM. Graphics can be drawn into the
//   Sprite just as it can be drawn directly to the screen. Once
//   the Sprite is completed it can be plotted onto the screen in
//   any position. If there is sufficient RAM then the Sprite can
//   be the same size as the screen and used as a frame buffer.

//   A 4-bit Sprite occupies (width * height)/2 bytes in RAM.

// */

// // Set delay after plotting the sprite
// #define DELAY 1000

// // Width and height of sprite
// #define WIDTH  128
// #define HEIGHT 128

// #include <Arduino.h>
// #include <SPI.h>
// #include <TFT_eSPI.h>                 // Include the graphics library (this includes the sprite functions)

// TFT_eSPI    tft = TFT_eSPI();         // Declare object "tft"

// TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object

// hw_timer_t *Timer0_Cfg = NULL;
// volatile int fps = 0;
 
// void IRAM_ATTR Timer0_ISR()
// {
//   Serial.println("ONE SECOND ELAPSED");
//   Serial.printf("FPS: %d\n", fps);
//   fps = 0;
// }

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println();

//   delay(500);

//   // Initialise the TFT registers
//   tft.init();

//   // Set the sprite colour depth to 4
//   spr.setColorDepth(4);

//   // Create a sprite of defined size
//   spr.createSprite(WIDTH, HEIGHT);

//   // Clear the TFT screen to blue
//   tft.fillScreen(TFT_BLUE);

//   Timer0_Cfg = timerBegin(0, 80, true);
//   timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
//   timerAlarmWrite(Timer0_Cfg, 1000000, true);
//   timerAlarmEnable(Timer0_Cfg);
// }

// void loop(void)
// {
//   // Fill the whole sprite with color 0 (Sprite is in memory so not visible yet)
//   spr.fillSprite(0);

//    // create a color map with known colors (16 maximum for 4-bit Sprite
//   uint16_t cmap[16];


//   cmap[0]  = TFT_BLACK; // We will keep this as black
//   cmap[1]  = TFT_NAVY;
//   cmap[2]  = TFT_DARKGREEN;
//   cmap[3]  = TFT_DARKCYAN;
//   cmap[4]  = TFT_MAROON;
//   cmap[5]  = TFT_PURPLE;
//   cmap[6]  = TFT_PINK;
//   cmap[7]  = TFT_LIGHTGREY;
//   cmap[8]  = TFT_YELLOW;
//   cmap[9]  = TFT_BLUE;
//   cmap[10] = TFT_GREEN;
//   cmap[11] = TFT_CYAN;
//   cmap[12] = TFT_RED;
//   cmap[13] = TFT_MAGENTA;
//   cmap[14] = TFT_WHITE;  // Keep as white for text
//   cmap[15] = TFT_BLUE;   // Keep as blue for sprite border

//   // Pass the palette to the Sprite class
//   spr.createPalette(cmap);

//   // Push Sprite partially off-screen to test cropping
//   spr.pushSprite(-40, -40);
//   spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2, 10);
//   spr.pushSprite(tft.width() - WIDTH + 40, tft.height() - HEIGHT + 40);

//   // Number of pixels to draw
//   uint16_t n = 100;

//   // Draw 100 random color pixels at random positions in sprite
//   while (n--)
//   {
//     uint16_t color = random(0x10); // Returns color 0 - 0x0F (i.e. 0-15)
//     int16_t x = random(WIDTH);     // Random x coordinate
//     int16_t y = random(HEIGHT);    // Random y coordinate
//     spr.drawPixel(x, y, color);    // Draw pixel in sprite
//   }

//   // Draw some lines
//   spr.drawLine(1, 0, WIDTH, HEIGHT-1, 10);
//   spr.drawLine(0, 0, WIDTH, HEIGHT, 10);
//   spr.drawLine(0, 1, WIDTH-1, HEIGHT, 10);
//   spr.drawLine(0, HEIGHT-1, WIDTH-1, 0, 12);
//   spr.drawLine(0, HEIGHT, WIDTH, 0, 12);
//   spr.drawLine(1, HEIGHT, WIDTH, 1, 12);

//  // Draw some text with Middle Centre datum
//   spr.setTextDatum(MC_DATUM);
//   spr.setTextColor(14); // White text
//   spr.drawString("Sprite", WIDTH / 2, HEIGHT / 2, 4);

//   // Now push the sprite to the TFT at 3 positions on screen
//   spr.pushSprite(-40, -40);
//   spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2);
//   spr.pushSprite(tft.width() - WIDTH + 40, tft.height() - HEIGHT + 40);

//   delay(DELAY * 4);

//   // create a new color map for colours 1-13 and use it instead
//   for (auto i = 1; i <= 13; i++)
//   {
//     cmap[i] = random(0x10000);
//   }

//   spr.createPalette(cmap, 16);
//   // Now push the sprite to the TFT at position 0,0 on screen

//   spr.pushSprite(-40, -40);
//   spr.pushSprite(tft.width() / 2 - WIDTH / 2, tft.height() / 2 - HEIGHT / 2);
//   spr.pushSprite(tft.width() - WIDTH + 40, tft.height() - HEIGHT + 40);

//   delay(DELAY);

//     // Fill TFT screen with blue
//   tft.fillScreen(TFT_BLUE);

//   // Draw a blue rectangle in sprite so when we move it 1 pixel it does not leave a trail
//   // on the blue screen background
//   spr.createPalette(cmap);

//   spr.drawRect(0, 0, WIDTH, HEIGHT, 15); // Blue rectangle

//   int x = tft.width() / 2  -  WIDTH / 2;
//   int y = tft.height() / 2 - HEIGHT / 2;

//   uint32_t updateTime = 0;       // time for next update

//   while (true)
//   {
//     // Random movement direction
//     int dx = 1; if (random(2)) dx = -1;
//     int dy = 1; if (random(2)) dy = -1;

//     // Pull it back onto screen if it wanders off
//     if (x < -WIDTH/2) dx = 1;
//     if (x >= tft.width()-WIDTH/2) dx = -1;
//     if (y < -HEIGHT/2) dy = 1;
//     if (y >= tft.height()-HEIGHT/2) dy = -1;

//     // Randomise the palette to change colours without redrawing
//     // the sprite
//     for (auto i = 1; i <= 13; i++)
//     {
//       cmap[i] = random(0x10000);
//     }
//     spr.createPalette(cmap); // Update sprite class palette

//     // Draw it 50 times, moving in random direct or staying still
//     n = 50;
//     int wait = random (50);
//     while (n)
//     {
//       if (updateTime <= millis())
//       {
//         // Use time delay so sprite does not move fast when not all on screen
//         updateTime = millis() + wait;

//         // Push the sprite to the TFT screen
//           spr.pushSprite(x, y);

//         // Change coord for next loop
//         x += dx;
//         y += dy;
//         n--;
//         yield(); // Stop watchdog reset

//         fps++;
//       }
//     }
//   } // Infinite while, will not exit!
// }

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
// #include <fstream>

// static gb::GBConsole emulator;
gb::GBConsole* emulator = nullptr;

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

  // if (!SPIFFS.exists("/text.txt"))
  // {
  //   Serial.println("File not found!");
  //   return;
  // }
  
  // Serial.println("\nFS lib initialized!");
  
  // File file = SPIFFS.open("/text.txt");

  // if(!file)
  // {
  //   Serial.println("Failed to open file for reading");
  //   return;
  // }

  // Serial.println("File opened!");

  // std::ifstream ifs;

  // ifs.open("/spiffs/text.txt", std::ifstream::binary);

  // if (ifs.is_open())
  //   Serial.println("File opened ifstream");
  // else
  //   Serial.println("File not opened ifstream");

  emulator = new gb::GBConsole();
  Ref<gb::GamePak> cartridge = std::make_shared<gb::GamePak>("Tetris V1.1.gb");

  emulator->insertCartridge(cartridge);
  emulator->reset();
}

void loop()
{
  // Serial.println("Executing loop");
  u32 startTime = millis();

  emulator->controllerState.buttons |= 0xF;
  emulator->controllerState.dpad |= 0xF;

  Dabble.processInput(); // This function is used to refresh data obtained from smartphone. Hence calling this function is mandatory in order to get data properly from your mobile.
  
  if (GamePad.isUpPressed())
  {
    emulator->controllerState.dpad &= ~0x4;
    Serial.println("UP pressed!");
  }

  if (GamePad.isDownPressed())
  {
    emulator->controllerState.dpad &= ~0x8;
    Serial.println("DOWN pressed!");
  }

  if (GamePad.isLeftPressed())
  {
    emulator->controllerState.dpad &= ~0x2;
    Serial.println("LEFT pressed!");
  }

  if (GamePad.isRightPressed())
  {
    emulator->controllerState.dpad &= ~0x1;
    Serial.println("RIGHT pressed!");
  }

  // if (GamePad.isSquarePressed())
  // {
  //   Serial.println("Square");
  // }

  if (GamePad.isCirclePressed())
  {
    emulator->controllerState.buttons &= ~0x1;
    Serial.println("A pressed!");
  }

  if (GamePad.isCrossPressed())
  {
    emulator->controllerState.buttons &= ~0x2;
    Serial.println("B pressed!");
  }

  // if (GamePad.isTrianglePressed())
  // {
  //   Serial.println("Triangle");
  // }

  if (GamePad.isStartPressed())
  {
    emulator->controllerState.buttons &= ~0x8;
    Serial.println("START pressed!");
  }

  if (GamePad.isSelectPressed())
  {
    emulator->controllerState.buttons &= ~0x4;
    Serial.println("SELECT pressed!");
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
}