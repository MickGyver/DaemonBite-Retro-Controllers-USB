# DaemonBite PC Engine / TurboGrafx-16 Controllers To USB Adapter

## Introduction
This is a simple to build adapter for connecting PC Engine / TurboGrafx-16 controllers to USB with turbo functionality support.

The input lag for this adapter is minimal (should be less 1ms average connected to MiSTer).

## Parts you need
- Arduino Pro Micro (ATMega32U4)
- Female connector end of controller extension cable or...
- Female DIN 8-pin and/or Female Mini-DIN 8-pin connector
- Micro USB cable

## Wiring
| PC Engine (P1) | Arduino Pro Micro |
| ------ | ------ |
| 1 VCC | VCC |
| 2 UP/I | 3   PD0 |
| 3 RIGHT/II | 2   PD1 |
| 4 DOWN/SELECT | RXI PD2 |
| 5 LEFT/START | TXO PD3 |
| 6 DSELECT | 15  PB1 (Shared with P2) |
| 7 ENABLE | 14  PB3 (Shared with P2) |
| 8 GND | GND |

| PC Engine (P2) | Arduino Pro Micro |
| ------ | ------ |
| 1 VCC | VCC |
| 2 UP/I | A3  PF4 |
| 3 RIGHT/II | A2  PF5 |
| 4 DOWN/SELECT | A1  PF6 |
| 5 LEFT/START | A0  PF7 |
| 6 DSELECT | 15  PB1 (Shared with P1) |
| 7 ENABLE | 14  PB3 (Shared with P1) |
| 8 GND | GND |

## License
This project is licensed under the GNU General Public License v3.0.
