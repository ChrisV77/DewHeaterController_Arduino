// heater temp data from DS18B20 sensors

void setUpHeaterSensors() {
  DeviceAddress deviceAddress;
  for (int theChan = 0; theChan < numChannels; theChan++) {
    sensor[theChan].setOneWire(&DS18B20pin[theChan]);
    sensor[theChan].begin();
    if (sensor[theChan].getAddress(deviceAddress, 0)) {
      sensor[theChan].setResolution(deviceAddress, resDS18B20);
    }
  }
}

void getDS18B20data() {
  int theChan;

  // first - setup and read the sensors
  for (theChan=0; theChan < numChannels; theChan++) {
    // zero the PWM output to heaters while talking to DS18B20s. Do this as PWM can cause interference (DS18B20 & heater wires close)
    if (blankHeaterDuringRead) setHeaterOutput(theChan, 0);
  
    // restart the library if last pass of DS18B20 had a read error
    if (errorDS18B20[theChan]) {
      // have deleted resetting. maybe need to add in?
    }
    sensor[theChan].requestTemperatures();    // request temperature
  }
  // need a delay to get the temperature request (93, 187, 375 & 750ms for res = 9, 10, 11 & 12)
  delay(750/ (1 << (12-resDS18B20)));         // a single delay after all the channel requests

  // then - get the converted temperatures
  for (theChan=0; theChan < numChannels; theChan++) {
    tempHeater[theChan] = sensor[theChan].getTempCByIndex(0);   // get temperature in celsius

    // if using blank heater, turn the PWMs back on  finished reading DS18B20
    if (blankHeaterDuringRead) setHeaterOutput(theChan, outputPower[theChan]);

    // save error status
    errorDS18B20[theChan] = false;            // no read error
    if ( (tempHeater[theChan] == DEVICE_DISCONNECTED_C) || (tempHeater[theChan] == -127) ) {
      // if read error, i.e. disconnected
      errorDS18B20[theChan] = true;             // read error
      tempHeater[theChan] = errorValue;         // read error value
    }
  }
}
