/* 
Tandy 1000/1000A/1000SX keyboard converter by Adrian Black
(Modified for maintainability by Rutger van Bergen)
Version 0.2 20-SEP-2020

Converts from AT/PS2 keyboard _TO_ the Tandy 1000 so you can use a regular keyboard with a Tandy.
Power provided by the Tandy so you can create a cable with the Arduino in the middle. 
A standard 5 pin DIN extension cable is all you need to make this work.

Warning: it's possible to insert the DIN 5 into the Tandy turned one or more pins!!
Make sure you have the center notch on the DIN-5 facing straight up -- or use the proper DIN cable to prevent this issue!

Original timing diagrams used from: http://www.kbdbabel.org/
Updating timing using Rigol DS1054Z and a real Tandy 1000 keyboard and Tandy 1000 Technical Reference Manual Page 105
Verified scancodes using scope but they are all listed in the Tandy 1000 (original) Technical manual (page 106-107)

Pinout: http://www.kbdbabel.org/conn/index.html

Idea and code based on this project: https://github.com/kesrut/pcxtkbd
Using some code snippets from here: https://github.com/techpaul/PS2KeyAdvanced
Uses this library to read raw codes from the PS2 keyboard: https://github.com/techpaul/PS2KeyRaw/find/master
 
Todo: 
Handle the Pause key
Handle the Print Screen key
Get the CAPS/NUM/SCROLL LED working 

Tandy 1000 8-pin DIN (using DIN 5 is fine but watch alignment!)
1 - Data
2 - Busy
3 - Ground
4 - Clock
5 - +5v
6 - Reset
7 - NC
8 - NC

IBM - Tandy
2 - 1 Data
4 - 3 Ground
5 - 5 +5v
1 - 4 Clock
*/

#include <PS2KeyRaw.h> // for library, see github repo above

#define PS2_CLK  3 // PS2 clock pin
#define PS2_DATA 4 // PS2 data pin

#define XT_CLK 2 // Tandy 1000 clock pin
#define XT_DATA 5 // Tandy 1000 data pin

bool capsLockOn = false; // tracking of caps lock status
bool numLockOn = false;  // tracking of num lock status
bool shiftOn = false; // tracking of shift key for \ and ` key

byte translationTable[256];

//
// PS2 raw keycodes
#define PS2_CODE_BREAK   0xF0 // sent before the key code when a key is released
#define PS2_CODE_EXTEND  0xE0 // sent before the key code in case the key is an "extended" key
#define PS2_MAX_KEYCODE 0x7F // maximum value of normal key codes

