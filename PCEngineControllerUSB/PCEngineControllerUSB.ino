/*  DaemonBite PC Engine / TurboGrafx-16 controllers to USB Adapter
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

#define GAMEPAD_COUNT 2  // Set to 1 or 2 depending if you want to make a 1 or 2 port adapter
#define SELECT_PAUSE 20  // How many microseconds to wait after setting select/enable lines?
#define FRAME_TIME 16667 // The time of one "frame" in µs (used for turbo functionality)


#define UP       0x01
#define DOWN     0x04
#define LEFT     0x08
#define RIGHT    0x02
#define UP_SH    0
#define DOWN_SH  2
#define LEFT_SH  3
#define RIGHT_SH 1

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "PC Engine to USB";

/* -------------------------------------------------------------------------
PC Engine / TurboGrafx-16  controller wiring

PC Engine (P1)    Arduino Pro Micro
--------------------------------------
1 VCC             VCC
2 UP/I            3   PD0
3 RIGHT/II        2   PD1
4 DOWN/SELECT     RXI PD2
5 LEFT/START      TXO PD3
6 DSELECT         15  PB1 (Shared with P2)
7 ENABLE          14  PB3 (Shared with P2)
8 GND             GND

PC Engine (P2)    Arduino Pro Micro
--------------------------------------
1 VCC             VCC
2 UP/I            A3  PF4
3 RIGHT/II        A2  PF5
4 DOWN/SELECT     A1  PF6
5 LEFT/START      A0  PF7
6 DSELECT         15  PB1 (Shared with P1)
7 ENABLE          14  PB3 (Shared with P1)
8 GND             GND

------------------------------------------------------------------------- */

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controllers
uint8_t buttons[2][2]     = {{0,0},{0,0}};
uint8_t buttonsPrev[2][2] = {{0,0},{0,0}};
uint8_t gp = 0;

// Turbo timing
uint32_t microsNow = 0;
uint32_t microsEnable = 0;

void setup()
{ 
  // Set D0-D3 as inputs and enable pull-up resistors (port1 data pins)
  DDRD  &= ~B00001111;
  PORTD |=  B00001111;
 
  // Set F4-F7 as inputs and enable pull-up resistors (port2 data pins)
  DDRF  &= ~B11110000;
  PORTF |=  B11110000;

  // Set B1 and B3 as outputs and set them LOW
  PORTB &= ~B00001010;
  DDRB  |=  B00001010;
  
  // Wait for the controller(s) to settle
  delay(100);
}

void loop() { while(1)
{
  // Handle clock for turbo functionality
  microsNow = micros();
  if((microsNow-microsEnable) >= FRAME_TIME) 
  {
    PORTB |= B00001000;                      // Set enable pin HIGH to increase clock for turbo
    delayMicroseconds(SELECT_PAUSE);         // Wait a while...
    PORTB &= ~B00001000;                     // Set enable pin LOW again
    microsEnable = microsNow;
  }
 
  // Clear button data
  buttons[0][0]=0; buttons[0][1]=0;
  buttons[1][0]=0; buttons[1][1]=0;

  // Read all button and axes states
  PORTB |= B00000010;                        // Set SELECT pin HIGH
  delayMicroseconds(SELECT_PAUSE);           // Wait a while...
  buttons[0][0] = PIND & B00001111;          // Read DPAD for controller 1
  if(GAMEPAD_COUNT==2)
    buttons[1][0] = (PINF & B11110000) >> 4; // Read DPAD for controller 2
  PORTB &= ~B00000010;                       // Set SELECT pin LOW
  delayMicroseconds(SELECT_PAUSE);           // Wait a while...
  buttons[0][1] = PIND & B00001111;          // Read buttons for controller 1
  if(GAMEPAD_COUNT==2)
    buttons[1][1] = (PINF & B11110000) >> 4; // Read buttons for controller 2

  // Invert the readings so a 1 means a pressed button
  buttons[0][0] = ~buttons[0][0]; buttons[0][1] = ~buttons[0][1];
  buttons[1][0] = ~buttons[1][0]; buttons[1][1] = ~buttons[1][1];
  
  // Send data to USB if values have changed
  for(gp=0; gp<GAMEPAD_COUNT; gp++)
  {
    // Has any buttons changed state?
    if (buttons[gp][0] != buttonsPrev[gp][0] || buttons[gp][1] != buttonsPrev[gp][1] )
    {
      Gamepad[gp]._GamepadReport.buttons = buttons[gp][1];
      Gamepad[gp]._GamepadReport.Y = ((buttons[gp][0] & DOWN) >> DOWN_SH) - ((buttons[gp][0] & UP) >> UP_SH);
      Gamepad[gp]._GamepadReport.X = ((buttons[gp][0] & RIGHT) >> RIGHT_SH) - ((buttons[gp][0] & LEFT) >> LEFT_SH);
      buttonsPrev[gp][0] = buttons[gp][0];
      buttonsPrev[gp][1] = buttons[gp][1];
      Gamepad[gp].send();
    }
  }

}}