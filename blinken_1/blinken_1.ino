#include <Blinken.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

//Flash based string table. Necessary because otherwise the strings will take up all of our ram.
//Use either PrintLCDAt_P or PrintLCD_P to print these strings to the LCD.
prog_char string_0[] PROGMEM =   "RAINBOW ";  //0
prog_char string_1[] PROGMEM =   "RED     ";  //1
prog_char string_2[] PROGMEM =   "GREEN   ";  //2
prog_char string_3[] PROGMEM =   "BLUE    ";  //3
prog_char string_4[] PROGMEM =   "5. fooo ";  //4
prog_char string_5[] PROGMEM =   "6. foo  ";  //5
prog_char string_6[] PROGMEM =   "SELECT: ";  //6


PROGMEM PGM_P StringTable[] = {
  string_0, string_1, string_2, string_3, string_4, string_5, string_6
};


#define dataPin 12 //data bus for blinkenlichten

#define buttonBasePin 0 //analog input for keymatrix

#define KeyUp    3
#define KeyDown  4
#define KeySel   5
#define KeyInv   6   //invalid key value


#define debounceDelay 30    // debounce time for button reading
unsigned long lastDebounceTime = 0;  // keep millis() for dutton debounce
int lastButtonState = 0;   // the previous reading from the input pin

int gdelay=100; // global delay for current mode
byte gcolor[3];  // stores RGB values globally
byte goption=0; // number of selcted menu option

unsigned int gcount;    // global counter for light effects
unsigned long previousMillis = 0;    // last time of processed sub-routine

#define buttonSendDelay 400  // wait this time until next valid key press is accepted
unsigned long lastButtonSend = 0;    // stores millis of last send after keyscan

#define lcdUpdateInterval 2000    // refresh LCD content from time to time
unsigned long lcdLastUpdated = 0;


//LiquidCrystal(rs, enable, d4, d5, d6, d7) set LCD input pins
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

Blinken blinken(dataPin);  // set bus pin for Blinkenlichten

byte blinkens[255]; // preallocate an array to hold a list of Blinkens (IDs) on the bus
byte nBlinkens=0;  // how many were discovered


void setup()
{
  lcd.begin(8, 2); // LCD with 2 rows, 8 chars each

  //"bootscreen" with some text and counter of devices found on the bus
  //lcd.print("Blinkens");
  //lcd.setCursor(0, 1);
  //lcd.print("Found");
  PrintLCDAt("Blinkens", 0, 0);
  PrintLCDAt("Found", 0, 1);
  lcd.setCursor(6, 1);

  blinken.reset(); // Make sure if the Adruino was powered but not active (e.g. during code upload) and pins may have been floating, everyone is back in sync
  Serial.begin(9600);
  Serial.println("Scan blinkens");
  delay(500);

  for (byte i=1; i<0xFF; i++)    // "i<=0xFF" creates an infinite loop, wtf?
  {
    if(blinken.identify(i)) // check if anyone responds to this address
    {
      Serial.print("ID ");
      Serial.print(i, HEX);
      Serial.print(" responded\n");
      blinkens[nBlinkens]=i;    // if something responded, add its ID to the list of active IDs
      nBlinkens++; // and update the total count. blinkens[0 .. nBlinkens-1] are valid IDs.
      lcd.print(nBlinkens, DEC);
    }
  }
  lcdLastUpdated = millis();  // we just wrote the welcome screen, keep it for the "lcdUpdateInterval"
  mainMenu();
}


void loop()
{
}

void mainMenu() {

  byte whichkey, offset=0;

  do {
  
    whichkey = PollKey();
    switch(whichkey) {
    case KeyUp:
      Serial.println("up");
      if (offset > 0) offset--;
      break;
    case KeyDown:
      Serial.println("down");
      if (offset < 5) offset++;
      break;
    case KeySel:
      switch (offset) {
      case 0: // show rainbow mode. pressing select key again increases delay
        //Serial.println("m0");
        if (goption == 0) {
          if (gdelay >= 0 && gdelay <= 500) {
           gdelay += 50;
          } else {
           gdelay = 0;
          }
          //lcd.setCursor(0, 0);
          //lcd.print("Rate:");
          PrintLCDAt("Rate:", 0, 0);
          lcd.setCursor(5, 0);
          lcd.print(gdelay, DEC);
          lcdLastUpdated = millis();  // we just wrote to the LCD, keep the time
        }
        else {
          gcount = 0;
          gdelay = 200;
        }
        //rainbow2(gdelay);
        goption = 0;
        break;
      case 1:              // red
        //Serial.println("m1");
        goption = 3;
        if (gcolor[0] == 8) {
          gcolor[0] = 0;
        }
        else {
          gcolor[0] += 1;
        }
        gdelay = 500;  // update every 5 seconds
        previousMillis = millis() + gdelay*10;  // update now
        break;
      case 2:               // green
        //Serial.println("m2");
        goption = 3;
        if (gcolor[1] == 8) {
          gcolor[1] = 0;
        }
        else {
          gcolor[1] += 1;
        }
        gdelay = 500;  // update every 5 seconds
        previousMillis = millis() + gdelay*10;
        break;
      case 3:               // blue
        //Serial.println("m3");
        goption = 3;
        if (gcolor[2] == 8) {
          gcolor[2] = 0;
        }
        else {
          gcolor[2] += 1;
        }
        gdelay = 500;  // update every 5 seconds
        previousMillis = millis() + gdelay*10;
        break;
      case 4: // 
        //Serial.println("m4");
        //listUsers();
        break;
      case 5: // 
        //Serial.println("m5");
        //optionsMenu();
        break;
      }
      break;
    //case KeyInv: //nothing pressed or false read, continue with what we did
    }
    switch (goption) {
    case 0:
      rainbow2(gdelay);
      break;
    case 1:
      //foo
      break;
    case 3:
      setColor(gcolor, gdelay);
      break;
    } // end switch (goption)
    
    // udate the display periodically and on up/down key presses
    if ((millis() - lcdLastUpdated) > lcdUpdateInterval || whichkey == KeyUp || whichkey == KeyDown) {
//    PrintLCD_P(6); //"	Main Menu     "
    PrintLCDAt_P(6, 0, 0);
    PrintLCDAt_P(offset, 0, 1);
    lcdLastUpdated = millis();
    }
    
  }
  while (1 == 1);
}  // end of function MainMenu


