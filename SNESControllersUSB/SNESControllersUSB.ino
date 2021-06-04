/*  DaemonBite (S)NES Controllers to USB Adapter
 *  Author: Mikael Norrgård <mick@daemonbite.com>
 *
 *  Copyright (c) 2020 Mikael Norrgård <http://daemonbite.com>
 *  
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 */

#include "Gamepad.h"

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "NES/SNES to USB";

//#define DEBUG

#define GAMEPAD_COUNT      2 // NOTE: To have more than 2 two gamepads you need to disable the CDC of the Arduino, there is a specific project for that.
#define GAMEPAD_COUNT_MAX  2  
#define BUTTON_READ_DELAY 20 // Delay between button reads in µs
#define CYCLES_LATCH     128 // 128 12µs according to specs (8 seems to work fine) (1 cycle @ 16MHz takes 62.5ns so 62.5ns * 128 = 8000ns = 8µs)
#define CYCLES_CLOCK      64 // 6µs according to specs (4 seems to work fine)
#define CYCLES_PAUSE1     64 // 6µs according to specs (4 seems to work fine)
#define CYCLES_PAUSE2     58 // 6µs according to specs (4 seems to work fine)

#define BUTTONS  0
#define AXES     1
#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

#define DELAY_CYCLES(n) __builtin_avr_delay_cycles(n)

inline void sendLatch() __attribute__((always_inline));
inline void sendClock() __attribute__((always_inline));

// Wire it all up according to the following table:
//
// NES           SNES        Arduino Pro Micro
// --------------------------------------
// VCC                       VCC (All gamepads)
// GND                       GND (All gamepads)
// OUT0 (LATCH)              2   (PD1, All gamepads)
// CUP  (CLOCK)              3   (PD0, All gamepads)
// D1   (GP1: DATA)          A0  (PF7, Gamepad 1) 
// D1   (GP2: DATA)          A1  (PF6, Gamepad 2)
// D1   (GP3: DATA)          A2  (PF5, Gamepad 3, not currently used)
// D1   (GP4: DATA)          A3  (PF4, Gamepad 4, not currently used)

enum ControllerType {
  NONE,
  NES,
  SNES
};

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controllers
uint8_t buttons[GAMEPAD_COUNT_MAX][2] = {{0,0},{0,0}};
uint8_t buttonsPrev[GAMEPAD_COUNT_MAX][2] = {{0,0},{0,0}};
uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B10000000,B01000000};
ControllerType controllerType[GAMEPAD_COUNT_MAX] = {NONE,NONE};
uint8_t btnByte[12] = {0,0,0,0,1,1,1,1,0,0,0,0};
uint8_t btnBits[12] = {0x01,0x04,0x40,0x80,UP,DOWN,LEFT,RIGHT,0x02,0x08,0x10,0x20};
uint8_t gp = 0;
uint8_t buttonCount = 12;

// Timing
uint32_t microsButtons = 0;

#ifdef DEBUG
uint32_t microsStart = 0;
uint32_t microsEnd = 0;
uint8_t counter = 0;
#endif

void setup()
{
  // Setup latch and clock pins (2,3 or PD1, PD0)
  DDRD  |=  B00000011; // output
  PORTD &= ~B00000011; // low

  // Setup data pins A0-A3 (PF7-PF4)
  DDRF  &= ~B11110000; // inputs
  PORTF |=  B11110000; // enable internal pull-ups

  #ifdef DEBUG
  Serial.begin(115200);
  delay(2000);
  #endif

  delay(300);
  detectControllerTypes();
}

