/* 
 *  These functions:
 * - set up PID etc
 * - setHeaterData    = sets the power levels
 * - outputHeaterData = writes to output pins
 */

// set up heater output pins
void setupHeaterOutputPins() {
  // setup heater output pins
  for (int currChan=0; currChan < numChannels; currChan++) {
    pinMode(heaterPin[currChan], OUTPUT);                   // set the heater output pin as output
  }
}

void setUpPIDcontrol() {
  for (int currChan=0; currChan < numChannels; currChan++) {
    PID_control[currChan].SetMode(AUTOMATIC);
    PID_control[currChan].SetOutputLimits(0, 100);                // output limit 0-100%
    PID_control[currChan].SetTunings(baseKp*gainPID[currChan], baseKi*gainPID[currChan], baseKd*gainPID[currChan]);        // start with conservative K-params for PID
  }
}

// calc heater value - for mode 2 = auto-ambient
int calcOutputAutoAmbientMode(int theChan) {
  float aboveDewPoint;
  float aboveDewPointConstrain;
  int thePowerPercent;

  // calculate how far ambient temp is above dew point & constrain = aboveDewPoint
  // if aboveDewPoint < aAmbThresh heater turns on. Increases to max power when aboveDewPoint=0 (i.e. ambient temp = dew point)
  aboveDewPoint = ambientTemperature - ambientDewpoint;
  aboveDewPointConstrain = constrain( aboveDewPoint , 0 , aAmbThresh );   // constrain aboveDewPoint to values between 0 & aAmbThresh
  thePowerPercent = map((aboveDewPointConstrain*10), 0, (aAmbThresh*10), aAmbMaxPwr, 0);   // inverted map to power range (NB function uses int so temp*10 to get to 1 dp)
  return thePowerPercent;
}

// calc heater value - for mode 3 = auto-heater (PID control)
int calcOutputAutoHeaterModePID(int theChan) {
  // Calc setPoint temperature for PID. Set point is always
  // - above ambientTemp & more than heaterTargetTemp above dewPoint
  setPoint[theChan] = max( (ambientDewpoint + aHtrTargetTemp), ambientTemperature );
  
  Kp = baseKp*gainPID[theChan];
  Ki = baseKi*gainPID[theChan];
  Kd = baseKd*gainPID[theChan];
  PID_control[theChan].SetTunings(Kp, Ki, Kd);
  PID_control[theChan].Compute();
  return PIDout[theChan];
}

// set the heater output pin value
void setHeaterOutput(int theChan, int thePowerPercentLevel) {
  int thePWMlevel;

  // map the 0-100% power to 0-255 PWM level for digital pins
  thePWMlevel = map(thePowerPercentLevel, 0, 100, 0, 255);
  // write the heater's duty cycle to the heater
  analogWrite(heaterPin[theChan] , thePWMlevel);
}

void setHeaterMode(int theChan) {
  int theMode;

  // (1) change the channel mode from global mode if errors or above cut-off. Errors can be due to sensor malfunction or lost contact (electrics hate dew!)
  theMode = globalMode;
  switch (theMode) {
    case 3: // MODE 3: AUTO-heater
      if (errorDS18B20[theChan] ) theMode = 2;              // if error heater sensor read, change to mode to 2 (auto-ambient), then
      if (errorAmbientSensor) theMode = 1;                  // if error ambient read change mode to 1 (manual), then
      if (tempHeater[theChan] > tempCutOff) theMode = 0;    // if temp above cut-off, change to mode 0 (heater off)
      break;
    case 2: // MODE 2 : Auto-ambient
      if (errorAmbientSensor) theMode = 1;                  // if error ambient read change mode to 1 (manual)
      if (tempHeater[theChan] > tempCutOff) theMode = 0;    // if temp above cut-off, change to mode 0 (heater off)
      break;
    case 1: // MODE 1 : Manual
      if (tempHeater[theChan] > tempCutOff) theMode = 0;    // if temp above cut-off, change to mode 0 (heater off)
      break;
    // MODE 0: no checks
  }
  chanCycleMode[theChan] = theMode;                       // set channelMode: either globalMode or a defaulted mode if errors

  // (2) set the output power
  switch (theMode) {
    case 3: // MODE 3: AUTO Heater
      outputPower[theChan] = calcOutputAutoHeaterModePID(theChan);
      break;
    case 2: // MODE 2 : Auto DEWPOINT
      outputPower[theChan] = calcOutputAutoAmbientMode(theChan);
      break;
    case 1: // MODE 1: MANUAL
      outputPower[theChan] = manualPower;
      break;
    case 0: // MODE 0: Off
      outputPower[theChan] = 0;
      break;
  }
}

// OPTIONAL (use if have LEDS across heater output). Flash: x times for heaterMode (1,2,3,4 = off,man,aAmb,aHtr), then at heater power level
// - occurs at end of each cycle
void flashHeaterModePower() {
  int theMode;
  int theChan;
  int displayPowerLevel = 50;                                 // % power level for display

  // Output the mode - flash mode number of times: 1=off 2=manual 3=auto-amb 4=auto-htr
  for (theChan=0; theChan < numChannels; theChan++) setHeaterOutput(theChan,0);
  delay(displayDelay/2);
  for (theMode=0; theMode < numHeaterModes; theMode++) {
    for (theChan=0; theChan < numChannels; theChan++) {
      if (chanCycleMode[theChan] >= theMode) setHeaterOutput(theChan,displayPowerLevel);
    }
    delay(displayDelay/2);
    for (theChan=0; theChan < numChannels; theChan++) setHeaterOutput(theChan,0);
    delay(displayDelay/2);
  }
  // turn back on to current power
  for (int theChan=0; theChan < numChannels; theChan++) setHeaterOutput(theChan,outputPower[theChan]);
}

// OPTIONAL (use if have LEDS across heater output). Flash: x times for number of channels
// - occurs only at startup
void flashNumHeaterChannels(int numFlashes) {
  int displayPowerLevel = 50;                                     // % power level for display

  // flash all outputs at startup - number times = number channels
  for (int i=0; i < numFlashes; i++) {
    for (int currChan=0; currChan < numFlashes; currChan++) {   // flash ON
      outputPower[currChan] = displayPowerLevel;
      setHeaterOutput(currChan, outputPower[currChan]);
    }
    delay(displayDelay/2);
    for (int currChan=0; currChan < numFlashes; currChan++) {   // flash OFF
      outputPower[currChan] = 0;
      setHeaterOutput(currChan, outputPower[currChan]);
    }
    delay(displayDelay/2);                                        // flash interval
  }
}
