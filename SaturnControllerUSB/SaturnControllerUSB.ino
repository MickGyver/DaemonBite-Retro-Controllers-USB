/*  DaemonBite Saturn USB Adapter
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

#define GAMEPAD_COUNT 2 // Set to 1 or 2 depending if you want to make a 1 or 2 port adapter
#define SELECT_PAUSE  3 // How many microseconds to wait after setting select lines? (2µs is enough according to the Saturn developer's manual)
                        // 20µs is required for Retrobit wired controllers
//#define RETROBIT      // Uncomment to support the Retro Bit 2.4GHz controller (this will increase lag a lot)
 
#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "SATURN to USB";

/* -------------------------------------------------------------------------
Saturn controller socket (looking face-on at the front of the socket):
___________________
/ 1 2 3 4 5 6 7 8 9 \
|___________________|

Saturn controller plug (looking face-on at the front of the controller plug):
___________________
/ 9 8 7 6 5 4 3 2 1 \
|___________________|

Saturn (P1)    Arduino Pro Micro
--------------------------------------
1 VCC         VCC
2 DATA1       2   PD1
3 DATA0       3   PD0
4 SEL1        15  PB1 (Shared with P2)
5 SEL0        14  PB3 (Shared with P2)
6 TL (5V)     4   PD4
7 DATA3       TXO PD3
8 DATA2       RXI PD2
9 GND         GND

Saturn (P2)    Arduino Pro Micro
--------------------------------------
1 VCC         VCC
2 DATA1       A2  PF5
3 DATA0       A3  PF4
4 SEL1        15  PB1 (Shared with P1)
5 SEL0        14  PB3 (Shared with P1)
6 TL (5V)     6   PD7
7 DATA3       A0  PF7
8 DATA2       A1  PF6
9 GND         GND

NOTE: The receiver of the Retro Bit 2.4GHz controller needs to be plugged
      in after the adapter has been connected to USB and the RETROBIT
      define needs to be uncommented.
------------------------------------------------------------------------- */

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controllers
uint8_t buttons[2][2]     = {{0,0},{0,0}};
uint8_t buttonsPrev[2][2] = {{0,0},{0,0}};
uint8_t gp = 0;

void setup()
{ 
  // Set D0-D3 as inputs and enable pull-up resistors (port1 data pins)
  DDRD  &= ~B00001111;
  PORTD |=  B00001111;
 
  // Set F4-F7 as inputs and enable pull-up resistors (port2 data pins)
  DDRF  &= ~B11110000;
  PORTF |=  B11110000;

  // Set D4 and D7 as inputs and enable pull-up resistors (port1/2 TL)
  DDRD  &= ~B10010000;
  PORTD |=  B10010000;

  // Set B1 and B3 as outputs and set them HIGH (select pins)
  PORTD |=  B00001010; 
  DDRB  |=  B00001010;

  // Wait for the controller(s) to settle
  delay(100);
}

void loop() { while(1)
{
  // Clear button data
  buttons[0][0]=0; buttons[0][1]=0;
  buttons[1][0]=0; buttons[1][1]=0;

  // Read all button and axes states
  read3();
  read2();
  read1();
  read4();

  // Invert the readings so a 1 means a pressed button
  buttons[0][0] = ~buttons[0][0]; buttons[0][1] = ~buttons[0][1];
  buttons[1][0] = ~buttons[1][0]; buttons[1][1] = ~buttons[1][1];
  
  // Send data to USB if values have changed
  for(gp=0; gp<GAMEPAD_COUNT; gp++)
  {
    // Has any buttons changed state?
    if (buttons[gp][0] != buttonsPrev[gp][0] || buttons[gp][1] != buttonsPrev[gp][1] )
    {
      Gamepad[gp]._GamepadReport.buttons = buttons[gp][1] | ((buttons[gp][0] & 0x80)<<1);
      Gamepad[gp]._GamepadReport.Y = ((buttons[gp][0] & DOWN) >> 1) - (buttons[gp][0] & UP);
      Gamepad[gp]._GamepadReport.X = ((buttons[gp][0] & RIGHT) >> 3) - ((buttons[gp][0] & LEFT) >> 2);
      buttonsPrev[gp][0] = buttons[gp][0];
      buttonsPrev[gp][1] = buttons[gp][1];
      Gamepad[gp].send();
    }
  }

  #ifdef RETROBIT
    // This delay is needed for the retro bit 2.4GHz wireless controller, making it more or less useless with this adapter
    delay(17);
  #endif

}}

// Read R, X, Y, Z
void read1()
{
  PORTB &= ~B00001010; // Set select outputs to 00
  delayMicroseconds(SELECT_PAUSE);
  buttons[0][1] |= (PIND & 0x0f) << 4;
  if(GAMEPAD_COUNT == 2)
    buttons[1][1] |= (PINF & 0xf0);
}
  
// Read ST, A, C, B
void read2()
{
  PORTB ^= B00001010; // Toggle select outputs (01->10 or 10->01)
  delayMicroseconds(SELECT_PAUSE);
  buttons[0][1] |= (PIND & 0x0f);
  if(GAMEPAD_COUNT == 2)
    buttons[1][1] |= (PINF & 0xf0)>>4;
}

// Read DR, DL, DD, DU  
void read3()
{
  PORTB ^= B00000010; // Set select outputs to 10 from 11 (toggle)
  delayMicroseconds(SELECT_PAUSE);
  buttons[0][0] |= (PIND & 0x0f);
  if(GAMEPAD_COUNT == 2)
    buttons[1][0] |= (PINF & 0xf0) >> 4;
}
  
// Read L, *, *, *
void read4()
{
  PORTB |= B00001010; // Set select outputs to 11
  delayMicroseconds(SELECT_PAUSE);
  buttons[0][0] |= (PIND & 0x0f) << 4;
  if(GAMEPAD_COUNT == 2)
    buttons[1][0] |= (PINF & 0xf0);
}