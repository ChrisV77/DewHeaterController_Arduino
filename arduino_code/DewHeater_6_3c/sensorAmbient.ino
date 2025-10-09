// get ambient temp & humidity data from AM2320 or BME280 sensor

#ifdef AM2320_ON
void resetAmbientSensor() {
  am2320.begin();
}

void getAmbientData() {
/*
  // use the (1) standard or (2) modified adafruit am2320 sensor library. Will later look at Tilaart library

  // (1) standard adafruit library. NB: this doesn't handle read errors, so dealt with in calDewPoint()
  ambientHumidity = am2320.readHumidity();
  ambientTemperature = am2320.readTemperature();
*/
  // (2) modified adafruit library: use if having AM2320 read problems. NB: have to modify installed adarfruit AM2320 library (see instructions)
  if (am2320.readTemperatureAndHumidity(&ambientTemperature, &ambientHumidity)) errorAmbientSensor = false;
  else   errorAmbientSensor = true;

  // then calculate dew point, this has error handling
  calcDewPoint();
}
#endif

#ifdef BME280_ON
void resetAmbientSensor() {
  // Start up the bme280. address is 0x76
  bme.begin();
}

void getAmbientData() {
  bool bmeStatus;

  errorAmbientSensor = true;
  // read ambient temp and humidity - takes 100s of msec?.
  bmeStatus = bme.begin();
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);    // units for read
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  delay(comDelay);      // short delay for read - not sure if needed
  if (bmeStatus) {
    // sensor read OK
    errorAmbientSensor = false;
    bme.read(ambientPressure, ambientTemperature, ambientHumidity, tempUnit, pressUnit);
  }
  else {
    // error reading sensor
    errorAmbientSensor = true;
  }

  // then calculate dew point, this has error handling
  calcDewPoint();
}
#endif

// calculation doesn't do any reading, so is common to all sensors
void calcDewPoint() {
  float logEx;

  if ( errorAmbientSensor || isnan(ambientTemperature) || isnan(ambientHumidity) ) {
    // sensor read error
    ambientHumidity     = errorValue;     // set both to read error value
    ambientTemperature  = errorValue;
    ambientDewpoint     = errorValue;
  }
  else {
    // Calc dew point if no sensor read error: complex or simple calculation (use complex = better)
    if (dewPointComplexCalc) {
      logEx = 0.66077 + 7.5*ambientTemperature/(237.3+ambientTemperature) + (log10(ambientHumidity) - 2);
      ambientDewpoint = (logEx - 0.66077)*237.3/(0.66077+7.5-logEx);    
    }
    else {
      ambientDewpoint = ambientTemperature - ((100 - ambientHumidity)/5.0);
    }
  }
}
