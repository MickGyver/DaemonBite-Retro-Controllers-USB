# DaemonBite Retro Controllers To USB Adapters
## Introduction
This is a collection of easy to build adapters for connecting Mega Drive/Genesis (3/6-button), Master System, Atari, Commodore, Amiga (incl. CD32) controllers to USB. Support for more controllers is on the way (NES, SNES, NeoGeo etc.).

The input lag for these adapters is minimal. Here is the result of the Sega controller adapter from a test with a 1ms polling rate on a MiSTer:

| Controller | Samples | Average | Max | Min | Std Dev |
| ------ | ------ | ------ | ------ | ------ | ------ | 
| Original 3-Button Mega Drive Controller | 2342 | 0.75ms | 1.28ms | 0.24ms | 0.29ms |
| 8bitdo M30 Wireless 2.4G | 2348 | 4.54ms | 8.05ms | 2.22ms | 1.31ms |

## How to build
See the README files in the subfolders for build instructions. All the adapters are build around the Arduino Pro Micro.

## License
This project is licensed under the GNU General Public License v3.0.

## Credits
The Mega Drive gamepad interface is based on this repository : https://github.com/jonthysell/SegaController but almost entirely rewritten and a lot of optimisations have been made.
