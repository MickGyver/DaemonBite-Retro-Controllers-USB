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

#define GAMEPAD_COUNT 2      // NOTE: No more than TWO gamepads are possible at the moment due to a USB HID issue.
#define GAMEPAD_COUNT_MAX 4  // NOTE: For some reason, can't have more than two gamepads without serial breaking. Can someone figure out why?
                             //       (It has something to do with how Arduino handles HID devices)
#define BUTTON_READ_DELAY 20 // Delay between button reads in µs
#define MICROS_LATCH      10 // 12µs according to specs (8 seems to work fine)
#define MICROS_CLOCK       5 //  6µs according to specs (4 seems to work fine)
#define MICROS_PAUSE       5 //  6µs according to specs (4 seems to work fine)

#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

#define NTT_CONTROL_BIT 0x20000000
#define NES_CONTROL_BIT 0xF3A0

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
  SNES,
  NTT
};

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controllers
uint32_t buttons[GAMEPAD_COUNT_MAX] = {0,0,0,0};
uint32_t buttonsPrev[GAMEPAD_COUNT_MAX] = {0,0,0,0};
uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B10000000,B01000000,B00100000,B00010000};
ControllerType controllerType[GAMEPAD_COUNT_MAX] = {NONE,NONE,NONE,NONE};
uint32_t btnBits[32] = {0x10,0x40,0x400,0x800,UP,DOWN,LEFT,RIGHT,0x20,0x80,0x100,0x200,          // Standard SNES controller
                        0x10000000,0x20000000,0x40000000,0x80000000,0x1000,0x2000,0x4000,0x8000, // NTT Data Keypad (NDK10)
                        0x10000,0x20000,0x40000,0x80000,0x100000,0x200000,0x400000,0x800000,
                        0x1000000,0x2000000,0x4000000,0x8000000};
uint8_t gp = 0;
uint8_t buttonCount = 32;

// Timing
uint32_t microsButtons = 0;

void setup()
{
  // Setup latch and clock pins (2,3 or PD1, PD0)
  DDRD  |=  B00000011; // output
  PORTD &= ~B00000011; // low

  // Setup data pins (A0-A3 or PF7-PF4)
  DDRF  &= ~B11110000; // inputs
  PORTF |=  B11110000; // enable internal pull-ups

  delay(500);
  detectControllerTypes();
}

void loop() { while(1)
{
  // See if enough time has passed since last button read
  if((micros() - microsButtons) > BUTTON_READ_DELAY)
  {    
    // Pulse latch
    sendLatch();

    for(uint8_t btn=0; btn<buttonCount; btn++)
    {
      for(gp=0; gp<GAMEPAD_COUNT; gp++) 
        (PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
      sendClock();
    }

    // Check gamepad type
    for(gp=0; gp<GAMEPAD_COUNT; gp++) 
    {
      if(controllerType[gp] == NES) {    // NES
        bitWrite(buttons[gp], 5, bitRead(buttons[gp], 4));
        bitWrite(buttons[gp], 4, bitRead(buttons[gp], 6));
        buttons[gp] &= 0xC3F;
      }
      else if(controllerType[gp] == NTT) // SNES NTT Data Keypad
        buttons[gp] &= 0x3FFFFFF;
      else                               // SNES Gamepad
        buttons[gp] &= 0xFFF; 
    }

    for(gp=0; gp<GAMEPAD_COUNT; gp++)
    {
      // Has any buttons changed state?
      if (buttons[gp] != buttonsPrev[gp])
      {
        Gamepad[gp]._GamepadReport.buttons = (buttons[gp] >> 4); // First 4 bits are the axes
        Gamepad[gp]._GamepadReport.Y = ((buttons[gp] & DOWN) >> 1) - (buttons[gp] & UP);
        Gamepad[gp]._GamepadReport.X = ((buttons[gp] & RIGHT) >> 3) - ((buttons[gp] & LEFT) >> 2);
        buttonsPrev[gp] = buttons[gp];
        Gamepad[gp].send();
      }
    }

    microsButtons = micros();
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
        (PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
      sendClock();
    }

    // Check controller types and set buttonCount to max needed
    for(gp=0; gp<GAMEPAD_COUNT; gp++) 
    {
      if(buttons[gp] & NES_CONTROL_BIT) {   // NES
        if(controllerType[gp] != SNES && controllerType[gp] != NTT)
          controllerType[gp] = NES;
        if(buttonCountNew < 8)
          buttonCountNew = 8;
      }
      else if(buttons[gp] & NTT_CONTROL_BIT) { // SNES NTT Data Keypad
        controllerType[gp] = NTT;
        buttonCountNew = 32;
      }
      else {                                   // SNES Gamepad
        if(controllerType[gp] != NTT)
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
  delayMicroseconds(MICROS_LATCH);
  PORTD &= ~B00000010; // Set LOW
  delayMicroseconds(MICROS_PAUSE); 
}

void sendClock()
{
  // Send a clock pulse to (S)NES controller(s)
  PORTD |=  B10000001; // Set HIGH
  delayMicroseconds(MICROS_CLOCK);
  PORTD &= ~B10000001; // Set LOW
  delayMicroseconds(MICROS_PAUSE); 
}
