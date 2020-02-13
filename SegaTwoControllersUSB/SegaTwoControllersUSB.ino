/*  DaemonBite Sega USB Adapter
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

#include "SegaControllers32U4.h"
#include "Gamepad.h"

// Controller DB9 pins (looking face-on to the end of the plug):
//
// 5 4 3 2 1
//  9 8 7 6
//
// Connect pin 5 to +5V and pin 8 to GND
// Connect the remaining pins to digital I/O pins (see below)
// DB9    Arduino Pro Micro
// --------------------------------------
//  1     A0  PF7
//  2     A1  PF6
//  3     A2  PF5
//  4     A3  PF4
//  6     14  PB3
//  7     16  PB2 (6  PD7)
//  9     15  PB1

//  1     TXO PD3
//  2     RXI PD2
//  3      2  PD1
//  4      3  PD0
//  6      4  PD4
//  7      5  PC6
//  9      6  PD7

SegaControllers32U4 controllers;

// Set up USB HID gamepads
Gamepad_ Gamepad[2];
bool usbUpdate[2] = {false,false}; // Should gamepad data be sent to USB?

// Controller states
word currentState[2] = {0,0};
word lastState[2] = {1,1};

void setup()
{
  Gamepad[0].reset();
  Gamepad[1].reset();
}

void loop()
{
  currentState[0] = controllers.getStateMD1();
  sendState(0);
  currentState[1] = controllers.getStateMD2();
  sendState(1);
}

void sendState(byte gp)
{
  // Only report controller state if it has changed
  if (currentState[gp] != lastState[gp])
  {
    Gamepad[gp]._GamepadReport.buttons = currentState[gp] >> 5;
    Gamepad[gp]._GamepadReport.Y = ((currentState[gp] & B00000100) >> 2) - ((currentState[gp] & B00000010) >> 1);
    Gamepad[gp]._GamepadReport.X = ((currentState[gp] & B00010000) >> 4) - ((currentState[gp] & B00001000) >> 3);
    Gamepad[gp].send();
    lastState[gp] = currentState[gp];
  }
}
