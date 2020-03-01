/*  
 *  PSX JogCon based Arcade USB controller
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

//               LOOKING AT THE PLUG
//          |-----------------------------|
//  PIN 1-> | o  o  o | o  o  o | o  o  o |
//          \_________|_________|_________/
//     
//
//   Arduino   Plug    PCB connector
// -----------------------------------
//   RXI(D0)     1      2 DATA
//   GND         4      3 GND
//   3.3V        5      4 3.3V (main)
//   D4          6      5 ATT
//   TXO(D1)     2      6 COMMAND
//   D3          7      7 CLOCK
//   5V          3      8 5V (motor)
//

////////////////////////////////////////////////////////

#define CMD 1
#define DAT 0
#define CLK 3
#define ATT 4

#define DELAY   4
#define SP_MAX  160

////////////////////////////////////////////////////////

// ID for special support in MiSTer 
// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char *gp_serial = "MiSTer-A1 JogCon";

#include <EEPROM.h>
#include "Gamepad.h"

Gamepad_ Gamepad;

byte ff = 0;
byte mode = 0;
byte force = 15;
int16_t sp_step = 4;
byte data[8];

const byte cmd_read[]       = {0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const byte cmd_cfg_enter[]  = {0x43, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
const byte cmd_cfg_exit[]   = {0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const byte cmd_set_mode_a[] = {0x44, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00};
const byte cmd_set_mode_d[] = {0x44, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
const byte cmd_get_mode[]   = {0x45, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
const byte cmd_unlock_ff[]  = {0x4D, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xFF};

byte psx_io(byte out)
{
  byte in = 0;

  for (byte i = 0; i < 8; i++)
  {
    digitalWrite(CLK, LOW);
    digitalWrite(CMD, (out & (1 << i)) ? HIGH : LOW);
    
    delayMicroseconds(DELAY);
    in = in | (digitalRead(DAT) << i);
    digitalWrite(CLK, HIGH);

    delayMicroseconds(DELAY);
  }

  delayMicroseconds(DELAY*2);
  return in;
}

void send_cmd(const byte *cmd)
{
  digitalWrite(ATT, LOW);

  psx_io(0x01);
  for(int i=0; i<7; i++) data[i] = psx_io((i==2 && cmd[0] == 0x42 && ff) ? (force | 0x30) : cmd[i]);
  data[7] = psx_io(0xFF);

  data[2] = ~data[2];
  data[3] = ~data[3];

  digitalWrite(ATT, HIGH);
  delayMicroseconds(DELAY*10);
}

void init_jogcon()
{
  send_cmd(cmd_cfg_enter);
  send_cmd(cmd_set_mode_a);
  send_cmd(cmd_unlock_ff);
  send_cmd(cmd_read);
  send_cmd(cmd_read);
}

#define UP    0x1
#define RIGHT 0x2
#define DOWN  0x4
#define LEFT  0x8

uint8_t dpad2hat(uint8_t dpad)
{
  switch(dpad & (UP|DOWN|LEFT|RIGHT))
  {
    case UP:         return 0;
    case UP|RIGHT:   return 1;
    case RIGHT:      return 2;
    case DOWN|RIGHT: return 3;
    case DOWN:       return 4;
    case DOWN|LEFT:  return 5;
    case LEFT:       return 6;
    case UP|LEFT:    return 7;
  }
  return 15;
}

uint8_t sp_div;
int16_t sp_max;
int16_t sp_half;

void setup()
{
  Gamepad.reset();

  pinMode(CLK, OUTPUT); digitalWrite(CLK, HIGH);
  pinMode(CMD, OUTPUT); digitalWrite(CMD, HIGH);
  pinMode(ATT, OUTPUT); digitalWrite(ATT, HIGH);
  pinMode(DAT, INPUT_PULLUP);

  mode = EEPROM.read(0) & 0x3;
  if(mode == 3) mode = 0;

  force = EEPROM.read(1) & 0xF;
  if(!force) force = 15;
  
  sp_step = EEPROM.read(2);
  if(sp_step > 8) sp_step = 8;
  if(sp_step < 1) sp_step = 1;

  sp_div = EEPROM.read(3) ? 1 : 2;
  sp_max = SP_MAX/sp_div;
  sp_half = sp_max/2;

  init_jogcon();
}

void loop()
{
  static uint16_t counter = 0, newcnt = 0, cleancnt = 0;
  static uint16_t newbtn = 0, oldbtn = 0;
  static int32_t pdlpos = sp_half;
  static uint16_t prevcnt = 0;

  send_cmd(cmd_read);
  if(data[0] == 0x41 || !data[1])
  {
    if(data[0] == 0xF3 && !data[1])
    {
      // Mode switch by pressing "mode" button while holding:
      // L2 - paddle mode (with FF stoppers)
      // R2 - steering mode (FF always enabled)
      // L2+R2 - spinner mode (no FF)
      if(data[3]&3)
      {
        mode = data[3] & 3;
        if(mode == 3) mode = 0;
        EEPROM.write(0, mode);
      }

      // Force Feedback adjust
      // by pressing "mode" button while holding /\,O,X,[]
      if(data[3] & 0xF0)
      {
        if(data[3] & 0x10) force = 1;
        if(data[3] & 0x20) force = 3;
        if(data[3] & 0x40) force = 7;
        if(data[3] & 0x80) force = 15;
        EEPROM.write(1, force);
      }

      // Spinner pulses per step adjust
      // by pressing "mode" button while holding up,right,down,left
      if(data[2] & 0xF0)
      {
        if(data[2] & 0x10) sp_step = 1;
        if(data[2] & 0x20) sp_step = 2;
        if(data[2] & 0x40) sp_step = 4;
        if(data[2] & 0x80) sp_step = 8;
        EEPROM.write(2, sp_step);
      }
    }

    // Paddle range switch by pressing "mode" button while holding:
    // L1 - 270 degree
    // R1 - 135 degree
    if(data[3]&0xC)
    {
      sp_div = (data[3] & 4) ? 2 : 1;
      sp_max = SP_MAX/sp_div;
      sp_half = sp_max/2;
      EEPROM.write(3, !(sp_div>>1));
    }

    // some time for visual confirmation
    delay(200);

    // reset zero position
    init_jogcon();

    prevcnt = 0;
    cleancnt = 0;
    counter = (data[5] << 8) | data[4];
    pdlpos = sp_half;
  }

  newcnt = (data[5] << 8) | data[4];
  newbtn = (data[3] << 8) | data[2];
  newbtn = (newbtn & ~3) | ((newbtn&1)<<2);

  if(data[0] == 0xF3)
  {
    if(data[6]&3) 
    {
      cleancnt += newcnt - counter;
      if(!mode)
      {
        ff = 0;
        pdlpos += (int16_t)(newcnt - counter);
        if(pdlpos<0) pdlpos = 0;
        if(pdlpos>sp_max) pdlpos = sp_max;
      }
    }

    if(mode)
    {
      if(((int16_t)newcnt) < -sp_half)
      {
        pdlpos = 0;
        if(mode == 1) ff = 1;
      }
      else if(((int16_t)newcnt) > sp_half)
      {
        pdlpos = sp_max;
        if(mode == 1) ff = 1;
      }
      else
      {
        if(mode == 1) ff = 0;
        pdlpos = (uint16_t)(newcnt + sp_half);
      }
    }

    if(mode == 2) ff = 1;

    int16_t val = ((int16_t)(cleancnt - prevcnt))/sp_step;
    if(val>127) val = 127; else if(val<-127) val = -127;
    prevcnt += val*sp_step;
    int8_t spinner = val;
    
    uint8_t paddle = ((pdlpos*255)/sp_max);

    if(oldbtn != newbtn || Gamepad._GamepadReport.paddle != paddle || Gamepad._GamepadReport.spinner != spinner)
    {
      oldbtn = newbtn;
      Gamepad._GamepadReport.buttons = (newbtn & 0xF) | ((newbtn>>4) & ~0xF);
      Gamepad._GamepadReport.paddle = paddle;
      Gamepad._GamepadReport.spinner = spinner;
      Gamepad._GamepadReport.hat = dpad2hat(newbtn>>4);
      Gamepad.send();
    }
  }

  counter = newcnt;
}
