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

#include "SegaController32U4.h"
#include "Gamepad.h"

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "Sega/C= to USB";

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
//  7      6  PD7
//  9     15  PB1

SegaController32U4 controller;

// Set up USB HID gamepad
Gamepad_ Gamepad;
bool usbUpdate = false; // Should gamepad data be sent to USB?

// Controller states
word currentState = 0;
word lastState = 1;

void setup()
{ 
}

void loop()
{
  currentState = controller.getStateMD();
  sendState();
}

void sendState()
{
  // Only report controller state if it has changed
  if (currentState != lastState)
  {
    Gamepad._GamepadReport.buttons = currentState >> 5;
    Gamepad._GamepadReport.Y = ((currentState & B00000100) >> 2) - ((currentState & B00000010) >> 1);
    Gamepad._GamepadReport.X = ((currentState & B00010000) >> 4) - ((currentState & B00001000) >> 3);
    Gamepad.send();
    lastState = currentState;
  }
}
