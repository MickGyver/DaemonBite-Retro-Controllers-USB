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
//  7      7  PE6
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

// Controller previous states
word lastState[2] = {1,1};

void setup()
{
  for(byte gp=0; gp<=1; gp++)
    Gamepad[gp].reset();
}

void loop() { while(1)
{
  controllers.readState();
  sendState(0);
  sendState(1);
}}

void sendState(byte gp)
{
  // Only report controller state if it has changed
  if (controllers.currentState[gp] != lastState[gp])
  {
    Gamepad[gp]._GamepadReport.buttons = controllers.currentState[gp] >> 4;
    Gamepad[gp]._GamepadReport.Y = ((controllers.currentState[gp] & SC_BTN_DOWN) >> SC_BIT_SH_DOWN) - ((controllers.currentState[gp] & SC_BTN_UP) >> SC_BIT_SH_UP);
    Gamepad[gp]._GamepadReport.X = ((controllers.currentState[gp] & SC_BTN_RIGHT) >> SC_BIT_SH_RIGHT) - ((controllers.currentState[gp] & SC_BTN_LEFT) >> SC_BIT_SH_LEFT);
    Gamepad[gp].send();
    lastState[gp] = controllers.currentState[gp];
  }
}
