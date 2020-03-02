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
    // Setup select pin as output high (6, PD7)
    DDRD  |= B10000000; // output
    PORTD |= B10000000; // high
    // Setup select pin as output high (5, PC6)
    DDRC  |= B01000000; // output
    PORTC |= B01000000; // high

    // Setup input pins (A0,A1,A2,A3,14,15 or PF7,PF6,PF5,PF4,PB3,PB1)
    DDRF  &= ~B11110000; // input
    PORTF |=  B11110000; // high to enable internal pull-up
    DDRB  &= ~B00001010; // input
    PORTB |=  B00001010; // high to enable internal pull-up
    // Setup input pins (TXO,RXI,2,3,4,6 or PD3,PD2,PD1,PD0,PD4,PE6)
    DDRD  &= ~B00011111; // input
    PORTD |=  B00011111; // high to enable internal pull-up
    DDRE  &= ~B01000000; // input
    PORTE |=  B01000000; // high to enable internal pull-up
    
    _inputReg1 = 0;
    _inputReg2 = 0;
    _inputReg3 = 0;
    _inputReg4 = 0;
    for(byte i=0; i<=1; i++)
    {
      _currentState[i] = 0;
      _connected[i] = 0;
      _sixButtonMode[i] = false;
      _ignoreCycles[i] = 0;
      _pinSelect[i] = true;
    }
}

word SegaControllers32U4::getStateMD1()
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
  _pinSelect[0] = !_pinSelect[0];
  (!_pinSelect[0]) ? PORTD &= ~B10000000 : PORTD |= B10000000; // Set LOW on even cycle, HIGH on uneven cycle

  // Short delay to stabilise outputs in controller
  delayMicroseconds(SC_CYCLE_DELAY);

  // Read input register(s)
  _inputReg1 = PINF;
  _inputReg2 = PINB;

  if(_ignoreCycles[0] <= 0)
  {
    if(_pinSelect[0]) // Select pin is HIGH
    {
      if(_connected[0])
      {
        // Check if six button mode is active
        if(_sixButtonMode[0])
        {
          // Read input pins for X, Y, Z, Mode
          (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) ? _currentState[0] |= SC_BTN_Z : _currentState[0] &= ~SC_BTN_Z;
          (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) ? _currentState[0] |= SC_BTN_Y : _currentState[0] &= ~SC_BTN_Y;
          (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) ? _currentState[0] |= SC_BTN_X : _currentState[0] &= ~SC_BTN_X;
          (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) ? _currentState[0] |= SC_BTN_MODE : _currentState[0] &= ~SC_BTN_MODE;
          _sixButtonMode[0] = false;
          _ignoreCycles[0] = 2; // Ignore the two next cycles (cycles 6 and 7 in table above)
        }
        else
        {
          // Read input pins for Up, Down, Left, Right, B, C
          (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) ? _currentState[0] |= SC_BTN_UP : _currentState[0] &= ~SC_BTN_UP;
          (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) ? _currentState[0] |= SC_BTN_DOWN : _currentState[0] &= ~SC_BTN_DOWN;
          (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) ? _currentState[0] |= SC_BTN_LEFT : _currentState[0] &= ~SC_BTN_LEFT;
          (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) ? _currentState[0] |= SC_BTN_RIGHT : _currentState[0] &= ~SC_BTN_RIGHT;
          (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) ? _currentState[0] |= SC_BTN_B : _currentState[0] &= ~SC_BTN_B;
          (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) ? _currentState[0] |= SC_BTN_C : _currentState[0] &= ~SC_BTN_C;
        }
      }
      else // No Mega Drive controller is connected, use SMS/Atari mode
      {
        // Clear current state
        _currentState[0] = 0;
        
        // Read input pins for Up, Down, Left, Right, Fire1, Fire2
        if (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) { _currentState[0] |= SC_BTN_UP; }
        if (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) { _currentState[0] |= SC_BTN_DOWN; }
        if (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) { _currentState[0] |= SC_BTN_LEFT; }
        if (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) { _currentState[0] |= SC_BTN_RIGHT; }
        if (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) { _currentState[0] |= SC_BTN_A; }
        if (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) { _currentState[0] |= SC_BTN_B; }
      }
    }
    else // Select pin is LOW
    {
      // Check if a controller is connected
      _connected[0] = (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW && bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW);
      
      // Check for six button mode
      _sixButtonMode[0] = (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW && bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW);
      
      // Read input pins for A and Start 
      if(_connected[0])
      {
        if(!_sixButtonMode[0])
        {
          (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) ? _currentState[0] |= SC_BTN_A : _currentState[0] &= ~SC_BTN_A;
          (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) ? _currentState[0] |= SC_BTN_START : _currentState[0] &= ~SC_BTN_START; 
        }
      }
    }
  }
  else
  {
    _ignoreCycles[0]--;
  }

  return _currentState[0];
}

