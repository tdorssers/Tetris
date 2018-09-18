# ATtiny45 Tetris

Tetris game for an Atmel ATtiny45 using a SSD1306 based 128x32 pixel OLED screen.

## Overview

The SSD1306 controller is capable of driving an 128x64 OLED screen and has 1K SRAM. When driving an 128x32 OLED, only 512 bytes are used. The ATtiny45 has only 256 bytes SRAM, which is not enough to hold a frame buffer. Therefore the screen is rendered per line of 32 bits and sent over the I2C bus to the display controller.

The Tetris game uses a 10x30 playing field and implements hard and soft drop of the pieces. There is a piece preview and hold function as well.

## Schematic

![](schematic/Tetris.png)

## Prototype

![](media/Prototype.jpg)