/* single byte key codes */
#define PS2_KEY_NUM      0x77
#define PS2_KEY_SCROLL   0x7E
#define PS2_KEY_CAPS     0x58
#define PS2_KEY_L_SHIFT  0x12
#define PS2_KEY_R_SHIFT  0x59
/* this is left CTRL and ALT; right version is PS2_CODE_EXTEND followed by the same code */
#define PS2_KEY_L_CTRL   0x14
#define PS2_KEY_L_ALT    0x11
#define PS2_KEY_ESC      0x76
#define PS2_KEY_BS       0x66
#define PS2_KEY_TAB      0x0D
#define PS2_KEY_ENTER    0x5A
#define PS2_KEY_SPACE    0x29
#define PS2_KEY_KP0      0x70
#define PS2_KEY_KP1      0x69
#define PS2_KEY_KP2      0x72
#define PS2_KEY_KP3      0x7A
#define PS2_KEY_KP4      0x6B
#define PS2_KEY_KP5      0x73
#define PS2_KEY_KP6      0x74
#define PS2_KEY_KP7      0x6C
#define PS2_KEY_KP8      0x75
#define PS2_KEY_KP9      0x7D
#define PS2_KEY_KP_DOT   0x71
#define PS2_KEY_KP_PLUS  0x79
#define PS2_KEY_KP_MINUS 0x7B
#define PS2_KEY_KP_TIMES 0x7C
#define PS2_KEY_KP_EQUAL 0x0F // Some keyboards have an '=' on right keypad
#define PS2_KEY_0        0x45
#define PS2_KEY_1        0x16
#define PS2_KEY_2        0x1E
#define PS2_KEY_3        0x26
#define PS2_KEY_4        0x25
#define PS2_KEY_5        0x2E
#define PS2_KEY_6        0x36
#define PS2_KEY_7        0x3D
#define PS2_KEY_8        0x3E
#define PS2_KEY_9        0x46
#define PS2_KEY_APOS     0x52
#define PS2_KEY_COMMA    0x41
#define PS2_KEY_MINUS    0x4E
#define PS2_KEY_DOT      0x49
#define PS2_KEY_DIV      0x4A
#define PS2_KEY_SINGLE   0x0E // key shared with tilde needs special handling
#define PS2_KEY_A        0x1C
#define PS2_KEY_B        0x32
#define PS2_KEY_C        0x21
#define PS2_KEY_D        0x23
#define PS2_KEY_E        0x24
#define PS2_KEY_F        0x2B
#define PS2_KEY_G        0x34
#define PS2_KEY_H        0x33
#define PS2_KEY_I        0x43
#define PS2_KEY_J        0x3B
#define PS2_KEY_K        0x42
#define PS2_KEY_L        0x4B
#define PS2_KEY_M        0x3A
#define PS2_KEY_N        0x31
#define PS2_KEY_O        0x44
#define PS2_KEY_P        0x4D
#define PS2_KEY_Q        0x15
#define PS2_KEY_R        0x2D
#define PS2_KEY_S        0x1B
#define PS2_KEY_T        0x2C
#define PS2_KEY_U        0x3C
#define PS2_KEY_V        0x2A
#define PS2_KEY_W        0x1D
#define PS2_KEY_X        0x22
#define PS2_KEY_Y        0x35
#define PS2_KEY_Z        0x1A
#define PS2_KEY_SEMI     0x4C
#define PS2_KEY_BACK     0x5D // backslash -- needs special handling
#define PS2_KEY_OPEN_SQ  0x54
#define PS2_KEY_CLOSE_SQ 0x5B
#define PS2_KEY_EQUAL    0x55
#define PS2_KEY_F1       0x05
#define PS2_KEY_F2       0x06
#define PS2_KEY_F3       0x04
#define PS2_KEY_F4       0x0C
#define PS2_KEY_F5       0x03
#define PS2_KEY_F6       0x0B
#define PS2_KEY_F7       0x83
#define PS2_KEY_F8       0x0A
#define PS2_KEY_F9       0x01
#define PS2_KEY_F10      0x09
#define PS2_KEY_F11      0x78
#define PS2_KEY_F12      0x07
#define PS2_KEY_KP_COMMA 0x6D

// extended key codes -- scan code is received with a preceeding PS2_CODE_EXTEND then the code below
// all of the keys below need special handling, except for PS2_KEY_HOME
#define PS2_KEY_IGNORE   0x12
#define PS2_KEY_PRTSCR   0x7C
#define PS2_KEY_L_GUI    0x1F // windows key
#define PS2_KEY_R_GUI    0x27
#define PS2_KEY_MENU     0x2F
#define PS2_KEY_BREAK    0x7E // Break is CTRL + PAUSE generated inside keyboard */
#define PS2_KEY_HOME     0x6C
#define PS2_KEY_END      0x69
#define PS2_KEY_PGUP     0x7D
#define PS2_KEY_PGDN     0x7A
#define PS2_KEY_INSERT   0x70
#define PS2_KEY_DELETE   0x71
#define PS2_KEY_KP_ENTER 0x5A
#define PS2_KEY_KP_DIV   0x4A

#define XT_KEY_NUM       0x45
#define XT_KEY_CAPS      0x3A
#define XT_KEY_8         0x09
#define XT_KEY_KP1       0x4F
#define XT_KEY_KP2       0x50
#define XT_KEY_KP3       0x51
#define XT_KEY_KP4       0x4B
#define XT_KEY_KP7       0x47
#define XT_KEY_KP8       0x48
#define XT_KEY_KP9       0x49
#define XT_KEY_EQUAL     0x0D
#define XT_KEY_ARROW_L   0x2B      
#define XT_KEY_ARROW_R   0x4E   
#define XT_KEY_ARROW_U   0x29   
#define XT_KEY_ARROW_D   0x4A
#define XT_KEY_L_SHIFT   0x2A
#define XT_KEY_R_SHIFT   0x36
#define XT_KEY_KP_ENTER  0x57
#define XT_KEY_DELETE    0x53
#define XT_KEY_INSERT    0x55
#define XT_KEY_HOME      0x58

