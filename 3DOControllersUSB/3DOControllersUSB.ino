/*  DaemonBite 3DO Controllers to USB Adapter
 *  
 *  Based on DaemonBite NES / SNES Adapter by Mikael Norrgård <mick@daemonbite.com>
 *
 *  Author: Chris Chaplin
 *  
 *  Copyright (c) 2022 Chris Chaplin
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

// The bulk of the information on this comes from http://kaele.com/~kashima/games/3do-e.html
// and https://github.com/fluxcorenz/UPCB/blob/master/3do.h
//
// Controller DB9 pins (looking face-on to the end of the plug):
//
// 5 4 3 2 1
//  9 8 7 6
//
// Wire it all up according to the following table:
//
// 3DO-DB9                     Arduino Pro Micro
// ---------------------------------------------
// 1 GND                       GND
// 2 VCC                       VCC
// 3 Audio.1(1v-pp)            
// 4 Audio.1(1v-pp)            
// 5 VCC                       VCC
// 6 LATCH                     2    (PD1)
// 7 CLOCK                     3    (PD0)
// 8 GND                       GND
// 9 DATA                      A0   (PF7)

#include "Gamepad.h"

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "3DO to USB";

//#define DEBUG

#define GAMEPAD_COUNT      1  // NOTE: No more than ONE gamepad is possible at the moment due to the author only having one pad to test with.  3DO gamepads are daisychained (see linked info) so this will need more work to support.
#define GAMEPAD_COUNT_MAX  1  // NOTE: Currently set to 1 due to the above.  The loops from the (S)NES code to support more than one pad have been left in but don't do anything.
#define BUTTON_COUNT       11 // Panasonic FZ-JP1 controller has seven buttons and four axes, totalling 11
#define BUTTON_READ_DELAY  88 // Delay between button reads in µs (11 buttons x 8µs clock cycle).  Any less than this and buttons reset themselves
#define MICROS_CLOCK       4

#define BUTTONS  0
#define AXES     1
#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

// Set up USB HID gamepads
Gamepad_ Gamepad[GAMEPAD_COUNT];

// Controller
uint8_t buttons[GAMEPAD_COUNT_MAX][2] =  {{0,0}};
uint8_t buttonsPrev[GAMEPAD_COUNT_MAX][2] =  {{0,0}};
uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B10000000};
uint8_t btnByte[BUTTON_COUNT] = {1,1,1,1,0,0,0,0,0,0,0};
uint8_t btnBits[BUTTON_COUNT] = {UP,DOWN,LEFT,RIGHT,0x01,0x02,0x04,0x08,0x10,0x20,0x40};
uint8_t gp = 0;
uint8_t buttonCount = 11;  // Panasonic FZ-JP1 has seven buttons and four axes, totalling 11

// Timing
uint32_t microsButtons = 0;

#ifdef DEBUG
uint32_t microsStart = 0;
uint32_t microsEnd = 0;
uint8_t counter = 0;
#endif


void setup()
{
  //
  // Setup latch and clock pins (2,3 or PD1, PD0)
  DDRD  = B00000011; // Sets bits 0&1 as outputs
  PORTD = B00000010; // Clock (PD0) low, Latch (PD1) high
  
  // Setup data pins (A0)
  DDRF  &= ~B00000000; // inputs
  PORTF |=  B10000000; // enable internal pull-ups

  #ifdef DEBUG
  Serial.begin(115200);
  delay(2000);
  #endif

  // Short delay to let controllers stabilize
  delay(200);

}

void loop() { while(1)
{
  // See if enough time has passed since last button read
  if((micros() - microsButtons) > BUTTON_READ_DELAY)
  {    

    // Reset the button and axes state
    for(gp=0; gp<GAMEPAD_COUNT; gp++) {
      buttons[gp][BUTTONS] = 0;
      buttons[gp][AXES] = 0;
    }

    // Get the Clock and Latch lines in the right state for the controller to start sending data
    pullClock(); //pull clock line high ready it can be dropped in sync with the latch
    dropLatch(); //drop latch low, controller will start sending data

    // We don't care about the first two (three?) bits from the controller
    uint8_t clockpulse = 0;
    while(clockpulse<4) {
        sendClock();    
        clockpulse++;
    }

    // Loop through the number of configured buttons and sample the pin state for each
    for(uint8_t btn=0; btn<BUTTON_COUNT; btn++)
    {
        for(gp=0; gp<GAMEPAD_COUNT; gp++) {
          if((PINF & gpBit[gp])==0) {
            buttons[gp][btnByte[btn]] |= btnBits[btn];
            }
          sendClock(); // Send a clock pulse and loop around to get the next button until we've got them all
        }
    }

    // Finished getting button and axes data so pull the latch back up
    pullLatch();

    // Set the USB gamepad based on the button and axes data gathered from the data line
    for(gp=0; gp<GAMEPAD_COUNT; gp++)
    {
     // Has any buttons changed state?
     // Note checked added for BUTTONS = B00000000 to avoid all buttons reporting as pressed if controller disconnected
     if (buttons[gp][BUTTONS] != B00000000 && (buttons[gp][BUTTONS] != buttonsPrev[gp][BUTTONS] || buttons[gp][AXES] != buttonsPrev[gp][AXES]))
      {
        Gamepad[gp]._GamepadReport.buttons = ~buttons[gp][BUTTONS]; // 3DO controller buttons are low when pressed, so invert  
        Gamepad[gp]._GamepadReport.Y = ((buttons[gp][AXES] & DOWN) >> 1) - (buttons[gp][AXES] & UP);
        Gamepad[gp]._GamepadReport.X = ((buttons[gp][AXES] & RIGHT) >> 3) - ((buttons[gp][AXES] & LEFT) >> 2);
        buttonsPrev[gp][BUTTONS] = buttons[gp][BUTTONS];
        buttonsPrev[gp][AXES] = buttons[gp][AXES];
        Gamepad[gp].send();
      }
    }

    microsButtons = micros();
    
  }
}}

void dropLatch()
{
  PORTD &= ~B00000010; // Set LOW PD1
}

void pullLatch()
{
  PORTD |=  B00000010; // Set HIGH PD1
}

void pullClock()
{
  PORTD |=  B00000001; // Set HIGH PD0
  delayMicroseconds(MICROS_CLOCK);
}

void sendClock()
{
  // Send a clock pulse to the 3DO controller
  PORTD &= ~B00000001; // Set LOW PD0
  delayMicroseconds(MICROS_CLOCK);
  PORTD |=  B00000001; // Set HIGH PD0
  delayMicroseconds(MICROS_CLOCK); 
}
