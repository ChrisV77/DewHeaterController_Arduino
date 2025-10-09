/*
  Save to & write params from EEPROM to store between power cycles
  The format of saved EEPROM data is:
  Byte 0:   mode
  Byte 2:   manual power
  Byte 4:   temp target - auto-heater mode
  Byte 6:   temp Cut-Off
  Byte 8:   max power - auto-ambient mode
  Byte 10:  temp threshold - autoambient mode
*/

// read dew heater params from EEPROM
void getEEPROMparams() {
  unsigned int eepromAddress;   // counter for EEPROM address
  int numBytes = 2;             // 2 bytes per unsigned integer value stored on EEPROM for Arduino UNO
  int theChan;

  eepromAddress = 0;
  EEPROM.get( eepromAddress , globalMode );           // global mode of operation 1=off 2=manual 3=auto-ambient 4=auto-ambient+heater sensor. NB: channels can default when running
  eepromAddress += numBytes;
  EEPROM.get( eepromAddress , manualPower );          // manual power level
  eepromAddress += numBytes;
  EEPROM.get( eepromAddress , aAmbMaxPwr );           // max power (for auto-ambient mode)
  eepromAddress += numBytes;
  EEPROM.get( eepromAddress , aAmbThresh );           // temp threshold (for ambient mode)
  eepromAddress += numBytes;
  EEPROM.get( eepromAddress , aHtrTargetTemp );       // target heater temp - currently temp above ambient
  eepromAddress += numBytes;
  EEPROM.get( eepromAddress , tempCutOff );           // heater temp at which heater powers cuts off - to prevent over heating

  msgEEPROM = "EEPROM data OK";
  if (checkErrorEEPROMdata()) {
    saveEEPROMparams();
    msgEEPROM = "Err: rst EEPROM";
  }
}

// save dew heater params to EEPROM
void saveEEPROMparams() {
  unsigned int eepromAddress;  // counter for EEPROM address
  int numBytes = 2;            // 2 bytes per unsigned integer value stored on EEPROM for Arduino UNO

  //global parameters
  eepromAddress = 0;
  EEPROM.put( eepromAddress , globalMode );           // global mode of operation 1=off 2=manual 3=auto-ambient 4=auto-ambient+heater sensor. NB: channels can default when running
  eepromAddress += numBytes;
  EEPROM.put( eepromAddress , manualPower );          // manual power level
  eepromAddress += numBytes;
  EEPROM.put( eepromAddress , aAmbMaxPwr );           // max power (for auto-ambient mode)
  eepromAddress += numBytes;
  EEPROM.put( eepromAddress , aAmbThresh );           // temp threshold (for ambient mode)
  eepromAddress += numBytes;
  EEPROM.put( eepromAddress , aHtrTargetTemp );       // target heater temp - currently temp above ambient
  eepromAddress += numBytes;
  EEPROM.put( eepromAddress , tempCutOff );           // heater temp at which heater powers cuts off - to prevent over heating
}

bool checkParamError(int theVal, int minVal, int maxVal, int incrVal) {
  int validVal;
  bool errorVal;

  errorVal = true;
//  return ((val >= minval) && (val <= maxval));
  // check parameters in valid = in the range of allowed values & follows the increment, e.g. manualPower = 0 - 100% in increments of 10%
  for (validVal = minVal; validVal <= maxVal; validVal+=incrVal) {
    if (theVal == validVal) errorVal = false;
  }

  return errorVal;       // return true if error, false if no error
//  return ((val >= minval) && (val <= maxval));
}

bool checkErrorEEPROMdata() {
  bool errorParam = false;

  // check EEPROM value within allowed range & at their increment. If not set to default
  // values from arrays: 0=min 1=max 2=default 3=incr
  if (checkParamError( globalMode, globalModeRange[0], globalModeRange[1], globalModeRange[3]) ) {     
    globalMode = globalModeRange[2];
    errorParam = true;
  }
  if (checkParamError( manualPower, manualPowerRange[0], manualPowerRange[1], manualPowerRange[3]) ) {
    manualPower = manualPowerRange[2];
    errorParam = true;
  }
  if (checkParamError( aAmbMaxPwr, aAmbMaxPwrRange[0], aAmbMaxPwrRange[1], aAmbMaxPwrRange[3]) ) {     
    aAmbMaxPwr = aAmbMaxPwrRange[2];
    errorParam = true;
  }
  if (checkParamError( aAmbThresh, aAmbThreshRange[0], aAmbThreshRange[1], aAmbThreshRange[3]) ) {     
    aAmbThresh = aAmbThreshRange[2];
    errorParam = true;
  }
  if (checkParamError( aHtrTargetTemp, aHtrTargetTempRange[0], aHtrTargetTempRange[1], aHtrTargetTempRange[3]) ) {
    aHtrTargetTemp = aHtrTargetTempRange[2];
    errorParam = true;
  }
  if (checkParamError( tempCutOff, tempCutOffRange[0], tempCutOffRange[1], tempCutOffRange[3]) ) {     
    tempCutOff = tempCutOffRange[2];
    errorParam = true;
  }
  return errorParam;
}