#define XT_BIT_BREAK     0x80 // to tell the Tandy a key is released, OR the code of the key with this

PS2KeyRaw keyboard;

void setup() 
{
  delay(1000);
  setupTable(); 
  keyboard.begin(PS2_DATA, PS2_CLK);

  pinMode(XT_CLK, OUTPUT); 
  pinMode(XT_DATA, OUTPUT); 
  digitalWrite(XT_CLK, LOW); 
  digitalWrite(XT_DATA, HIGH); 
}

void toggleButton(bool *toggle, byte code) 
{
    if (*toggle) 
      code |= XT_BIT_BREAK;

    sendToTandy(code);
    *toggle = !*toggle;
}

void numLockForcedOnPressAndRelease(byte code)
{
  if (numLockOn) 
  { // numlock is already on, so just push and release the key
    sendToTandy(code);
    sendToTandy(code | XT_BIT_BREAK);
  }  
  else 
  { // numlock is off, so turn it on, press and release key and turn it back off
    sendToTandy(XT_KEY_NUM);
    sendToTandy(code);
    sendToTandy(code | XT_BIT_BREAK);
    sendToTandy(XT_KEY_NUM | XT_BIT_BREAK);
  } 
}

void numLockForcedOffPressAndRelease(byte code)
{
  if (numLockOn) 
  { // numlock is on, so turn it off, press and release key and turn it back on
    sendToTandy(XT_KEY_NUM | XT_BIT_BREAK);
    sendToTandy(code);
    sendToTandy(code | XT_BIT_BREAK);
    sendToTandy(XT_KEY_NUM);
  }
  else 
  { // numlock is already off, so just push and release the key 
    sendToTandy(code);
    sendToTandy(code | XT_BIT_BREAK); 
  }
}

void handleNormalKeyPress(int code)
{
  switch (code)
  {
  case PS2_KEY_KP_TIMES: // send a shift 8 for * 
    sendToTandy(XT_KEY_R_SHIFT);
    sendToTandy(XT_KEY_8);
    break; 

  case PS2_KEY_KP_PLUS: // send a shift = for +
    sendToTandy(XT_KEY_R_SHIFT);
    sendToTandy(XT_KEY_EQUAL); 
    break;

  case PS2_KEY_L_SHIFT: 
    sendToTandy(XT_KEY_L_SHIFT); 
    shiftOn = true; 
    break;
  
  case PS2_KEY_R_SHIFT: 
    sendToTandy(XT_KEY_R_SHIFT); 
    shiftOn = true; 
    break;      

  case PS2_KEY_CAPS: // capslock -- we have to do special handling and state tracking
    toggleButton(&capsLockOn, XT_KEY_CAPS);
    break;
  
  case PS2_KEY_NUM: // numlock -- we have to do special handling and state tracking
    toggleButton(&numLockOn, XT_KEY_NUM);
    break;

  case PS2_KEY_BACK:   
    if (shiftOn) // shift is on, so we send | via numeric keypad 4 with numlock forced ON, to reverse the shift
      numLockForcedOnPressAndRelease(XT_KEY_KP4);
    else // shift is off, so we send \ via numeric keypad 7 with numlock forced OFF
      numLockForcedOffPressAndRelease(XT_KEY_KP7);
    break; 

  case PS2_KEY_SINGLE:  
    if (shiftOn) // shift is on, so we send ~ via numeric keypad 8 with numlock forced ON, to reverse the shift
      numLockForcedOnPressAndRelease(XT_KEY_KP8);
    else // shift is off, so we send ` via numeric keypad 2 with numlock forced OFF
      numLockForcedOffPressAndRelease(XT_KEY_KP2);
    break;

  default: // no special handling, use the translation table to send the right key
    sendToTandy(translationTable[code]);
  }
}

