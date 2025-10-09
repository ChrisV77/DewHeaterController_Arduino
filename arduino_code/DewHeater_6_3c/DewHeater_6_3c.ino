/*
 Dew Heater Controller
  - This controller provide automatic control of heater straps to warm scope optics
  - This automatic control can be provided by
    - feedback control by temperature sensors in the heater straps. Auto-heater MODE
    - simpler control by ambient temperature/humidity sensors. Auto-ambient MODE
  - The device can be controlled two means:
    - directly on the device via a single push-button and a display, or by
    - USB connection to a PC & the windows app <DewHeaterPCinterface>. A display on the dvice can also be included
 - see README.md (and the instructions) for more details

IMPORTANT NOTES:
- if you use this version, 6.3c, with the Windows app 
  - download the latest version of the windows app <DewHeaterPCinterface> 
  - from github <http://github.com/ChrisV77/DewHeaterControllerArduino> (25-Feb-2025 or later)

 Devices used:
  - Ambient temperature/humidity sensor: AM2320/BME280 on I2C
  - Heater strap temperature sensor: DS18B20
  - OPTIONAL Displays: OLED (SSD1306), LCD (16x2 16x4) but not working with new version yet
  - OPTIONAL Button control: on unit to control settings. Can't be used if PC control activae
  - OPTIONAL PC comms: to work with windows app DewHeaterPCinterface - displays everything & allows control of settings

v6.3a
  - improved LCD & OLED displaying. 
  - added menus if using a display: change mode, change params, dipslay params. These all done separately. NB: changing mode doesn't set any params
  - changed auto modes
    - auto-ambient: auto-ambient threshold  = temperature below ambient to turn on heater (& max power when amivbent temp = dew point). I'm using between 5-8C
    - auto-heater: set target temperature relative to ambient. I'm using 2-3C
  - change: if in PC_CONTROL mode then can also have a display (but not mode button)
v6.3b
  - new set point calculation
  - working on:
    - variable aggression with PID = aggressive & conservative. Have a few levels depending on heater - set point temp, & allow each channel to set Kpid accordingly
    - have gain for PID on each channel = 1- 5x basal Kp,i,d values (for conservative value, & x4? this for aggressive). Have commented out line in serialPCinterface to send dat to PC
  - maybe = setting modes and parameters for individual channels
v6.3c
  - changed to sending floating point numbers to PC. Needs new windows app if using PC_Control
    - This avoids x10 transformation of data to send as an int (e.g. temp=25.6 sent as 256) & increases resolution.

To do later (maybe):
  - add Rob Tillart AM2320 sensor library. But check if better.
  - WINDOWS APP - MAYBE use panels for mode settings (with grp boxes) to just blank out labels and values within each
*/

#define DH_version "6.3c"
#include "config.h"    // include the variables that need to be assigned by the user
#include "Globals.h"    // include the global variables, constants etc
#include "EEPROM.h"     // set up to use the EEPROM for parameter storage between power cycles

void setup(){
  Serial.begin(9600);                         // start serial port to communicate with PC
  getEEPROMparams();                          // read stored params from EEPROM
  #ifdef MODEBUTTON                           // setup mode control pin - only if MODEBUTTON enabled
    pinMode(controlSwitchPin, INPUT_PULLUP);  // internal pullup resistor, so LOW = active
  #endif

  // set up 
  setupHeaterOutputPins();    // heater pins
  resetAmbientSensor();       // sensors  - ambient: BME280,AM2320
  setUpHeaterSensors();       //          - & heaters DS18B20
  setUpPIDcontrol();          // PID controller params

  // set up timing, ambient sensor state & channel set state
  currentMillis = millis();
  lastAmbientCheck = currentMillis; 
  ambientChecked = false;                                   // start with ambient not checked yet
  for (int currChan=0; currChan < numChannels; currChan++) {
    channelSet[currChan] = false;                           // reset channel set state to false
  }

  // set up display - if any display enabled
  #ifdef DISPLAY_ON
    displaySetUp();
    displayVersion();
    menuL2ParamDisplay();
    displayClear();
  #endif 
  if (outputFlashMode) flashNumHeaterChannels(numChannels);    // flash output mode number of times, then power level
}

void loop() { 
  // Check for commands from PC or button press (depending on control mode)
  #ifdef PC_CONTROL
   if (checkPC()) doCMDReceived();
  #endif
  #ifdef MODEBUTTON
    if (checkButtonPress()) menuL1Main_Select();
  #endif

 currentMillis = millis();                                        // get current time
 if (!ambientChecked) {
    // do ambient check (if not already done & at start/end cycle)
    if ((currentMillis - lastAmbientCheck) > cycleDuration) {   
      // reset time & send mode data to PC
      ambientChecked = true;                                    // set ambientChecked to true
      lastAmbientCheck = currentMillis;                         // set time of last ambient sensor check
      // read ambient sensor & display or send to PC
      getAmbientData();
      #ifdef DISPLAY_ON
        displayAmbientData();                                   // display on LCD/OLED if connected
      #endif
      for (int currChan=0; currChan < numChannels; currChan++) {
        channelSet[currChan] = false;                           // reset channel set/read state to false
      }
    }
  }
  else {
    // Get heater info and display (if ambient check done & 1/2 way through cycle)
    if ((currentMillis - lastAmbientCheck) > (cycleDuration/2)) {
      ambientChecked = false;
      getDS18B20data();                                         // read all the DS18B20 heater sensors
      // go through channels and set the heaters
      for (int currChan=0; currChan < numChannels; currChan++) {
        if (!channelSet[currChan]) {
          setHeaterMode(currChan);                         // set heater output (depends on mode)
          setHeaterOutput(currChan, outputPower[currChan]);      // set heater output pins
          channelSet[currChan] = true;                          // the channel has been set
        }
      }
      // finished checking/setting all channels, reset cycle & tell PC if connected
      // channel heater info: if these device enabled, display & send to PC (if connected)
      #ifdef PC_CONTROL
        if (PCconnected) {                                      // if PC connected
          sendAmbientDataToPC();                                // send ambient data
          for (int currChan=0; currChan < numChannels; currChan++) {
            sendChannelDataToPC(currChan);                      // send channel data
          }
          tellPCcycleDone();                                    // tell PC cycle completed: PC will display in app and save cycle data
        }
      #endif
      #ifdef DISPLAY_ON
        for (int currChan=0; currChan < numChannels; currChan++) {
          displayChannelData(currChan);
        }
      #endif
      if (outputFlashMode) flashHeaterModePower();              // flash output mode number of times, then power level
    }
  }
}
