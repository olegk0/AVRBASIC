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

*All variables only integer type (-32768...32768), variable name - one uppercase character in the range A to F

*Not create new files on SD, can only overwrite old. Therefore it is necessary to prepare template files (example folder utils) and write them on a SD card.

*enter commands like Sinclair BASIC48

Shortkeys
---------
*ESC - Break / Clear command line  (Ctrl+B in Linux console)

*CapsLock - switch keyboard layout Rus/Lat  (Rus(cp1251) only for text or remarks)

not available yet//*Ctrl+C - Reset

:Commands
---------
b        BEEP E , E 

c        CLS 

c :Alt CLEAR 

f        FOR V = E TO E 

g :Alt GOSUB E 

g        GOTO E 

h        HELP A 

i        IF E THEN C                                            

i :Alt INPUT V 

l        LET V = E 

l :Alt LOCATE E , E 

l        LIST n 

l :Alt LOAD S 

m        MEM 

n        NEW

n        NEXT V 

o        OUT N , E 

p :Alt PAUSE E 

p        PRINT M 

r        REM A 

r :Alt RETURN 

r        RUN                                                    

s        SAVE S 

s :Alt STOP

:Functions
----------
i        IN N 

i :Alt INKEY 

r        RND 



project folder
--------------
https://drive.google.com/folderview?id=0B6QRwjacGTzCc01Gcl9DWVNFMEU&usp=sharing