void handleNormalKeyRelease(int code)
{
  switch (code)
  {
  case PS2_KEY_KP_TIMES: 
    sendToTandy(XT_KEY_8 | XT_BIT_BREAK);
    sendToTandy(XT_KEY_R_SHIFT | XT_BIT_BREAK); 
    break;
  
  case PS2_KEY_KP_PLUS: 
    sendToTandy(XT_KEY_EQUAL | XT_BIT_BREAK);
    sendToTandy(XT_KEY_R_SHIFT | XT_BIT_BREAK); 
    break;

  case PS2_KEY_L_SHIFT: 
    sendToTandy(XT_KEY_L_SHIFT | XT_BIT_BREAK); 
    shiftOn = false; 
    break;
  
  case PS2_KEY_R_SHIFT: 
    sendToTandy(XT_KEY_R_SHIFT | XT_BIT_BREAK); 
    shiftOn = false; 
    break;     
          
  // the release of the following keys are communicated to the Tandy when the key is pressed, so we must not do anything here 
  case PS2_KEY_CAPS: 
  case PS2_KEY_NUM: 
  case PS2_KEY_BACK:
  case PS2_KEY_SINGLE: 
    break;

  default: // this is the "break" signal -- when you let go of a key -- so need to send the scancode to the computer OR'ed with XT_BIT_BREAK which tells it you let go of the key
    sendToTandy(translationTable[code] | XT_BIT_BREAK);
  }
}

void handleSpecialKeyPress(int code)
{
  switch (code) 
  {
  case PS2_KEY_KP4: // left arrow
    sendToTandy(XT_KEY_ARROW_L); 
    break; 
  
  case PS2_KEY_KP6: // right arrow
    sendToTandy(XT_KEY_ARROW_R); 
    break;  
  
  case PS2_KEY_KP8: // up arrow
    sendToTandy(XT_KEY_ARROW_U); 
    break;  
  
  case PS2_KEY_KP2: // down arrow
    sendToTandy(XT_KEY_ARROW_D); 
    break;  
  
  case PS2_KEY_ENTER: // keypad enter
    sendToTandy(XT_KEY_KP_ENTER); 
    break; 

  case PS2_KEY_DELETE: // the delete key is on the numpad on the Tandy, and only works if numlock is off
    numLockForcedOffPressAndRelease(XT_KEY_DELETE);       
    break;

  case PS2_KEY_INSERT: // the insert key is on the numpad on the Tandy, and only works if numlock is off
    numLockForcedOffPressAndRelease(XT_KEY_INSERT); 
    break;

  case PS2_KEY_PGUP: // the page up key is on the numpad on the Tandy, and only works if numlock is off
    numLockForcedOffPressAndRelease(XT_KEY_KP9); 
    break;

  case PS2_KEY_PGDN: // the page down key is on the numpad on the Tandy, and only works if numlock is off
    numLockForcedOffPressAndRelease(XT_KEY_KP3);
    break;

  case PS2_KEY_END: // the end key is on the numpad on the Tandy, and only works if numlock is off 
    numLockForcedOffPressAndRelease(XT_KEY_KP1);
    break;

  default: // any other special keys are handled through the translation table
    sendToTandy(translationTable[code]);
  }
}

void handleSpecialKeyRelease(int code) 
{
  switch (code)
  {
  case PS2_KEY_KP4: // left arrow
    sendToTandy(XT_KEY_ARROW_L | XT_BIT_BREAK); 
    break;  

  case PS2_KEY_KP6: // right arrow 
    sendToTandy(XT_KEY_ARROW_R | XT_BIT_BREAK); 
    break; 

  case PS2_KEY_KP8: // up arrow
    sendToTandy(XT_KEY_ARROW_U | XT_BIT_BREAK); 
    break; 

  case PS2_KEY_KP2: // down arrow
    sendToTandy(XT_KEY_ARROW_D | XT_BIT_BREAK); 
    break; 

  case PS2_KEY_ENTER: // keypad enter
    sendToTandy(XT_KEY_KP_ENTER | XT_BIT_BREAK); 
    break; 

  // the release of the following keys are communicated to the Tandy when the key is pressed, so we must not do anything here
  case PS2_KEY_DELETE:  
  case PS2_KEY_INSERT: 
  case PS2_KEY_PGUP: 
  case PS2_KEY_PGDN: 
  case PS2_KEY_END: 
    break;
    
  default: // this is the "break" signal -- when you let go of a key -- so need to send the scancode to the computer OR'ed with XT_BIT_BREAK which tells it you let go of the key
    sendToTandy(translationTable[code] | XT_BIT_BREAK);
  }
}

