// function to display various thinngs
// these don't take into account different display types, that is dealt with in display_Basics
// ---------------------------------------------------------------------------------------------------------------------------------------------------
#ifdef DISPLAY_ON

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// display ambient temp, humidity, dew point

void displayAmbientData() {
  // displayText("Ambient",0,0,0);
  displayText("Temp",0,0,0);
  displayText("Humid",5,0,0);
  displayText("DewPt",11,0,0);
  
  if (!errorAmbientSensor) {
    // if ambient read okay display AM2320/BME data
    displayNumber(ambientTemperature,0,1,0);
    displayText("C",-1,-1,0);
    displayNumber(ambientHumidity,5,1,0);
    displayText("%",-1,-1,0);
    displayNumber(ambientDewpoint,11,1,0);
    displayText("C",-1,-1,0);
  }
  else {
    // error reading ambient sensor
    displayText("--",0,1,0);
    displayText("--",5,1,0);
    displayText("--",11,1,0);
  }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// display heater channel data

void displayChannelData(int currChan) {
  int aboveDewPoint;  // convert temp to integer for display

  // display the heater number
  displayNumber((currChan+1),0,currChan,0);

  // display mode dependent parameter
  switch (chanCycleMode[currChan]) {
    case 0:     // OFF mode: diplay --
      displayText("--",2,currChan,0);
      break;
    case 1:     // Manual mode: display --
      displayText("--",2,currChan,0);
      break;
    case 2:     // auto-ambient mode: display ambient temp - dew point
      aboveDewPoint = int(ambientTemperature - ambientDewpoint);
      if (aboveDewPoint > 0)  displayText("+",2,currChan,0);
      if (aboveDewPoint == 0) displayText("",2,currChan,0);
      if (aboveDewPoint < 0)  displayText("-",2,currChan,0);
      displayNumber(abs(aboveDewPoint),-1,-1,0);
      displayText("C",-1,-1,0);
      break;
    case 3:     // auto-heater mode: display heater temp
      displayNumber(tempHeater[currChan],2,currChan,0);
      displayText("C",-1,-1,0);
      break;
  }

  // display the % power (duty cycle)
  displayNumber(outputPower[currChan],6,currChan,0);
  displayText("%",-1,-1,0);

  // display the heater mode 
  // If an OLED display reverse the colour if has defaulted from auto-htr to auto-amb, manual or OFF (i.e. != globalMode)
  if (globalMode == chanCycleMode[currChan]) {
    displayText(heaterModes[chanCycleMode[currChan]],11,currChan,0);
  }
  else {
    displayText(heaterModes[chanCycleMode[currChan]],11,currChan,1);
  }
  // delay to hold current view: if line1 (2nd) for 2-row display; line3 (4th) for all displays
  addDisplayDelay(currChan, 2);
}  

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// startup display

void displayVersion(){
  displayText(msgEEPROM,0,0,0);
  addDisplayDelay(-1,2);
  displayText(DH_start,0,0,0);
  displayText(DH_version,11,0,0);
  displayText(DH_modes,0,1,0);
  addDisplayDelay(1,2);
  displayText(DH_control,0,2,0);
  displayText(DH_display,0,3,0);
  addDisplayDelay(3,2);
  displayClear();
}

#endif