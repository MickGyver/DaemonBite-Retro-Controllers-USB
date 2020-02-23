/*  
 *  A2600 Paddles/Spinners USB Adapter
 *  (C) Alexey Melnikov 
 *   
 *  Based on project by Mikael Norrgård <mick@daemonbite.com>
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

// PADDLE:
// Paddles need a modification: connect unused 3rd pin on potentiometer (make sure it's not connected to middle pin!)
// to the ground (black wire on button).

// SPINNER:
// Any spinner should work. Original A2600 driving controller has very low resolution, so it's better to upgrade it with some 3rd party spinner
// component with at least 80 ppr. Spinner components with clicking mechanism is not recommended as it won't be smooth.

// Controller DB9 pins (looking face-on to the end of the plug):
//
// 5 4 3 2 1
//  9 8 7 6
//
// Joystick Port 1
// DB9    Arduino Pro Micro     Paddle x2   Driving x1
// ---------------------------------------------------
//  1     TXO(1)  PD3                       EncA
//  2     RXI(0)  PD2                       EncB
//  3     3       PD0           button 1
//  4     4       PD4           button 0
//  5     A0      PF7           paddle 0
//  6     6       PD7                       button
//  7     VCC                   VCC         VCC
//  8     GND                   GND         GND
//  9     A1      PF6           paddle 1
//
// Joystick Port 2
// DB9    Arduino Pro Micro                 Driving x1
// ---------------------------------------------------
//  1     2    PD1                          EncA
//  2     7    PE6                          EncB
//  3
//  4
//  5
//  6     15   PB1                          button
//  7     VCC                               VCC
//  8     GND                               GND
//  9
//
//
// Note: spinners pins must support interrupts!
//

///////////////// Customizable settings /////////////////////////

//uncomment following line for Beetle board pin mapping.
//#define BEETLE

// Spinner pulses per revolution
// For arduino shield spinner: 20
#define SPINNER_PPR 20

// Comment it to dosable paddle emulation by spinner
#define PADDLE_EMU

// Optional parameter. Leave it commented out.
//#define SPINNER_SENSITIVITY 1

/////////////////////////////////////////////////////////////////

// pins map
#ifdef BEETLE
  const int8_t encpin[2][2] = {{1,0},{3,2}}; // rotary encoder
  const int8_t dbtnpin[2]   = {9,15};        // driving controller button
  const int8_t pbtnpin[2]   = {11,10};       // paddle button
  const int8_t pdlpin[2]    = {A0,A1};       // paddle pot
#else
  const int8_t encpin[2][2] = {{1,0},{2,7}}; // rotary encoder
  const int8_t dbtnpin[2]   = {6,15};        // driving controller button
  const int8_t pbtnpin[2]   = {4,3};         // paddle button
  const int8_t pdlpin[2]    = {A0,A1};       // paddle pot
#endif

////////////////////////////////////////////////////////

#ifndef SPINNER_SENSITIVITY
  #if SPINNER_PPR < 50
    #define SPINNER_SENSITIVITY 1
  #else
    #define SPINNER_SENSITIVITY 2
  #endif
#endif

// ID for special support in MiSTer 
const char *gp_serial = "MiSTer PD/SP v1";

#include <ResponsiveAnalogRead.h> 
#include "Gamepad.h"

Gamepad_ Gamepad[2];
ResponsiveAnalogRead analog[2] = {ResponsiveAnalogRead(pdlpin[0], true),ResponsiveAnalogRead(pdlpin[1], true)};

int8_t pdlena[2]  = {0,0};
uint16_t drvpos[2];
int16_t drvX[2] = {0,0};

void setup()
{
  float snap = .01;
  float thresh = 8.0;

  for(int idx=0; idx<2; idx++)
  {
    Gamepad[idx].reset();

    pinMode(encpin[idx][0], INPUT_PULLUP);
    pinMode(encpin[idx][1], INPUT_PULLUP);
    pinMode(dbtnpin[idx],   INPUT_PULLUP);
    drv_proc(idx);
    drvpos[idx] = 0;
    attachInterrupt(digitalPinToInterrupt(encpin[idx][0]), idx ? drv1_isr : drv0_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encpin[idx][1]), idx ? drv1_isr : drv0_isr, CHANGE);
    
    pdlena[idx] = 0;
    pinMode(pbtnpin[idx],   INPUT_PULLUP);
    pinMode(pdlpin[idx],    INPUT);
    analog[idx].setSnapMultiplier(snap);
    analog[idx].setActivityThreshold(thresh);
  }
}

void loop()
{
  sendState(0);
  sendState(1);
}

const uint16_t sp_max = ((SPINNER_PPR*4*270UL)/360);
int32_t sp_clamp[2] = {0,0};

void drv_proc(int8_t idx)
{
  static int8_t prev[2];
  int8_t a = digitalRead(encpin[idx][0]);
  int8_t b = digitalRead(encpin[idx][1]);

  int8_t spval = (b << 1) | (b^a);
  int8_t diff = (prev[idx] - spval)&3;

  if(diff == 3) 
  {
    drvpos[idx] += 10;
    if(sp_clamp[idx] < sp_max-1) sp_clamp[idx]++;
  }
  if(diff == 1) 
  {
    drvpos[idx] -= 10;
    if(sp_clamp[idx] > 0) sp_clamp[idx]--;
  }

  prev[idx] = spval;
}

void drv0_isr()
{
  drv_proc(0);
}

void drv1_isr()
{
  drv_proc(1);
}

const int16_t sp_step = (SPINNER_PPR*10)/(20*SPINNER_SENSITIVITY);

void sendState(byte idx)
{
  // LEDs off
  TXLED1; //RXLED1;

  analog[idx].update();

  // paddle
  int8_t newA = !digitalRead(pbtnpin[idx]);
  //int8_t newB = 0; // reserved for paddles mixed in a single USB controller.
  int8_t newX = 0;
  int8_t newY = 0;

  // spinner
  int8_t newC = !digitalRead(dbtnpin[idx]);
  int8_t newR = 0;
  int8_t newL = 0;

  if(newA) pdlena[idx] = 1;
  if(newC) pdlena[idx] = 0;

  if(pdlena[idx])
  {
    newX = (analog[idx].getValue()>>2) ^ 0x80;
  }
  else
  {
    if(!Gamepad[idx]._GamepadReport.b3 && !Gamepad[idx]._GamepadReport.b4)
    {
      static uint16_t prev[2] = {0,0};
      int16_t diff = drvpos[idx] - prev[idx];

      if(diff >= sp_step)
      {
        newR = 1;
        prev[idx] += sp_step;
        //Serial.println("RIGHT");
      }
      else if(diff <= -sp_step)
      {
        newL = 1;
        prev[idx] -= sp_step;
        //Serial.println("LEFT");
      }
    }

#ifdef PADDLE_EMU
    uint16_t val = (sp_clamp[idx]*255)/sp_max;
    newX = val ^ 0x80;
#endif

  }

  int8_t diff = newX - Gamepad[idx]._GamepadReport.X;

  // Only report controller state if it has changed
  if (diff
    || ((Gamepad[idx]._GamepadReport.b0 ^ newA) & 1)
    || ((Gamepad[idx]._GamepadReport.b2 ^ newC) & 1)
    || ((Gamepad[idx]._GamepadReport.b3 ^ newL) & 1)
    || ((Gamepad[idx]._GamepadReport.b4 ^ newR) & 1))
  {
    //if(!idx) Serial.println(newX);
    Gamepad[idx]._GamepadReport.X  = newX;
    Gamepad[idx]._GamepadReport.b0 = newA;
    Gamepad[idx]._GamepadReport.b2 = newC;
    Gamepad[idx]._GamepadReport.b3 = newL;
    Gamepad[idx]._GamepadReport.b4 = newR;
    Gamepad[idx].send();
  }
}
