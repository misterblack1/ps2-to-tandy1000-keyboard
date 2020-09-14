# PS2/AT Keyboard to Tandy 1000 Keyboard Adapter

Tandy 1000/1000A/1000SX keyboard converter by Adrian Black
Version 0.1 10-AUG-2020

Parts needed:
- Arduino Pro Mini or any other Atmel ATMEGA 168/368 board
- Arduino IDE
- PS2 or AT Keyboard
- DIN 5 Female and DIN 8 or Din 5 Male
- Some wire

Converts from AT/PS2 keyboard _TO_ the Tandy 1000 so you can use a regular keyboard with a Tandy 1000! Should work on the Tandy 1000, 1000A, 1000SX and possibly others that use the original Tandy Keyboard. 

Power provided by the Tandy so you can create a cable with the arduino in the middle. 

A standard 5 pin DIN extension cable is all you need to make this work or ideally you can make your own cable using a DIN 8 on the Tandy side and either a PS2 female or a DIN-5 Female (AT keyboard) connetor.

Warning it's possible to insert the DIN 5 into the Tandy turned one or more pins! Make sure you have the center notch on the DIN-5 facing straight up -- or use the proper DIN cable to prevent this issue!

Original timing diagrams used from: http://www.kbdbabel.org/
Updating timing using Rigol DS1054Z and a real Tandy 1000 keyboard and Tandy 1000 Technical Reference Manual Page 105
Verified scancodes using scope but they are all listed in the Tandy 1000 (original) Technical manual (page 106-107)

Pinouts for various keyboard: http://www.kbdbabel.org/conn/index.html

Idea and code based on this project: https://github.com/kesrut/pcxtkbd
Using some code snippets from here: https://github.com/techpaul/PS2KeyAdvanced
Uses this library to read raw codes from the PS2 keyboard: https://github.com/techpaul/PS2KeyRaw/find/master
 
Todo: 
- Handle the Pause key
- Handle the print screen key
- Get the CAPS/NUN/SCROLL LED working 

Tandy 1000 8-pin DIN (using DIN 5 is fine but watch alignment!)
- 1 - Data
- 2 - Busy
- 3 - Ground
- 4 - Clock
- 5 - +5v
- 6 - Reset
- 7 - NC
- 8 - NC

IBM - Tandy Mapping
- 2 - 1 Data
- 4 - 3 Ground
- 5 - 5 +5v
- 1 - 4 Clock

(I am ignoring the Reset and Busy line. The machine seems to work perfectly without them.)