void rainbow2(int gdelay) {
  //unsigned long currentMillis = millis();
  byte r, g, b;
//  if ((currentMillis - previousMillis) > gdelay) {
  if ((millis() - previousMillis) > gdelay*10) {
    previousMillis = millis();

/*
0 0 0
8 0 0
0 8 0
0 0 8
8 0 8
8 8 0
8 8 8
0 8 8
*/
    for (int which=0; which < nBlinkens; which++)
    {
      if (gcount >= 63 ) { gcount = 0; }
      if (gcount <= 8)                  { r = gcount;          g = 8-gcount;      b = 8-gcount;   }
      if (gcount >= 9  && gcount < 18 ) { r = 8-(gcount-9);    g = gcount-9;      b = 0;  }
      if (gcount >= 18 && gcount < 27 ) { r = 0;               g = 8-(gcount-18); b = gcount-18;  }
      if (gcount >= 27 && gcount < 36 ) { r = gcount-27;       g = 0;             b = 8;  }
      if (gcount >= 36 && gcount < 45 ) { r = 8;               g = gcount-36;     b = 8-(gcount-36);  }
      if (gcount >= 45 && gcount < 54 ) { r = 8;               g = 8;             b = gcount-45;  }
      if (gcount >= 54 && gcount < 63 ) { r = 8-(gcount-54);   g = 8;             b = 8;  }
      
      blinken.set_color(blinkens[which],r,g,b);
/*      Serial.print(r);
      Serial.print(g);
      Serial.println(b);
*/
    }
    gcount++;
  }
}  // end of function rainbow2


// Set the color of all Blinkens
void setColor(byte gcolor[3], int gdelay) {
  
  // only run on specified update interval
  if ((millis() - previousMillis) > gdelay*10) {
    previousMillis = millis();
    
  for (int which=0; which < nBlinkens; which++)
    {
      blinken.set_color(blinkens[which],gcolor[0],gcolor[1],gcolor[2]);
    }
  }
  
}  // end of function setColor


//Scan the key pin and determine which key was pressed. Then return a code specifying each key
//There is a fuzzy range for each key. This is a central space for all key presses. That way it's
//easy to tweak these values if need be.
char KeyScan() {
  int reading;
  char retVal;
  
  reading = analogRead(buttonBasePin);

  retVal = KeyInv;  // Invalid key by default
  
  // If the switch changed, due to noise or pressing:
  //if (reading < abs(lastButtonState-10) || reading > abs(lastButtonState+10)) {
  if (reading < lastButtonState-8 || reading > lastButtonState+8) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
////    buttonState = reading;
    //three AD values by a resistor array identify the buttons
    if (reading > 240 && reading < 260) retVal =  KeySel;
    if (reading > 500 && reading < 530) retVal =  KeyDown;
    if (reading > 750 && reading < 780) retVal =  KeyUp;  
  }
  lastButtonState = reading;

  return retVal;
}  // end of function KeyScan

//calls KeyScan after specific wait time and valid key press occurs
char PollKey() {
  char scannedKey;
  
  scannedKey = KeyInv;  // Invalid key by default
  
  // only run on specified update interval
  if ((millis() - lastButtonSend) > buttonSendDelay) {
  scannedKey = KeyScan();
    if (scannedKey != KeyInv) {
      // valid button was returned, reset the timer variable
      lastButtonSend = millis();
    }
  }
  return scannedKey;
}  // end of function PollKey


// this is the gatekeeper function. The only one to really touch the strings in program memory. This
// keeps the need for a buffer string to only one. So, basically we're only using 21 bytes
// of memory total to store strings.
void PrintLCD_P(int which) {
  //char buffer[21];
  char buffer[8];
  strcpy_P(buffer, (char*)pgm_read_word(&(StringTable[which])));
  //Serial.println(buffer);
  lcd.print(buffer);
  //delay(40);
}


//print a string found in program memory to the LCD at a certain location
//Basically this just sets the location and then calls PrintLCD_P to do the work
void PrintLCDAt_P(int which, char x, char y) {
  lcd.setCursor(x, y);
  PrintLCD_P(which);
}


//print a string to the LCD at a certain location
void PrintLCDAt(char text[8], char x, char y) {
  lcd.setCursor(x, y);
  lcd.print(text);  
}

