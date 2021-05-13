//
// SegaController32U4.h
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

//#define USE_OLD_WIRING // Uncomment this line to use the old wiring with DB9 pin 6 to Arduino pin 7

#ifdef USE_OLD_WIRING
  #define DDR_SELECT   DDRD
  #define PORT_SELECT  PORTD
  #define MASK_SELECT  B10000000
#else
  #define DDR_SELECT   DDRE
  #define PORT_SELECT  PORTE
  #define MASK_SELECT  B01000000
#endif

enum
{
  SC_BTN_UP    = 1,
  SC_BTN_DOWN  = 2,
  SC_BTN_LEFT  = 4,
  SC_BTN_RIGHT = 8,
  SC_BTN_A     = 16,
  SC_BTN_B     = 32,
  SC_BTN_C     = 64,
  SC_BTN_X     = 128,
  SC_BTN_Y     = 256,
  SC_BTN_Z     = 512,
  SC_BTN_START = 1024,
  SC_BTN_MODE  = 2048,
  SC_BTN_HOME  = 4096,
  SC_BIT_SH_UP    = 0,
  SC_BIT_SH_DOWN  = 1,
  SC_BIT_SH_LEFT  = 2,
  SC_BIT_SH_RIGHT = 3,
  DB9_PIN1_BIT = 7,
  DB9_PIN2_BIT = 6,
  DB9_PIN3_BIT = 5,
  DB9_PIN4_BIT = 4,
  DB9_PIN6_BIT = 3,
  DB9_PIN9_BIT = 1
};

const byte SC_CYCLE_DELAY = 12; // Delay (µs) between setting the select pin and reading the button pins

class SegaController32U4 {
  public:
    SegaController32U4(void);

    word getStateMD();

  private:
    word _currentState;

    boolean _pinSelect;

    byte _ignoreCycles;

    boolean _connected;
    boolean _sixButtonMode;

    byte _inputReg1;
    byte _inputReg2;
};

#endif
