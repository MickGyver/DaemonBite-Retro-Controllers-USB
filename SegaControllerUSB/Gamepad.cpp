/*  Gamepad.cpp
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

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
    0xa1, 0x01,                       // COLLECTION (Application)
    0x85, 0x01,                       // REPORT_ID (1) // change to 3 if using mouse and keyboard on 1&2
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x08,                       // USAGE_MAXIMUM (Button 8)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x08,                       // REPORT_COUNT (8)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)
    
      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};


Gamepad_::Gamepad_(void) 
{ 
  reportId=1;
  static HIDSubDescriptor node(_hidReportDescriptor, sizeof(_hidReportDescriptor));
  HID().AppendDescriptor(&node);
}

void Gamepad_::begin(uint8_t id) 
{
  reportId=id;
}

void Gamepad_::end(void) 
{
  this->reset();
}

void Gamepad_::reset()
{
  _GamepadReport.X = 0;
  _GamepadReport.Y = 0;
  _GamepadReport.buttons = 0;
  this->send();
}

void Gamepad_::send() 
{
  HID().SendReport(reportId,&_GamepadReport,sizeof(GamepadReport));
}
