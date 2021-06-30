# DaemonBite PSX Controller To USB Adapter
## Introduction
This is a simple to build adapter for connecting PSX controllers to USB. Currently it supports normal digital PSX controllers, analog sticks are not supported.

The input lag for this adapter is minimal (about 0.75ms average connected to MiSTer).

## Parts you need
- Arduino Pro Micro (ATMega32U4)
- Male end of a PSX controller extension cable
- Heat shrink tube (Ø ~20mm)
- Micro USB cable

## Wiring
PSX controller socket (looking face-on at the front of the socket):
```
_________________________
| 9 8 7 | 6 5 4 | 3 2 1 |
\_______________________/
```

PSX controller plug (looking face-on at the front of the controller plug):
```
_________________________
| 1 2 3 | 4 5 6 | 7 8 9 |
\_______________________/
```

| PSX | Arduino Pro Micro |
| ------ | ------ |
| 1 DATA  | A0  PF7 |
| 2 CMD   | 2   PD1 |
| 3 +7.6V ||
| 4 GND   | GND |
| 5 VCC   | VCC |
| 6 ATT   | 3   PD0 |
| 7 CLK   | 4   PD4 |
| 8 N/C   ||
| 9 ACK   ||


## License
This project is licensed under the GNU General Public License v3.0.

## Credits
PSX Library from Arduino Playground: https://playground.arduino.cc/Main/PSXLibrary/