word SegaControllers32U4::getStateMD2()
{
    // Set the select pin low/high
  _pinSelect[1] = !_pinSelect[1];
  (!_pinSelect[1]) ? PORTC &= ~B01000000 : PORTC |= B01000000; // Set LOW on even cycle, HIGH on uneven cycle

  // Short delay to stabilise outputs in controller
  delayMicroseconds(SC_CYCLE_DELAY);

  // Read input register(s)
  _inputReg3 = PIND;
  _inputReg4 = PINE;

  if(_ignoreCycles[1] <= 0)
  {
    if(_pinSelect[1]) // Select pin is HIGH
    {
      if(_connected[1])
      {
        // Check if six button mode is active
        if(_sixButtonMode[1])
        {
          // Read input pins for X, Y, Z, Mode
          (bitRead(_inputReg3, DB9_PIN1_BIT2) == LOW) ? _currentState[1] |= SC_BTN_Z : _currentState[1] &= ~SC_BTN_Z;
          (bitRead(_inputReg3, DB9_PIN2_BIT2) == LOW) ? _currentState[1] |= SC_BTN_Y : _currentState[1] &= ~SC_BTN_Y;
          (bitRead(_inputReg3, DB9_PIN3_BIT2) == LOW) ? _currentState[1] |= SC_BTN_X : _currentState[1] &= ~SC_BTN_X;
          (bitRead(_inputReg3, DB9_PIN4_BIT2) == LOW) ? _currentState[1] |= SC_BTN_MODE : _currentState[1] &= ~SC_BTN_MODE;
          _sixButtonMode[1] = false;
          _ignoreCycles[1] = 2; // Ignore the two next cycles (cycles 6 and 7 in table above)
        }
        else
        {
          // Read input pins for Up, Down, Left, Right, B, C
          (bitRead(_inputReg3, DB9_PIN1_BIT2) == LOW) ? _currentState[1] |= SC_BTN_UP : _currentState[1] &= ~SC_BTN_UP;
          (bitRead(_inputReg3, DB9_PIN2_BIT2) == LOW) ? _currentState[1] |= SC_BTN_DOWN : _currentState[1] &= ~SC_BTN_DOWN;
          (bitRead(_inputReg3, DB9_PIN3_BIT2) == LOW) ? _currentState[1] |= SC_BTN_LEFT : _currentState[1] &= ~SC_BTN_LEFT;
          (bitRead(_inputReg3, DB9_PIN4_BIT2) == LOW) ? _currentState[1] |= SC_BTN_RIGHT : _currentState[1] &= ~SC_BTN_RIGHT;
          (bitRead(_inputReg3, DB9_PIN6_BIT2) == LOW) ? _currentState[1] |= SC_BTN_B : _currentState[1] &= ~SC_BTN_B;
          (bitRead(_inputReg4, DB9_PIN9_BIT2) == LOW) ? _currentState[1] |= SC_BTN_C : _currentState[1] &= ~SC_BTN_C;
        }
      }
      else // No Mega Drive controller is connected, use SMS/Atari mode
      {
        // Clear current state
        _currentState[1] = 0;
        
        // Read input pins for Up, Down, Left, Right, Fire1, Fire2
        if (bitRead(_inputReg3, DB9_PIN1_BIT2) == LOW) { _currentState[1] |= SC_BTN_UP; }
        if (bitRead(_inputReg3, DB9_PIN2_BIT2) == LOW) { _currentState[1] |= SC_BTN_DOWN; }
        if (bitRead(_inputReg3, DB9_PIN3_BIT2) == LOW) { _currentState[1] |= SC_BTN_LEFT; }
        if (bitRead(_inputReg3, DB9_PIN4_BIT2) == LOW) { _currentState[1] |= SC_BTN_RIGHT; }
        if (bitRead(_inputReg3, DB9_PIN6_BIT2) == LOW) { _currentState[1] |= SC_BTN_A; }
        if (bitRead(_inputReg4, DB9_PIN9_BIT2) == LOW) { _currentState[1] |= SC_BTN_B; }
      }
    }
    else // Select pin is LOW
    {
      // Check if a controller is connected
      _connected[1] = (bitRead(_inputReg3, DB9_PIN3_BIT2) == LOW && bitRead(_inputReg3, DB9_PIN4_BIT2) == LOW);
      
      // Check for six button mode
      _sixButtonMode[1] = (bitRead(_inputReg3, DB9_PIN1_BIT2) == LOW && bitRead(_inputReg3, DB9_PIN2_BIT2) == LOW);
      
      // Read input pins for A and Start 
      if(_connected[1])
      {
        if(!_sixButtonMode[1])
        {
          (bitRead(_inputReg3, DB9_PIN6_BIT2) == LOW) ? _currentState[1] |= SC_BTN_A : _currentState[1] &= ~SC_BTN_A;
          (bitRead(_inputReg4, DB9_PIN9_BIT2) == LOW) ? _currentState[1] |= SC_BTN_START : _currentState[1] &= ~SC_BTN_START; 
        }
      }
    }
  }
  else
  {
    _ignoreCycles[1]--;
  }

  return _currentState[1];
}
