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

#define GAMEPAD_COUNT 2     // NOTE: No more than TWO gamepads are possible at the moment due to a USB HID issue.

#define BUTTON_READ_DELAY 300 // Button read delay in µs


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
// D1   (GP3: DATA)          A2  (PF5, Gamepad 3)
// D1   (GP4: DATA)          A3  (PF4, Gamepad 4)

const byte LATCH = 2;
const byte CLOCK = 3;
const byte DIN[4] = {A0,A1,A2,A3};

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controllers
uint32_t buttons[4] = {0,0,0,0};
uint32_t buttons_old[4] = {0,0,0,0};
uint8_t gp = 0;

// Timing
unsigned long microsButtons = 0;

void setup()
{
  for(gp=0; gp<4; gp++) pinMode(DIN[gp], INPUT_PULLUP);

  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);

  digitalWrite(LATCH, LOW);
  digitalWrite(CLOCK, LOW);

  for(gp=0; gp<GAMEPAD_COUNT; gp++) Gamepad[gp].reset();
}

void loop()
{
  // See if enough time has passed since last button read
  if((micros()-microsButtons) > BUTTON_READ_DELAY)
  {
    sendLatch();

    for(int i=0; i<32; i++)
    {
      for(gp=0; gp<4; gp++) buttons[gp] = (buttons[gp]>>1) | (((uint32_t)~digitalRead(DIN[gp]))<<31);
      sendClock();
    }

    microsButtons = micros();

    for(gp=0; gp<4; gp++)
    {
      if((buttons[gp] & 0xFF00) == 0xFF00) buttons[gp] &= 0xFF; // NES pad
      else if((buttons[gp] & 0xF000) == 0x2000) buttons[gp] = (buttons[gp] & 0xFFF) | ((buttons[gp]>>4) & 0xFFFF000); // SNES NTT;
      else buttons[gp] &= 0xFFF; // SNES generic
    }

    for(gp=0; gp<GAMEPAD_COUNT; gp++)
    {
      // Has any buttons changed state?
      if (buttons[gp] != buttons_old[gp])
      {
        buttons_old[gp] = buttons[gp];
        Gamepad[gp]._GamepadReport.buttons = (buttons[gp] & 0xF) | ((buttons[gp]>>4) & ~0xF);
        Gamepad[gp]._GamepadReport.hat = dpad2hat(buttons[gp]);
        Gamepad[gp].send();
      }
    }
  }
}

void sendLatch()
{
  // Send a latch pulse to (S)NES controller(s)
  digitalWrite(LATCH, HIGH);
  delayMicroseconds(12);
  digitalWrite(LATCH, LOW);
  delayMicroseconds(6); 
}

void sendClock()
{
  // Send a clock pulse to (S)NES controller(s)
  digitalWrite(CLOCK, HIGH);
  delayMicroseconds(6);
  digitalWrite(CLOCK, LOW);
  delayMicroseconds(6);
}

#define UP    0x10
#define DOWN  0x20
#define LEFT  0x40
#define RIGHT 0x80

uint8_t dpad2hat(uint8_t dpad)
{
  switch(dpad & (UP|DOWN|LEFT|RIGHT))
  {
    case UP:         return 0;
    case UP|RIGHT:   return 1;
    case RIGHT:      return 2;
    case DOWN|RIGHT: return 3;
    case DOWN:       return 4;
    case DOWN|LEFT:  return 5;
    case LEFT:       return 6;
    case UP|LEFT:    return 7;
  }
  return 15;
}
