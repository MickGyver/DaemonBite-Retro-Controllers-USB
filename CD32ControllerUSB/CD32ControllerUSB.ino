/*  DaemonBite CD32 to USB Adapter
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

enum Mode
{
  TwoButton,
  CD32
};

// Controller DB9 pins (looking face-on to the end of the plug):
//
// 5 4 3 2 1
//  9 8 7 6
//
// Wire it up according to the following table:
//
// DB9    Arduino Pro Micro
// --------------------------------------
//  1     TXO PD3
//  2     RXI PD2
//  3     3   PD0
//  4     4   PD4
//  5     A0  PF7
//  6     6   PD7
//  7     VCC
//  8     GND
//  9     A1  PF6
//
// Connect a slide switch to pins GND,GND and 2 (PD1)

// Set up USB HID gamepad
Gamepad_ Gamepad;
bool usbUpdate = false; // Should gamepad data be sent to USB?

// Controller
uint8_t axes = 0;
uint8_t axesPrev = 0;
uint8_t buttons = 0;
uint8_t buttonsPrev = 0;

Mode mode = CD32;
Mode modePrev = mode;

void setup()
{
  // Setup switch pin (2, PD1)
  DDRD  &= ~B00000010; // input
  PORTD |=  B00000010; // high to enable internal pull-up

  // Setup controller pins
  DDRD  &= ~B10011101; // inputs
  PORTD |=  B10011101; // high to enable internal pull-up
  DDRF  &= ~B11000000; // input
  PORTF |=  B11000000; // high to enable internal pull-up
}

void loop()
{
  // Set mode from switch
  (PIND & B00000010) ? mode = CD32 : mode = TwoButton;

  // Read X and Y axes
  axes = ~(PIND & B00011101);

  switch(mode)
  {
    // Two button mode
    case TwoButton:
      buttons = ~( ((PIND & B10000000) >> 7) | ((PINF & B01000000) >> 5) | B11111100 );
      break;

    // CD32 button mode
    case CD32:

      // Set pin 6 (clock, PD7) and pin 5 (latch, PF7) as output low
      PORTD &= ~B10000000; // low to disable internal pull-up (will become low when set as output)
      DDRD  |=  B10000000; // output
      PORTF &= ~B10000000; // low to disable internal pull-up (will become low when set as output)
      DDRF  |=  B10000000; // output
      delayMicroseconds(40);

      // Clear buttons
      buttons = 0;

      // Read buttons
      (PINF & B01000000) ? buttons &= ~B00000010 : buttons |= B00000010; // Blue (2)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B00000001 : buttons |= B00000001; // Red (1)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B00001000 : buttons |= B00001000; // Yellow (4)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B00000100 : buttons |= B00000100; // Green (3)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B00100000 : buttons |= B00100000; // RTrig (6)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B00010000 : buttons |= B00010000; // LTrig (5)
      sendClock();
      (PINF & B01000000) ? buttons &= ~B01000000 : buttons |= B01000000; // Play (7)

      // Set pin 5 (latch, PF7) and pin 6 (clock, PD7) as input with pull-ups
      DDRF  &= ~B10000000; // input
      PORTF |=  B10000000; // high to enable internal pull-up
      DDRD  &= ~B10000000; // input
      PORTD |=  B10000000; // high to enable internal pull-up 
      delayMicroseconds(40);

      break;
  }

  // Has any buttons changed state?
  if (buttons != buttonsPrev)
  {
    Gamepad._GamepadReport.buttons = buttons;
    buttonsPrev = buttons;
    usbUpdate = true;
  }

  // Has any axes changed state?
  if (axes != axesPrev)
  {
    Gamepad._GamepadReport.Y = ((axes & B00000100) >> 2) - ((axes & B00001000) >> 3);
    Gamepad._GamepadReport.X = ((axes & B00010000) >> 4) - (axes & B00000001);
    axesPrev = axes;
    usbUpdate = true;
  }

  if(usbUpdate)
  {
    Gamepad.send();
    usbUpdate = false;
  }
}

void sendClock()
{
  // Send a clock pulse to pin 6 and wait
  PORTD |=  B10000000;
  delayMicroseconds(10);
  PORTD &= ~B10000000;
  delayMicroseconds(40);
}
