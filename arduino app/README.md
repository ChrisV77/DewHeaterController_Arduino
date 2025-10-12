# Arduino Automatic Dew Heater Controller
This is an Arduino based automatic dew heater controller for telescopes. It *automatically* heats the optics on your telescope to stop the dreaded dew.

In this build, the power drive to the heater is automatically controlled by two modes
- *AUTO-heater mode*: automatic control driven by the difference between the ambient dew point and the current temperature of the heater on your lens/mirror (PID feedback control)
- *AUTO-ambient mode*: automatic control driven the difference between ambient temperature and dew point (not feedback control).
Auto-heater is much better but requires an extra sensor and wiring to the heater.

Heater power can also be set manually if desired, or turned off. These two modes also act as default modes in case of sensor read errors, ot heater exceeding a cutoff tempertaure. If these are all you want - this project is overkill.

The device can be controlled
- by a PC using the included windows app
- or by one button on the Arduino controller in conjunction with a display on the device
Both display current ambient and heater readings, plus the setting for the mode being used

At present an Arduino Uno or Nano can be used, with a:
- Display: either a 16x2 LCD, or a 128x64 OLED. The display is essential if using mode control with a button on the Arduino controller
- ambient temperature/humidity sensor: a BME280 or AM2320 (BME280 is better)
- heater temperature sensor: a DS18B20 in parasitic mode for each heater channel (supports up to 4x channels)
- MOSFET driver for each heater channel
- heater straps: you can use commercially available dew straps, or you can make your own with nichrome wire. I have included instructions for this. If using AUTO-heater mode then the heater strap will reqiuire an extra 2-wires for the temperature sensor.
- I power it all from a standard 12V battery used for telescopes.

All these can be attached to the Arduino through extension I/O boards. So not much soldering invovled:
- pure wiring if using Nano with a terminal block expansion board
- soldering required if using an Uno with a PCB protoboard.

The included files are:
- *Arduino code*: this folder contains the files to install on the Arduino
- *windowsapp*: this contains the windows app if running and display on a PC.
- *instructions_v6.pdf*: contains the rationale for the device, build options, assembly isntructions, and software installation & use.
- *Arduino_test_apps*: this folder contains Arduino programs for testing the sensors, displays, heater, and for setting up the PID control for auto-heater mode (if you wish to do this).
- *windows_test_apps*: this contains the windows apps for using the PIC control setup procedures.

IMPORTANT NOTES:
- if you use this version, 6.3d, with the Windows app 
  - download the latest version of the windows app <DewHeaterPCinterface> 
  - from github <http://github.com/ChrisV77/DewHeaterControllerArduino> (25-Feb-2025 or later)

 DEVICES USED:
  - Ambient temperature/humidity sensor: AM2320/BME280 on I2C
  - Heater strap temperature sensor: DS18B20
  - OPTIONAL Displays: OLED (SSD1306), LCD (16x2 16x4) but not working with new version yet
  - OPTIONAL Button control: on unit to control settings. Can't be used if PC control activae
  - OPTIONAL PC comms: to work with windows app DewHeaterPCinterface - displays everything & allows control of settings

UPDATES:

v6.3d (12-Oct-2025)
  - error checking when reading from EEPROM improved
  - sends PID gain for auto-heater mode to windows app for display (in extras menu)
  - Altered gainPID to float (from int) so can have gains <1, e.g. 0.5, 0.25 etc

v6.3c (2-Mar-2025)
  - changed to sending floating point numbers to PC. Needs new windows app if using PC_Control
    - This avoids x10 transformation of data to send as an int (e.g. temp=25.6 sent as 256) & increases resolution.

v6.3b (24-Feb-2025)
  - new set point calculation
  - working on:
    - variable aggression with PID = aggressive & conservative. Have a few levels depending on heater - set point temp, & allow each channel to set Kpid accordingly
    - have gain for PID on each channel = 1- 5x basal Kp,i,d values (for conservative value, & x4? this for aggressive). Have commented out line in serialPCinterface to send dat to PC
  - maybe = setting modes and parameters for individual channels

v6.3a (13-Feb-2025)
  - improved LCD & OLED displaying. 
  - added menus if using a display: change mode, change params, dipslay params. These all done separately. NB: changing mode doesn't set any params
  - changed auto modes
    - auto-ambient: auto-ambient threshold  = temperature below ambient to turn on heater (& max power when amivbent temp = dew point). I'm using between 5-8C
    - auto-heater: set target temperature relative to ambient. I'm using 2-3C
  - change: if in PC_CONTROL mode then can also have a display (but not mode button)

