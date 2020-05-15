# DaemonBite Saturn Controller To USB Adapter
## Introduction
This is a simple to build adapter for connecting SEGA Saturn controllers to USB. Currently it supports normal digital Saturn controllers, 3D controllers are not supported.

The input lag for this adapter is minimal (about 0.75ms average connected to MiSTer).

## Parts you need
- Arduino Pro Micro (ATMega32U4)
- Male end of a Saturn controller extension cable
- Heat shrink tube (Ø ~20mm)
- Micro USB cable

## Wiring
Saturn controller socket (looking face-on at the front of the socket):

/ 1 2 3 4 5 6 7 8 9 \

Saturn controller plug (looking face-on at the front of the controller plug):

/ 9 8 7 6 5 4 3 2 1 \

| Saturn (P1) | Arduino Pro Micro |
| ------ | ------ |
| 1 VCC | VCC |
| 2 DATA1 | 2   PD1 |
| 3 DATA0 | 3   PD0 |
| 4 SEL1 | 15  PB1 (Shared with P2) |
| 5 SEL0 | 14  PB3 (Shared with P2) |
| 6 TL (5V) | 4   PD4 |
| 7 DATA3 | TXO PD3 |
| 8 DATA2 | RXI PD2 |
| 9 GND | GND |

| Saturn (P2) | Arduino Pro Micro
| ------ | ------ |
| 1 VCC | VCC |
| 2 DATA1 | A2  PF5 |
| 3 DATA0 | A3  PF4 |
| 4 SEL1 | 15  PB1 (Shared with P1) |
| 5 SEL0 | 14  PB3 (Shared with P1) |
| 6 TL (5V) | 6   PD7 |
| 7 DATA3 | A0  PF7 |
| 8 DATA2 | A1  PF6 |
| 9 GND | GND |

## How to assemble (please ignore the switch)
Check the SegaControllerUSB readme for an idea how to assemble.

## Retro Bit 2.4GHz wireless controller
The Retro Bit 2.4GHz wireless controller is officially not supported but you can enable support for it in the code (with a huge increase in lag as a side effect).The receiver of the Retro Bit 2.4GHz controller needs to be plugged in after the adapter has been connected to USB and the RETROBIT define needs to be uncommented.

## License
This project is licensed under the GNU General Public License v3.0.
