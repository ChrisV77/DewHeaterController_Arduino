// PC USB communications
#ifdef PC_CONTROL

bool checkPC() {
  static byte ndx = 0;
  char inChar;                      // single chars received
  bool newCMDReceived = false;      // has a full command been received.

  // look for a computer COMMAND - read until there is serial.available UNTIL end of CMD delimiter received
  while (( Serial.available() > 0) && (newCMDReceived == false) ) {
    inChar = Serial.read();
    if (inChar != endCMD) {
      receivedCOMChars[ndx] = inChar;
      ndx++;
    }
    else {
      // leave out the endCMD char & terminate the string
      receivedCOMChars[ndx] = '\0';
      ndx = 0;
      newCMDReceived = true;
    }
  }
  if (newCMDReceived) {
    newCMDReceived = extractCMDReceived();
  }
  return newCMDReceived;
}

bool extractCMDReceived() {
  // extract the command & check it is REAL
  String i;
  bool realCMDreceived = false;
  String      inCMDstring;                // a string of the command (CMD only, not the number) received from the computer

  // check there is a correct start char
  if (receivedCOMChars[0] == startCMD) {
    // extract 2 char command
    inCMDstring =   String(receivedCOMChars[1]);
    inCMDstring +=  String(receivedCOMChars[2]);
    // check the command is real: the commands are in the string array controllerCMDs[]
    for (int i = 0; i < numOfCMDs; i++) {
      if (inCMDstring == controllerCMDs[i]) {
        realCMDreceived = true;
        inCMD = i;                                      // set the inCMD array pointer to the command text
      }
    }
    // extract the command value number
    inCMDval = atoi(&receivedCOMChars[3]);
  }
  return realCMDreceived;
}

void sendArduinoCMDToPC (int CMDout, float CMDparam) {
  String outCMD;

  outCMD = startCMD;
  outCMD += controllerCMDs[CMDout];
  outCMD += String(CMDparam); 
  Serial.println(outCMD);
  delay(comDelay);
}

void sendNumChansToPC() {
  sendArduinoCMDToPC( 18, numChannels);                     // tc - total number of channels being used by arduino (so PC will only display those)
}

void sendAmbientDataToPC() {
  // send ambient data. NB: x10 all temperatures to get an integer in the command from temp to one decomal place
  sendArduinoCMDToPC( 1, (ambientTemperature));        // at - ambient temp
  sendArduinoCMDToPC( 2, ambientHumidity);                  // ah - ambient humidity
  sendArduinoCMDToPC( 3, (ambientDewpoint));           // ad - ambient dew point
}

void sendModeDataToPC() {
  // send the global mode params
  sendArduinoCMDToPC( 4, globalMode );                      // md - set channel mode  
  sendArduinoCMDToPC( 5, manualPower );                     // mn - set manual power
  sendArduinoCMDToPC( 6, (aHtrTargetTemp));             // mt - target temp (auto-heater mode)
  sendArduinoCMDToPC( 7, aAmbMaxPwr);                       // mx - max power (auto-ambient mode)
  sendArduinoCMDToPC( 9, (aAmbThresh));                  // mh - threshold (auto-ambient mode)
  sendArduinoCMDToPC( 8, (tempCutOff));                  // mc - temperature cut-off
}

void sendChannelDataToPC(int theChan) {
  // send the cycle params for the channel. NB: x10 all temperatures to get an integer
  sendArduinoCMDToPC( 11, theChan);                         // cn - active channel number
  sendArduinoCMDToPC( 12, chanCycleMode[theChan]);          // cm - cycle channel mode  
  sendArduinoCMDToPC( 13, (tempHeater[theChan]));       // ct - cycle heater temp  
  sendArduinoCMDToPC( 14, outputPower[theChan]);            // cp - cycle heater power
  sendArduinoCMDToPC( 15, (setPoint[theChan]));         // cs - cycle heater set point (auto-heater)
  sendArduinoCMDToPC( 16, gainPID[theChan]);                // pg - PID gain (auto-heater mode PID gain)
}

void tellPCcycleDone() {
  // send command to PC that cycle complete - the PC will save the cycle data to a file
  sendArduinoCMDToPC( 17, 0 );                              // cd - no param value passed to PC
}

void doCMDReceived() {
  switch (inCMD){
    case 0:     // "rd": open/close PC commPort rd1=close
      switch (inCMDval) {
        case 0:
          // rd0 - open commPort - talk to PC
          sendArduinoCMDToPC(0,0);            // commPort opened, acknowledge opened by sending rd0 to PC
          PCconnected = true;                 // bool = PC connected
          sendNumChansToPC();                 // tell PC how many arduino heater channels
          sendModeDataToPC();                 // & send mode data. Ambient & channel data sent later with each cycle
          sendArduinoCMDToPC(0,2);            // commPort opened, now sending rd2 to PC - tells it all commands sent are for starting setup
          break;
        case 1:
          // rd1 - close commPort - PC disconnected so shouldn't send commands (BUT IT STILL DOES!!!!!)
          PCconnected = false;                // bool = PC not connected
          sendArduinoCMDToPC(0,1);            // commPort closed, acknowledge opened by sending rd1 to PC
          break;
      }
      break;

    case 4:    // "md": set heater mode
      globalMode = inCMDval;
      break;

    case 5:    // "mn": set manual power
      manualPower = inCMDval;
      break;

    case 6:    // "mt": set temp target (relative to dew point)
      aHtrTargetTemp = inCMDval;             // divide by as as all temperature comms are integers
      break;

    case 7:    // "mx": set max power (for auto-ambient mode)
      aAmbMaxPwr = inCMDval;
      break;

    case 8:    // "mc": temp cut-off (for auto-heater mode)
      tempCutOff = inCMDval;                 // divide by as as all temperature comms are integers
      break;

    case 9:    // "mh": threshold (for auto-ambient mode)
      aAmbThresh = inCMDval;                 // divide by as as all temperature comms are integers
      break;

    case 10:    // "ms": save data to EEPROM
      saveEEPROMparams();                       // a param changed- so save all params to EEPROM
      break;
  }
}

#endif