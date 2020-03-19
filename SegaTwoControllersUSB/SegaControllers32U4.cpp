//
// SegaControllers32U4.cpp
//
// Authors:
//       Jon Thysell <thysell@gmail.com>
//       Mikael Norrgård <mick@daemonbite.com>
//
// (Based on the code by Jon Thysell, but the interfacing is almost completely
//  rewritten by Mikael Norrgård)
//
// Copyright (c) 2017 Jon Thysell <http://jonthysell.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "Arduino.h"
#include "SegaControllers32U4.h"

SegaControllers32U4::SegaControllers32U4(void)
{
  // Setup input pins (A0,A1,A2,A3,14,15 or PF7,PF6,PF5,PF4,PB3,PB1)
  DDRF  &= ~B11110000; // input
  PORTF |=  B11110000; // high to enable internal pull-up
  DDRB  &= ~B00001010; // input
  PORTB |=  B00001010; // high to enable internal pull-up
  // Setup input pins (TXO,RXI,2,3,4,6 or PD3,PD2,PD1,PD0,PD4,PE6)
  DDRD  &= ~B10011111; // input
  PORTD |=  B10011111; // high to enable internal pull-up
  
  for(byte i=0; i<=1; i++)
  {
    *_ddrSelect[i] |= _maskSelect[i];  // Select pins as output
    *_portSelect[i] |= _maskSelect[i]; // Select pins high
    _inputReg[i] = 0;
    _currentState[i] = 0;
    _connected[i] = 0;
    _sixButtonMode[i] = false;
    _ignoreCycles[i] = 0;
    _pinSelect[i] = true;
  }
}

