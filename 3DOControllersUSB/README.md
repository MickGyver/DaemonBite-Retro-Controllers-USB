# DaemonBite 3DO USB Controller adapter
## Introduction
With this simple to build  adapter you can connect a 3DO gamepad (specifically tested using a Panasonic FZ-JP1 controller) to a PC, Raspberry PI, MiSTer FPGA etc. The Arduino Pro Micro has very low lag when configured as a USB gamepad and it is plug n' play once it has been programmed.

Only one controller is currently supported.

The headphone jack is not supported.

## Parts you need
- Arduino Pro Micro (ATMega32U4)
- Male DB9 plug
- Wire
- Heat shrink tube (Ø ~20mm)
- Micro USB cable

## Wiring
Male DB9 socket pins (look at the pins in the socket):

|   DB9     |
|-----------|
|\1 2 3 4 5/|
| \6 7 8 9/ |

3DO-DB9                    | Arduino Pro Micro
---------------------------|------------------
1 GND                      | GND
2 VCC(5v)                  | VCC
3 Audio.1(1v-pp)           |
4 Audio.1(1v-pp)           |
5 VCC(5v)                  | VCC
6 P/S (Shift/Load)         | 2    (PD1)
7 CLK(125KHz) (Shift/Clock)| 3    (PD0)
8 GND                      | GND
9 DATA                     | A0   (PF7)

## License
This project is licensed under the GNU General Public License v3.0.
