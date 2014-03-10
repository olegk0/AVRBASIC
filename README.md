AVRBASIC
========

BASIC interpreter for atmega328 (Arduino)

Warning! WIP(Work In Progress) status
-------------------------------------
build for AVR - make

for linux console - Make_it

Hardware:
---------
*atmega328

*display 128х64 on st7565 (SED1565)

*SD card with holder

*PS/2 keyboard

*a few simple parts


Restrictions:
-------------
*Maximum size of the program in 1200 bytes or 255 lines

*All variables only integer type (-32768...32768), variable name - one uppercase character in the range A to Z

*Not create new files on SD, can only overwrite old. Therefore it is necessary to prepare template files (example folder utils) and write them on a SD card.

*enter commands like Sinclair BASIC48

Shortkeys
---------
*ESC - Break / Clear command line  (Ctrl+B in Linux console)

*CapsLock - switch keyboard layout Rus/Lat  (Rus(cp1251) only for text or remarks)

not available yet//*Ctrl+C - Reset


project folder
--------------
https://drive.google.com/folderview?id=0B6QRwjacGTzCc01Gcl9DWVNFMEU&usp=sharing