word SegaControllers32U4::getStateMD(byte gp)
{
  // "Normal" Six button controller reading routine, done a bit differently in this project
  // Cycle  TH out  TR in  TL in  D3 in  D2 in  D1 in  D0 in
  // 0      LO      Start  A      0      0      Down   Up      
  // 1      HI      C      B      Right  Left   Down   Up
  // 2      LO      Start  A      0      0      Down   Up      (Check connected and read Start and A in this cycle)
  // 3      HI      C      B      Right  Left   Down   Up      (Read B, C and directions in this cycle)
  // 4      LO      Start  A      0      0      0      0       (Check for six button controller in this cycle)
  // 5      HI      C      B      Mode   X      Y      Z       (Read X,Y,Z and Mode in this cycle)    
  // 6      LO      ---    ---    ---    ---    ---    ---      
  // 7      HI      ---    ---    ---    ---    ---    ---    

  // Set the select pin low/high
  _pinSelect[gp] = !_pinSelect[gp];
  (!_pinSelect[gp]) ? *_portSelect[gp] &= ~_maskSelect[gp] : *_portSelect[gp] |= _maskSelect[gp]; // Set LOW on even cycle, HIGH on uneven cycle

  // Short delay to stabilise outputs in controller
  delayMicroseconds(SC_CYCLE_DELAY);

  // Read input register(s)
  _inputReg[0] = *_pinInputs[gp][0];
  _inputReg[1] = *_pinInputs[gp][1];

  if(_ignoreCycles[gp] <= 0)
  {
    if(_pinSelect[gp]) // Select pin is HIGH
    {
      if(_connected[gp])
      {
        // Check if six button mode is active
        if(_sixButtonMode[gp])
        {
          // Read input pins for X, Y, Z, Mode
          (bitRead(*_pinInputs[gp][SC_PIN1_BIT], _bitInputs[gp][SC_PIN1_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_Z : _currentState[gp] &= ~SC_BTN_Z;
          (bitRead(*_pinInputs[gp][SC_PIN2_BIT], _bitInputs[gp][SC_PIN2_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_Y : _currentState[gp] &= ~SC_BTN_Y;
          (bitRead(*_pinInputs[gp][SC_PIN3_BIT], _bitInputs[gp][SC_PIN3_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_X : _currentState[gp] &= ~SC_BTN_X;
          (bitRead(*_pinInputs[gp][SC_PIN4_BIT], _bitInputs[gp][SC_PIN4_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_MODE : _currentState[gp] &= ~SC_BTN_MODE;
          _sixButtonMode[gp] = false;
          _ignoreCycles[gp] = 2; // Ignore the two next cycles (cycles 6 and 7 in table above)
        }
        else
        {
          // Read input pins for Up, Down, Left, Right, B, C
          (bitRead(*_pinInputs[gp][SC_PIN1_BIT], _bitInputs[gp][SC_PIN1_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_UP : _currentState[gp] &= ~SC_BTN_UP;
          (bitRead(*_pinInputs[gp][SC_PIN2_BIT], _bitInputs[gp][SC_PIN2_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_DOWN : _currentState[gp] &= ~SC_BTN_DOWN;
          (bitRead(*_pinInputs[gp][SC_PIN3_BIT], _bitInputs[gp][SC_PIN3_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_LEFT : _currentState[gp] &= ~SC_BTN_LEFT;
          (bitRead(*_pinInputs[gp][SC_PIN4_BIT], _bitInputs[gp][SC_PIN4_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_RIGHT : _currentState[gp] &= ~SC_BTN_RIGHT;
          (bitRead(*_pinInputs[gp][SC_PIN6_BIT], _bitInputs[gp][SC_PIN6_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_B : _currentState[gp] &= ~SC_BTN_B;
          (bitRead(*_pinInputs[gp][SC_PIN9_BIT], _bitInputs[gp][SC_PIN9_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_C : _currentState[gp] &= ~SC_BTN_C;
        }
      }
      else // No Mega Drive controller is connected, use SMS/Atari mode
      {
        // Clear current state
        _currentState[gp] = 0;
        
        // Read input pins for Up, Down, Left, Right, Fire1, Fire2
        if (bitRead(*_pinInputs[gp][SC_PIN1_BIT], _bitInputs[gp][SC_PIN1_BIT]) == LOW) { _currentState[gp] |= SC_BTN_UP; }
        if (bitRead(*_pinInputs[gp][SC_PIN2_BIT], _bitInputs[gp][SC_PIN2_BIT]) == LOW) { _currentState[gp] |= SC_BTN_DOWN; }
        if (bitRead(*_pinInputs[gp][SC_PIN3_BIT], _bitInputs[gp][SC_PIN3_BIT]) == LOW) { _currentState[gp] |= SC_BTN_LEFT; }
        if (bitRead(*_pinInputs[gp][SC_PIN4_BIT], _bitInputs[gp][SC_PIN4_BIT]) == LOW) { _currentState[gp] |= SC_BTN_RIGHT; }
        if (bitRead(*_pinInputs[gp][SC_PIN6_BIT], _bitInputs[gp][SC_PIN6_BIT]) == LOW) { _currentState[gp] |= SC_BTN_A; }
        if (bitRead(*_pinInputs[gp][SC_PIN9_BIT], _bitInputs[gp][SC_PIN9_BIT]) == LOW) { _currentState[gp] |= SC_BTN_B; }
      }
    }
    else // Select pin is LOW
    {
      // Check if a controller is connected
      _connected[gp] = (bitRead(*_pinInputs[gp][SC_PIN3_BIT], _bitInputs[gp][SC_PIN3_BIT]) == LOW && bitRead(*_pinInputs[gp][SC_PIN4_BIT], _bitInputs[gp][SC_PIN4_BIT]) == LOW);
      
      // Check for six button mode
      _sixButtonMode[gp] = (bitRead(*_pinInputs[gp][SC_PIN1_BIT], _bitInputs[gp][SC_PIN1_BIT]) == LOW && bitRead(*_pinInputs[gp][SC_PIN2_BIT], _bitInputs[gp][SC_PIN2_BIT]) == LOW);
      
      // Read input pins for A and Start 
      if(_connected[gp])
      {
        if(!_sixButtonMode[gp])
        {
          (bitRead(*_pinInputs[gp][SC_PIN6_BIT], _bitInputs[gp][SC_PIN6_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_A : _currentState[gp] &= ~SC_BTN_A;
          (bitRead(*_pinInputs[gp][SC_PIN9_BIT], _bitInputs[gp][SC_PIN9_BIT]) == LOW) ? _currentState[gp] |= SC_BTN_START : _currentState[gp] &= ~SC_BTN_START; 
        }
      }
    }
  }
  else
  {
    _ignoreCycles[gp]--;
  }

  return _currentState[gp];
}
