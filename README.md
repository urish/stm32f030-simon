# Simon Game for STM32F030

Copyright (C) 2016-2019, Uri Shaked

Released under the MIT License.

## Required Hardware

* 4 LEDs connected to pins PA0, PA1, PA2, PA3
* 4 buttons between pins PA4, PA5, PA6, PA7 and ground
* A speaker between PB1 and VCC
* Optional: 7-Segment display with 74HC595 shift register on pins PF0, PF1, PA10

## Building the Code

Install the [STM32 core support for Arduino](https://github.com/stm32duino/wiki/wiki/Getting-Started).

When compiling the code, choose the following options:
* Board: Generic STM32F0 Series
* Board part number: STM32F030F4 Demo board (internal RC oscillator)
* Optimize: Smallest (-O2) with LTO
