# Luminardo 1284P: Platform files for Luminardo on ATmega1284P

## What is it?

Everything you need to run Luminardo on ATmega1284P.

## Current state

Alpha revision

## Installation

1. Download the [ZIP File](***TBD***)
2. Unzip it a folder called 'hardware'of your sketches directory, e.g. /Users/magictaler/Source/Arduino/hardware/luminardo-1284p
3. Restart the IDE
4. Select Tools > Board > Luminardo 1284p 10MHz
5. To burn the bootloader, follow the Arduino [Bootloader](http://arduino.cc/en/Hacking/Bootloader) instructions.

## Requirements

* Works only on Arduino >= 1.0
* Can be burned using [USBtinyISP](http://www.ladyada.net/make/usbtinyisp/).  That programmer reports errors when flashing to chips with >64k flash size hovewer still does the job.

## See also

http://atmega.magictale.com/Luminardo

## Supported Boards

* 'Luminardo 1284p 10MHz'.  The main and the only board for now.
