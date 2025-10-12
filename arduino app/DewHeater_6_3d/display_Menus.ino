// menus to change mode/parameters, and display parameters
// only need DISPLAY_ON for parameter displaying
// also need BUTTON_ON for changing modes/parameters
// -------------------------------------------------------------------------------
#ifdef DISPLAY_ON
// this is used if there is a display, even if there is no MODEBUTTON
void menuL2ParamDisplay() {
  int theParam;
  String theString;

  displayText ("Settings:",0,0,0);
  addDisplayDelay(-1,3);

  // display the mode
  displayText ("Mode:",0,0,0);
  displayText (heaterModes[globalMode],0,1,0);
  addDisplayDelay(-1,3);

  // & now all their settings
  for (theParam = 0; theParam < numHeaterParams;theParam++) {
    theString = heaterParams[theParam];
    theString.replace(" ","");
    displayText(theString,0,0,0);     // auto-ambient max power
    displayText (":",-1,-1,0);
    if (theParam == 0) {
      displayNumber(manualPower,0,1,0);
      displayText("%",-1,-1,0);
    }
    if (theParam == 1) {
      displayNumber(aAmbMaxPwr,0,1,0);
      displayText("%",-1,-1,0);
    }
    if (theParam == 2) {
      if (aAmbThresh > 0)  displayText("+",0,1,0);
      if (aAmbThresh == 0) displayText("",0,1,0);
      if (aAmbThresh < 0)  displayText("-",0,1,0);
      displayNumber(abs(aAmbThresh),-1,-1,0);
      displayText("C",-1,-1,0);
    }
    if (theParam == 3) {
      if (aHtrTargetTemp > 0)  displayText("+",0,1,0);
      if (aHtrTargetTemp == 0) displayText("",0,1,0);
      if (aHtrTargetTemp < 0)  displayText("-",0,1,0);
      displayNumber(abs(aHtrTargetTemp),-1,-1,0);
      displayText("C",-1,-1,0);
    }
    if (theParam == 4) {
      displayNumber(tempCutOff,0,1,0);
      displayText("C",-1,-1,0);
    }
    addDisplayDelay(-1,3);
  }
}
#endif

#ifdef MODEBUTTON
#ifdef DISPLAY_ON
void menuL1Main_Select() {
  int menuItemNum;

  // level1 menuID=0: select level2 menu item
  displayText("Main Menu",0,0,0);
  menuItemNum = selectMenuItem(0);    // ID = 0 for level1 main menu
  addDisplayDelay(-1,3);

  // go to level2 menus
  switch (menuItemNum) {
    case 0:   // level2 menu (ID: 1): change mode
      menuL2Mode_Select();
      break;
    case 1:   // level2 menu (ID: 2): change params
      menuL2ParamChange();
      break;
    case 2:   // level2 menu (ID: 3): display params
      menuL2ParamDisplay();
      break;
  }
}

// menu: change mode - level2 ID=2
void menuL2Mode_Select() {
  int menuItemNum;
  int newGlobalMode;

  displayText ("Change Mode",0,0,0);
  newGlobalMode = selectMenuItem(1);    // ID = 1 for level2 mode menu
  displayText ("selected",8,1,0);
  if (newGlobalMode != globalMode) {
    globalMode = newGlobalMode;         // if changed, set new param value
    saveEEPROMparams();                 // & save to EEPROM
  }
  addDisplayDelay(-1,2);
}

void menuL2ParamChange() {
  int menuItemNum;
  int paramMin;
  int paramMax;
  int paramDefault;
  int paramCurrent;

  // select the param to change
  displayText ("Change Param:",0,0,0);
  menuItemNum = selectMenuItem(2);    // ID = 2 for level2 change param
  addDisplayDelay(-1,2);

  // change the selected param
  setParamValue(menuItemNum);
}

