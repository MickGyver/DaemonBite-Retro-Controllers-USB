//
// SegaControllers32U4.h
//
// Authors:
//       Jon Thysell <thysell@gmail.com>
//       Mikael Norrgård <mick@daemonbite.com>
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

#ifndef SegaController32U4_h
#define SegaController32U4_h

enum
{
  SC_CTL_ON    = 1,   // The controller is connected (not used)
  SC_BTN_UP    = 2,
  SC_BTN_DOWN  = 4,
  SC_BTN_LEFT  = 8,
  SC_BTN_RIGHT = 16,
  SC_BTN_A     = 32,
  SC_BTN_B     = 64,
  SC_BTN_C     = 128,
  SC_BTN_X     = 256,
  SC_BTN_Y     = 512,
  SC_BTN_Z     = 1024,
  SC_BTN_START = 2048,
  SC_BTN_MODE  = 4096,
  SC_BTN_1     = 64,   // Master System compatibility
  SC_BTN_2     = 128,  // Master System compatibility
  DB9_PIN1_BIT1 = 7,
  DB9_PIN2_BIT1 = 6,
  DB9_PIN3_BIT1 = 5,
  DB9_PIN4_BIT1 = 4,
  DB9_PIN6_BIT1 = 3,
  DB9_PIN9_BIT1 = 1,
  DB9_PIN1_BIT2 = 3,
  DB9_PIN2_BIT2 = 2,
  DB9_PIN3_BIT2 = 1,
  DB9_PIN4_BIT2 = 0,
  DB9_PIN6_BIT2 = 4,
  DB9_PIN9_BIT2 = 6
};

const byte SC_INPUT_PINS = 6;

const byte SC_CYCLE_DELAY = 10; // Delay (µs) between setting the select pin and reading the button pins

class SegaControllers32U4 {
  public:
    SegaControllers32U4(void);

    word getStateMD1();
    word getStateMD2();

  private:
    word _currentState[2];

    boolean _pinSelect[2];

    byte _ignoreCycles[2];

    boolean _connected[2];
    boolean _sixButtonMode[2];

    byte _inputReg1;
    byte _inputReg2;
    byte _inputReg3;
    byte _inputReg4;
};

#endif
