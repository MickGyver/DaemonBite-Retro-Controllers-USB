# 5x DaemonBite SNES/NES USB Controller adapter  
## (modified version that supports 5 SNES controllers)

## Introduction
Based on the excellent work in the original project, this version will support 5 SNES gamepads at the same time.

Work around for this issue:
https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB/issues/3

"The CDC Serial uses 3 endpoints. This means you can add up to 3 devices for the 32u4 and 1 for the 8/16/32u2. If you add more, some will be ignored."

I found a fix, that others have used when creating usb keyboards. It disables the CDC to free up resources. https://github.com/gdsports/usb-metamorph/tree/master/USBSerPassThruLine


## Original text and parts information:
With this simple to build  adapter you can connect NES gamepads to a PC, Raspberry PI, MiSTer FPGA etc. The Arduino Pro Micro has very low lag when configured as a USB gamepad and it is plug n' play once it has been programmed. 

## Parts
- Arduino Pro Micro (ATMega32U4)
- Male end of NES controller extension cable
- Heat shrink tube (Ø ~20mm)
- Micro USB cable

## Updated 5-port Wiring
![Assemble1](images/snes-usb-adapter-wiring%20-5player.png)

## Example using tribal tap 5 port tap
 -In my case, I built this into an old tribal tap device.  I removed the IC (which never actually supported all 5 ports) and wired the arduino into the existing PCB in place of the right pins.  The board fits right in, and usb routes out the back of the device in place of the original connector.
 
 
					      (Arduino Pro Micro: Pin3)	SNES, clk - I0 -  1   V   20 - Vcc  (Arduino Pro Micro: VCC)
												                          SNES, IO - I1 -  2       19 - B7 - SNES, D0
					        (Arduino Pro Micro: Pin15)		P2,d0 - I2 -  3       18 - B6 - SNES, D1
							                            						P2,d1 - I3 -  4       17 - B5 - P2,latch (Arduino Pro Micro: Pin2)
										                   deals with switch - I4 -  5       16 - B4 - P2,IO
				      	  (Arduino Pro Micro: Pin16)		P3,d0 - I5 -  6       15 - B3 - P3,clk (Arduino Pro Micro: Pin3)
			 (Arduino Pro Micro: RST)	deals with switch - I6 -  7       14 - B2 - P4-P6,clk (Arduino Pro Micro: Pin3)
					         (Arduino Pro Micro: Pin8)		P5,d0 - I7 -  8       13 - B1 - P3-P6,latch (Arduino Pro Micro: Pin2)
					        (Arduino Pro Micro: Pin14)		P4,d0 - I8 -  9       12 - B0 - P6,d0 (Arduino Pro Micro: Pin9)
								             (Arduino Pro Micro: GND)   Gnd - 10       11 - I9 - "inverted" latch
								

![Assemble1](images/tap1.jpg)
![Assemble1](images/tap2.jpg)
![Assemble1](images/tap3.jpg)

## License
This project is licensed under the GNU General Public License v3.0.