void loop() { while(1)
{
  // See if enough time has passed since last button read
  if((micros() - microsButtons) > BUTTON_READ_DELAY)
  {    

    #ifdef DEBUG
    microsStart = micros();
    #endif
  
    // Pulse latch
    sendLatch();

    for(gp=0; gp<GAMEPAD_COUNT; gp++) {
      buttons[gp][BUTTONS] = 0;
      buttons[gp][AXES] = 0;
    }

    for(uint8_t btn=0; btn<buttonCount; btn++)
    {
      for(gp=0; gp<GAMEPAD_COUNT; gp++) {
        if((PINF & gpBit[gp])==0) buttons[gp][btnByte[btn]] |= btnBits[btn];
      }
      sendClock();
    }

    // Check gamepad type
    for(gp=0; gp<GAMEPAD_COUNT; gp++) 
    {
      if(controllerType[gp] == NES) {    // NES
        bitWrite(buttons[gp][BUTTONS], 1, bitRead(buttons[gp][BUTTONS], 0));
        bitWrite(buttons[gp][BUTTONS], 0, bitRead(buttons[gp][BUTTONS], 2));
        buttons[gp][BUTTONS] &= 0xC3;
      }
    }

    for(gp=0; gp<GAMEPAD_COUNT; gp++)
    {
      // Has any buttons changed state?
      if (buttons[gp][BUTTONS] != buttonsPrev[gp][BUTTONS] || buttons[gp][AXES] != buttonsPrev[gp][AXES])
      {
        Gamepad[gp]._GamepadReport.buttons = buttons[gp][BUTTONS];
        Gamepad[gp]._GamepadReport.Y = ((buttons[gp][AXES] & DOWN) >> 1) - (buttons[gp][AXES] & UP);
        Gamepad[gp]._GamepadReport.X = ((buttons[gp][AXES] & RIGHT) >> 3) - ((buttons[gp][AXES] & LEFT) >> 2);
        buttonsPrev[gp][BUTTONS] = buttons[gp][BUTTONS];
        buttonsPrev[gp][AXES] = buttons[gp][AXES];
        Gamepad[gp].send();
      }
    }
    
    microsButtons = micros();

    #ifdef DEBUG
    microsEnd = micros();
    if(counter < 20) {
      Serial.println(microsEnd-microsStart);
      counter++;
    }
    #endif
    
  }
}}

void detectControllerTypes()
{
  uint8_t buttonCountNew = 0;

  // Read the controllers a few times to detect controller type
  for(uint8_t i=0; i<4; i++) 
  {
    // Pulse latch
    sendLatch();

    // Read all buttons
    for(uint8_t btn=0; btn<buttonCount; btn++)
    {
      for(gp=0; gp<GAMEPAD_COUNT; gp++) 
        (PINF & gpBit[gp]) ? buttons[gp][btnByte[btn]] &= ~btnBits[btn] : buttons[gp][btnByte[btn]] |= btnBits[btn];
      sendClock();
    }

    // Check controller types and set buttonCount to max needed
    for(gp=0; gp<GAMEPAD_COUNT; gp++) 
    {
      if((buttons[gp][0] & 0xF3A) == 0xF3A) {   // NES
        if(controllerType[gp] != SNES)
          controllerType[gp] = NES;
        if(buttonCountNew < 8)
          buttonCountNew = 8;
      }
      else {                                      // SNES Gamepad
        controllerType[gp] = SNES;
        if(buttonCountNew < 12)
          buttonCountNew = 12;
      }
    }
  }

  // Set updated button count to avoid unneccesary button reads (for simpler controller types)
  buttonCount = buttonCountNew;
}

void sendLatch()
{
  // Send a latch pulse to (S)NES controller(s)
  PORTD |=  B00000010; // Set HIGH
  DELAY_CYCLES(CYCLES_LATCH); 
  PORTD &= ~B00000010; // Set LOW
  DELAY_CYCLES(CYCLES_PAUSE2);
}

void sendClock()
{
  // Send a clock pulse to (S)NES controller(s)
  PORTD |=  B10000001; // Set HIGH
  DELAY_CYCLES(CYCLES_CLOCK); 
  PORTD &= ~B10000001; // Set LOW
  DELAY_CYCLES(CYCLES_PAUSE1); 
}