int selectMenuItem(int menuID) {
  int itemNum;
  int menuSize;

  // use menu ID to get menu size, only menuID 0-2 not 3
  if (menuID == 0) menuSize = numMenuMain;                        // ID 0: main menu
  if (menuID == 1) menuSize = numHeaterModes;                     // ID 1: change mode menu
  if (menuID == 2) menuSize = numHeaterParams;                    // ID 2: change param menu
                                                                  // ID 3: display params = NO MENU
  // incr item number - press button to select. Only meduIDs 0-2, not 3
  if (menuID < 3) {
    itemNum = 0;
    do {
      displayText("                ",0,1,0);
      if (itemNum == menuSize) itemNum = 0;                       // reset item number
      if (menuID == 0) displayText(menuMain[itemNum],0,1,0);      // display item in menu1Main, if ID = 0
      if (menuID == 1) displayText(heaterModes[itemNum],0,1,0);   // display item in menu2Mode, if ID = 1
      if (menuID == 2) displayText(heaterParams[itemNum],0,1,0);  // display item in menu2Mode, if ID = 2
      itemNum += 1;                                               // incr item number
      addDisplayDelay(-1,2);
    } while (checkButtonPress() == false);                        // done when select button pressed (LOW = pressed)
  }
  return (itemNum-1);
}

void setParamValue(int theParam) {
  int valCurrent;
  int valMin;
  int valMax;
  int valIncr;
  int theVal;
  String theUnit;
  String theString;

  // set the current value, max & min
  switch (theParam) {
    case 0:
      valCurrent = manualPower;
      valMin = manualPowerRange[0];
      valMax = manualPowerRange[1];
      valIncr = manualPowerRange[3];
      theUnit = "%";
      break;
    case 1:
      valCurrent = aAmbMaxPwr;
      valMin = aAmbMaxPwrRange[0];
      valMax = aAmbMaxPwrRange[1];
      valIncr = aAmbMaxPwrRange[3];
      theUnit = "%";
      break;
    case 2:
      valCurrent = aAmbThresh;
      valMin = aAmbThreshRange[0];
      valMax = aAmbThreshRange[1];
      valIncr = aAmbThreshRange[3];
      theUnit = "C";
      break;
    case 3:
      valCurrent = aHtrTargetTemp;
      valMin = aHtrTargetTempRange[0];
      valMax = aHtrTargetTempRange[1];
      valIncr = aHtrTargetTempRange[3];
      theUnit = "C";
      break;
    case 4:
      valCurrent = tempCutOff;
      valMin = tempCutOffRange[0];
      valMax = tempCutOffRange[1];
      valIncr = tempCutOffRange[3];
      theUnit = "C";
      break;
  }

  // display starting stuff
  theString = heaterParams[theParam];   // the parameter
  theString.replace(" ","");
  theString += ":";
  displayText(theString,0,0,0);
  displayText ("(",6,1,0);              // the current value in "()" with units
  displayNumber(valCurrent,-1,-1,0);
  displayText (theUnit,-1,-1,0);
  displayText (")",-1,-1,0);

  // cycle through values min to max repeatedly
  theVal = valMin;
  do {
    displayText("      ",0,1,0);                               // clear - something weird happens on rotation through range
    if (theVal < valMin) theVal = valMin;                       // reset item number
    if (theVal > valMax) theVal = valMin;                       // reset item number
    displayNumber(theVal,0,1,0);                                // display the incrementing value
    theVal += valIncr;                                          // incr item number
    addDisplayDelay(-1,2);
  } while (checkButtonPress() == false);                        // done when select button pressed (LOW = pressed)
  
  // finished cycle. Set to preceeding incr value - goes in cycle (min to max)
  if (theVal == valMin) theVal = valMax;
  else theVal -=valIncr;
  displayNumber(theVal,0,1,0);                                // display the incrementing value
  displayText(theUnit,-1,-1,0);
  displayText("selected",6,1,0);
  addDisplayDelay(-1,3);

  // set the new param value & save to EEPROM. Maybe just do this if param changed
  if (theVal != valCurrent) {
    // parameter changed
    if (theParam == 0) manualPower    = theVal;
    if (theParam == 1) aAmbMaxPwr     = theVal;
    if (theParam == 2) aAmbThresh     = theVal;
    if (theParam == 3) aHtrTargetTemp = theVal;
    if (theParam == 4) tempCutOff     = theVal;
    saveEEPROMparams();
    displayText ("saved    ",6,1,0);
  }
  else {
    // parameter unchanged
    displayText ("no change",6,1,0);
  }
  addDisplayDelay(-1,2);
}

boolean checkButtonPress() {
  boolean buttonPressed;

  buttonPressed = false;
  // if button pressed, set buttonPressed true
  if (digitalRead(controlSwitchPin) == LOW) {
    buttonPressed = true;
    delay(btnDelay);
  }
  // if button pressed, wait for button release
  if ( buttonPressed == true) while (digitalRead(controlSwitchPin) == LOW) delay(btnDelay);
  return buttonPressed;
}

#endif
#endif