void loop() 
{
  if (!keyboard.available()) // check if byte is waiting to be read out of buffer, otherwise we're done for now
    return;

  int c = keyboard.read(); // read the keyboard buffer byte into variable c
   
  if (c <= PS2_MAX_KEYCODE)  // normal PS2 key codes without any special or special commands
  {
    handleNormalKeyPress(c);
  }
  else if (c == PS2_CODE_BREAK) // byte that is sent when a key is released. 
  {
    delay(10);
    handleNormalKeyRelease(keyboard.read());
  }
  else if (c == PS2_CODE_EXTEND) // special keys -- like ones that are replicated and have PS2_CODE_EXTEND first so you can tell them apart
  {
    delay(10);
    c = keyboard.read(); // first byte signaled a special (extended) key, so read the next byte which will be the key pushed
    
    if (c == PS2_CODE_BREAK)  // oh actually, a special key was released, so we need to handle that.
    {
      delay(10); // wait a little for the MCU to read the byte from the keyboard
      handleSpecialKeyRelease(keyboard.read());
    }
    else 
      handleSpecialKeyPress(c);
  }  
}

void setupTable () // translate from PS2 keycode to Tandy 1000
{  
  for (int i = 0; i < 256; i++) 
    translationTable[i] = 0x39;  // initialize the whole table so defaults anything not mapped to space

  // left side is the PS2 scan-code and right side is what to send to the Tandy 1000
  translationTable[PS2_KEY_0] = 0x0B;
  translationTable[PS2_KEY_1] = 0x02;
  translationTable[PS2_KEY_2] = 0x03;
  translationTable[PS2_KEY_3] = 0x04;
  translationTable[PS2_KEY_4] = 0x05;
  translationTable[PS2_KEY_5] = 0x06;
  translationTable[PS2_KEY_6] = 0x07;
  translationTable[PS2_KEY_7] = 0x08;
  translationTable[PS2_KEY_8] = XT_KEY_8;
  translationTable[PS2_KEY_9] = 0x0A;
  translationTable[PS2_KEY_APOS] = 0x28; 
  translationTable[PS2_KEY_A] = 0x1E;
  translationTable[PS2_KEY_BREAK] = 0x54;
  translationTable[PS2_KEY_BS] = 0x0E;
  translationTable[PS2_KEY_B] = 0x30;
  translationTable[PS2_KEY_CLOSE_SQ] = 0x1B;
  translationTable[PS2_KEY_COMMA] = 0x33;
  translationTable[PS2_KEY_C] = 0x2E;
  translationTable[PS2_KEY_DIV] = 0x35;
  translationTable[PS2_KEY_DOT] = 0x34;
  translationTable[PS2_KEY_D] = 0x20;
  translationTable[PS2_KEY_ENTER] = 0x1C;
  translationTable[PS2_KEY_EQUAL] = XT_KEY_EQUAL;  
  translationTable[PS2_KEY_ESC] = 0x01;
  translationTable[PS2_KEY_E] = 0x12;
  translationTable[PS2_KEY_F10] = 0x44;
  translationTable[PS2_KEY_F11] = 0x59;
  translationTable[PS2_KEY_F12] = 0x5A;
  translationTable[PS2_KEY_F1] = 0x3B;
  translationTable[PS2_KEY_F2] = 0x3C;
  translationTable[PS2_KEY_F3] = 0x3D;
  translationTable[PS2_KEY_F4] = 0x3E;
  translationTable[PS2_KEY_F5] = 0x3F;
  translationTable[PS2_KEY_F6] = 0x40;
  translationTable[PS2_KEY_F7] = 0x41;
  translationTable[PS2_KEY_F8] = 0x42;
  translationTable[PS2_KEY_F9] = 0x43;
  translationTable[PS2_KEY_F] = 0x21;
  translationTable[PS2_KEY_G] = 0x22;
  translationTable[PS2_KEY_H] = 0x23;
  translationTable[PS2_KEY_I] = 0x17;
  translationTable[PS2_KEY_J] = 0x24;
  translationTable[PS2_KEY_KP0] = 0x52;
  translationTable[PS2_KEY_KP1] = XT_KEY_KP1;
  translationTable[PS2_KEY_KP2] = XT_KEY_KP2;
  translationTable[PS2_KEY_KP3] = XT_KEY_KP3;
  translationTable[PS2_KEY_KP4] = XT_KEY_KP4;
  translationTable[PS2_KEY_KP5] = 0x4C;
  translationTable[PS2_KEY_KP6] = 0x4D;
  translationTable[PS2_KEY_KP7] = XT_KEY_KP7; 
  translationTable[PS2_KEY_KP8] = XT_KEY_KP8; 
  translationTable[PS2_KEY_KP9] = XT_KEY_KP9;
  translationTable[PS2_KEY_KP_DIV] = 0x35; 
  translationTable[PS2_KEY_KP_DOT] = 0x56;
  translationTable[PS2_KEY_KP_ENTER] = XT_KEY_KP_ENTER; 
  translationTable[PS2_KEY_KP_MINUS] = 0x0C;
  translationTable[PS2_KEY_K] = 0x25;
  translationTable[PS2_KEY_L] = 0x26;
  translationTable[PS2_KEY_L_ALT] = 0x38; 
  translationTable[PS2_KEY_L_CTRL] = 0x1D;
  translationTable[PS2_KEY_L_SHIFT] = XT_KEY_L_SHIFT;
  translationTable[PS2_KEY_MINUS] = 0x0C;
  translationTable[PS2_KEY_M] = 0x32;
  translationTable[PS2_KEY_N] = 0x31;
  translationTable[PS2_KEY_OPEN_SQ] = 0x1A;
  translationTable[PS2_KEY_O] = 0x18;
  translationTable[PS2_KEY_PRTSCR] = 0x37;
  translationTable[PS2_KEY_P] = 0x19;
  translationTable[PS2_KEY_Q] = 0x10;
  translationTable[PS2_KEY_R] = 0x13;
  translationTable[PS2_KEY_R_SHIFT] = XT_KEY_R_SHIFT; 
  translationTable[PS2_KEY_SCROLL] = 0x46;
  translationTable[PS2_KEY_SEMI] = 0x27;
  translationTable[PS2_KEY_SINGLE] = XT_KEY_KP2;
  translationTable[PS2_KEY_SPACE] = 0x39;
  translationTable[PS2_KEY_S] = 0x1F;
  translationTable[PS2_KEY_TAB] = 0x0F;
  translationTable[PS2_KEY_T] = 0x14;
  translationTable[PS2_KEY_U] = 0x16;
  translationTable[PS2_KEY_V] = 0x2F;
  translationTable[PS2_KEY_W] = 0x11;
  translationTable[PS2_KEY_X] = 0x2D;
  translationTable[PS2_KEY_Y] = 0x15;
  translationTable[PS2_KEY_Z] = 0x2C;
  translationTable[PS2_KEY_HOME] = XT_KEY_HOME;
}

void sendToTandy(byte value) // routine that writes to the data and clock lines to the Tandy
{
  byte bits[8];  

  for (int i = 0; i < 8; i++)
  {
    bits[i] = value & 1;    
    value >>= 1; 
  }

  while (digitalRead(XT_CLK) != LOW);

  for (int i = 0; i < 8; i++)
  {
    digitalWrite(XT_DATA, bits[i]);
    delayMicroseconds(5);
    digitalWrite(XT_CLK, HIGH);
    delayMicroseconds(7);
    digitalWrite(XT_DATA, HIGH);
    delayMicroseconds(5);
    digitalWrite(XT_CLK, LOW);
  } 

  digitalWrite(XT_CLK, LOW);
  delayMicroseconds(10);
  digitalWrite(XT_DATA, LOW);
  delayMicroseconds(5);
  digitalWrite(XT_DATA, HIGH);
  delay(10);
}