// functions: the basics that put things on specific displays
// - OLED v LCD & 2 v 4 row displays
// LCD display: 16*2, 16*4, 20*4 - using HD44780: col,row = 1,1 per char
// 128 x 32 OLED display (128*32) - SSD1306: col,row = 6,8 pixels per char; so ~20*8 chars
// ---------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef DISPLAY_ON

void displaySetUp(){
  // initialize display
  #ifdef OLED1306   // for OLED
    display.begin(&Adafruit128x32,display_address);
    display.setFont(Adafruit5x7);     // numver pixels per row/col for font
  #endif
  #ifdef LCD_ON     // for LCD display
    lcd.begin(numberCols,numberRows);
    lcd.backlight();
  #endif
}

void displayClear() {
  #ifdef OLED1306   // for OLED
    display.clear();  
    display.setCursor(0,0);
  #endif
  #ifdef LCD_ON     // for LCD display
    lcd.clear();  
    lcd.setCursor(0,0);
  #endif
}


void setDisplayPosition(int displayCol, int displayRow) {
  // set display row: only need to alter if 2 row display (4 row display no change)
  if (numberRows == 2) {
    // row 0 (1st row) - nothing to do; row 1 (2nd row) - add delay after displaying
    if (displayRow == 2) displayRow = 0;
    if (displayRow == 3) displayRow = 1;
  }
  // set cursor position - LCD v OLED drivers
  if ((displayCol == 0) && (displayRow == 0)) displayClear();
  #ifdef OLED1306   // for OLED
    if ((displayCol >= 0) && (displayRow >= 0))   // if set to -1,-1 then doesn't change cursor position
      display.setCursor(displayCol*uCol,displayRow*uRow);
  #endif
  #ifdef LCD_ON     // for LCD display
    if ((displayCol >= 0) && (displayRow >= 0)) 
      lcd.setCursor(displayCol*uCol,displayRow*uRow);
  #endif
}

void setDisplayText(String theText, int invertText) {
  #ifdef OLED1306   // for OLED
    display.setInvertMode(invertText);          // invert if needed
    display.print(theText);
    display.setInvertMode(0);                   // return to text colour if inverted
  #endif
  #ifdef LCD_ON     // for LCD display
    lcd.print(theText);                         // LCD doesn't have invert mode
  #endif
}

void setDisplayNumber(int theNumber, int invertText) {
  #ifdef OLED1306   // for OLED
    display.setInvertMode(invertText);          // invert if needed
    display.print(theNumber);
    display.setInvertMode(0);                   // return to text colour if inverted
  #endif
  #ifdef LCD_ON     // for LCD display
    lcd.print(theNumber);                         // LCD doesn't have invert mode
  #endif
}

// this adds a delay to retain current display. Can be used
// - as dependent on row number 1/3 (for 2/4-row displays), or force delay -1
// - delay time is multiples of displayDelay = 0.5s
void addDisplayDelay(int theRow, int delayMultiple) {
  if (theRow == 1 && numberRows == 2) {     // row 1 (2nd) for 2-row display
    delay(displayDelay*delayMultiple);
  }
  if (theRow == 3) {                        // row 3 (4th) for 2/4-row displays, so al display types
    delay(displayDelay*delayMultiple);
  }

  if (theRow < 0) {                         // force delay (for any row)
    delay(displayDelay*delayMultiple);
  }
}

// these functions display text or an int (number). They call
// - setDisplayPosition: set the row/col to display, then
// - setDisplayText(or NUmber): displays the text or number
void displayText(String theText, int theCol, int theRow, int invertText) {
  // inputs: text to display , @column, @row, normal/invert text = 0/1
  setDisplayPosition(theCol, theRow);           // set position for display
  setDisplayText(theText, invertText);          // display the text
}

void displayNumber(int theNumber, int theCol, int theRow, int invertText) {
  // inputs: text to display , @column, @row, normal/invert text = 0/1
  setDisplayPosition(theCol, theRow);           // set position for display
  setDisplayNumber(theNumber, invertText);      // display the text
}

#endif