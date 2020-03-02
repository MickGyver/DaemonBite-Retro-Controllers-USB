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

#define BUTTON_READ_DELAY 300 // Button read delay in µs

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
//  3      3  PD0
//  4      4  PD4
//  5     A0  PF7
//  6      6  PD7 (Important: Connect this pin via a 220Ω resistor!)
//  7     VCC
//  8     GND
//  9     A1  PF6
// -----------------
// (Second controller port for future reference)
//  1     15  PB1
//  2     14  PB3
//  3      2  PD1
//  4      5  PC6
//  5     A2  PF5
//  6      7  PE6 (Important: Connect this pin via a 220Ω resistor!)
//  7     VCC
//  8     GND
//  9     A3  PF4


// Set up USB HID gamepad
Gamepad_ Gamepad;
bool usbUpdate = false; // Should gamepad data be sent to USB?

// Controller
uint8_t axes = 0;
uint8_t axesPrev = 0;
uint8_t buttons = 0;
uint8_t buttonsPrev = 0;

// Timing
long microsNow = 0;
long microsButtons = 0;

// CD32 controller detection
uint8_t detection = 0;

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
  // Get current time
  microsNow = micros();

  // Read X and Y axes
  axes = ~(PIND & B00011101);

  // See if enough time has passed since last button read
  if(microsNow > microsButtons+BUTTON_READ_DELAY)
  {
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
    sendClock();
    (PINF & B01000000) ? detection |= B00000001 : detection &= ~B00000001; // First detection bit (should be 1)
    sendClock();
    (PINF & B01000000) ? detection |= B00000010 : detection &= ~B00000010; // Second detection bit (should be 0)

    // Set pin 5 (latch, PF7) and pin 6 (clock, PD7) as input with pull-ups
    DDRF  &= ~B10000000; // input
    PORTF |=  B10000000; // high to enable internal pull-up
    DDRD  &= ~B10000000; // input
    PORTD |=  B10000000; // high to enable internal pull-up 
    delayMicroseconds(40);

    // Was a CD32 gamepad detected? If not, read button 1 and 2 "normally".
    if(detection != B0000001)
      buttons = ~( ((PIND & B10000000) >> 7) | ((PINF & B01000000) >> 5) | B11111100 );
    
    microsButtons = microsNow+400;
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

  // Update USB data if necessary
  if(usbUpdate)
  {
    Gamepad.send();
    usbUpdate = false;
  }
}

void sendClock()
{
  // Send a clock pulse to pin 6 and wait
  PORTD |=  B10000000; // Enable pull-up
  delayMicroseconds(10);
  PORTD &= ~B10000000; // Disable pull-up
  delayMicroseconds(40); 
}
