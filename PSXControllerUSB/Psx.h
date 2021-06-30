/*  PSX Controller Decoder Library (Psx.h)
	Written by: Kevin Ahrendt June 22nd, 2008
	
	Controller protocol implemented using Andrew J McCubbin's analysis.
	http://www.gamesx.com/controldata/psxcont/psxcont.htm
	
	Shift command is based on tutorial examples for ShiftIn and ShiftOut
	functions both written by Carlyn Maw and Tom Igoe
	http://www.arduino.cc/en/Tutorial/ShiftIn
	http://www.arduino.cc/en/Tutorial/ShiftOut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef Psx_h
#define Psx_h

#include "Arduino.h"

// Button Hex Representations:
#define psxLeft		0x0001 
#define psxDown		0x0002
#define psxRight	0x0004
#define psxUp		0x0008
#define psxStrt		0x0010
#define psxSlct		0x0080

#define psxSqu		0x0100
#define psxX		0x0200
#define psxO		0x0400
#define psxTri		0x0800
#define psxR1		0x1000
#define psxL1		0x2000
#define psxR2		0x4000
#define psxL2		0x8000


class Psx
{
	public:
		Psx();
		void setupPins(byte , byte , byte , byte , byte );		// (Data Pin #, CMND Pin #, ATT Pin #, CLK Pin #, Delay)
															// Delay is how long the clock goes without changing state
															// in Microseconds. It can be lowered to increase response,
															// but if it is too low it may cause glitches and have some
															// keys spill over with false-positives. A regular PSX controller
															// works fine at 50 uSeconds.
															
		unsigned int read();								// Returns the status of the button presses in an unsignd int.
															// The value returned corresponds to each key as defined above.
		
	private:
		byte shift(byte _dataOut);

		byte _dataPin;
		byte _cmndPin;
		byte _attPin;
		byte _clockPin;
		
		byte _delay;
		byte _i;
		boolean _temp;
		byte _dataIn;
		
		byte _data1;
		byte _data2;
		unsigned int _dataOut;
};

#endif
