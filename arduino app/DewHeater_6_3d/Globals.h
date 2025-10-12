/*
 - All definitions, variables, constants etc for
 - pins, sensors, displays, feedback PID controller
*/
const int maxChannels = 4;                          // maximum number of possiblr channels

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// For the basic parameters that are stored in EEPROM; these are their constraints: [0]=min [1]=max [2]=default [3]=increment value

const int globalModeRange[maxChannels]      = {0,3,1,1};        // default 1=manual
const int manualPowerRange[maxChannels]     = {0,100,10,5};     // default 10%
const int aAmbMaxPwrRange[maxChannels]      = {0,100,30,10};    // default 30%
const int aAmbThreshRange[maxChannels]      = {0,12,6,1};       // default 6C
const int aHtrTargetTempRange[maxChannels]  = {0,10,3,1};       // default 3C
const int tempCutOffRange[maxChannels]      = {22,36,32,2};     // default 32C

// variable for use during running. I have set to the default values. But these will chnage with EEPROM read at startup
int   globalMode = globalModeRange[3];          // the global mode. Individual heater channels can default to other modes if heater temp & ambient read errors OR if above cutoff temp (only if heater connected)
                                                // modes: off (0,1), manual (1,2), auto-ambient (2,3), auto-heater (3,4). Values in brackets are those used for (code, display)
int   manualPower = manualPowerRange[3];        // manual power level: manual power between 0 - 100%
int   aAmbMaxPwr = aAmbMaxPwrRange[3];          // Auto-Ambient mode: max power level
int   aAmbThresh = aAmbThreshRange[3];          // Auto-Ambient mode: threshold below ambient temp to turn on heater (not feedback control)
int   aHtrTargetTemp = aHtrTargetTempRange[3];  // Auto-heater mode: target heater temperature above ambient (uses feedback control)
int   tempCutOff = tempCutOffRange[3];          // heater cut-off if too hot. Only works if have a DS18B20 temp sensor on heater

String      msgEEPROM;                          // message on opening EEPROM read - error check on values (check if in range of sensible values, see lines 34-39)

// THAT'S ALL
// ---------------------------------------------------------------------------------------------------------------------------------------------------

#define  DH_start   "DewHeater v"               // these are only used if an LCD/OLED display is being used
#define  DH_modes   "Modes: Man&Auto"

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// DIGITAL PINS & SENSORS

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Sensors for ambient temperature & humidity sensing on I2C
// ambient temp/humidity sensor: AM2320, BME280 (DHT21/22 not used anymore)
#include <Wire.h>   // needed for I2C comms

// the ambient sensor
 #ifdef AM2320_ON
  #include "Adafruit_Sensor.h"
  #include "Adafruit_AM2320.h"
  Adafruit_AM2320 am2320 = Adafruit_AM2320();
#endif
#ifdef BME280_ON
  #include <BME280I2C.h>
  BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                    // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
#endif

// and variables for ambient sensor
float     ambientTemperature;           // things to read
float     ambientHumidity;
float     ambientDewpoint;
float     ambientPressure;
bool      errorAmbientSensor;           // error, eg if not connected

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the displays if being used
// NB: if using this you'll need to run this and the ambint sensor on the I2C
// - LCD 16*2 or 16*4 20*4

// definitions for mode control
#ifdef MODEBUTTON
  #define  DH_control "Cntrl: Button   "
#endif
#ifdef PC_CONTROL
  #define  DH_control "Cntrl: PC       "
#endif

// definitions for LCD displays
#ifdef LCD1602
  #define LCD_ON
  #define numberCols 16
  #define numberRows 2
  #define  DH_display "Disp: LCD1602   "
#endif
#ifdef LCD1604
  #define LCD_ON
  #define numberCols 16
  #define numberRows 4
  #define  DH_display "Disp: LCD1604  "
#endif
#ifdef LCD2004
  #define LCD_ON
  #define numberCols 20
  #define numberRows 4
  #define  DH_display "Disp: LCD2004  "
#endif

#ifdef LCD_ON
  // LCD display 16x2, address usually 0x3F or 0x27
  #include <hd44780.h>                        // main hd44780 header, seems to be best library for LCD I2C
  #define DISPLAY_ON
  #include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c via expander backpack
  #ifdef LCD1602
    hd44780_I2Cexp lcd(0x27);
  #endif
  #ifdef LCD1604
    hd44780_I2Cexp lcd(0x3F);                 // I think this is correct??
  #endif
  #ifdef LCD2004
    hd44780_I2Cexp lcd(0x3F);
  #endif
  #define uCol 1                              // 1 units per column/character
  #define uRow 1                              // 1 units per row/character
#endif

// - definitions OLED 128*32 (SSD1306). Haven't sorted out SH1106
#ifdef OLED1306
  // OLED SSD1306 0.9"/1.3" display 128x32 displays usually on I2C 0x3C (sometimes 0x3D)
  // simpler library, uses much less memory
  #define DISPLAY_ON
  #define  DH_display "Disp:  OLED1306 "
  #include "SSD1306Ascii.h"
  #include "SSD1306AsciiAvrI2c.h"
  #define display_address 0x3c
  SSD1306AsciiAvrI2c display;
  #define uCol 6                   // 6 units per column/character (pixels)
  #define uRow 1                   // 1 unit per row/character
  #define numberCols 20
  #define numberRows 4
#endif

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Sensors for heater temperature: DS18B20s: temp sensors, uisng OneWire & DallasTemperature
// the DIGITAL PINS for sensor input, heater output & control.

#include <OneWire.h>
#include <DallasTemperature.h>

const int heaterPin[maxChannels]  = {9, 10, 5, 6};      // MOSFET heater output channels on PWM pins. 4 possible channels = pins 9, 10, 5, 6
OneWire DS18B20pin[maxChannels] = {7, 8, 4, 11};        // heater temp sensor pins. 2 channels = 7, 8, 4, 11

// include definitions for up to 4 possible channels
DallasTemperature sensor[maxChannels];      //  there are numChannels heater sensors

// DS18B20 variables
double      tempHeater[maxChannels];        // DS18B20 sensor temperatures from sensors1/2.
boolean     errorDS18B20[maxChannels];      // error if tempSensor == -90 or -127C (DEVICE_DISCONNECTED_C)
const int   resDS18B20 = 11;                // DS18B20 read value   = 9     10    11    12. I use 11 as nice resolution & delay not too much
                                            // resolution (C)       = 0.5   0.25  0.125 0.0625
                                            // read delay (ms)      = 93    187   375   750

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// digital PIN for MODE CONTROL: only if local MODEBUTTON control
#ifdef MODEBUTTON
  const int controlSwitchPin = 3;   // Mode control switch PIN, uses internal pull-up resistor  
#endif

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// PID controller - manually tuned PID for 50mm guidescope, gain = 
#include "PID_v1_bc.h"
double setPoint[maxChannels];                       // PID set point, dew point
double PIDout[maxChannels];                         // PID Output, NB: input is tempSensor

// PID tuning
const int baseKp = 10;                              // base Kp, Ki, Kd paraeters, multiply these by gainPID[] for each channel to set tuning of each channel
const int baseKi = 10;
const int baseKd = 0;
double Kp;    // K-values for used for PID control
double Ki;
double Kd;

PID PID_control[] = {
  PID(&tempHeater[0], &PIDout[0], &setPoint[0], Kp, Ki, Kp, DIRECT),
  PID(&tempHeater[1], &PIDout[1], &setPoint[1], Kp, Ki, Kp, DIRECT),
  PID(&tempHeater[2], &PIDout[2], &setPoint[2], Kp, Ki, Kp, DIRECT),
  PID(&tempHeater[3], &PIDout[3], &setPoint[3], Kp, Ki, Kp, DIRECT)
};

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// Variables int & char arrays: heaterMode, heaterParams (always). Also used as menus: mainMenu

// heaterModes - is also level2 mode menu: ID = 1
const int numHeaterModes = 4;
const char *heaterModes [] = {
  "Off  ", 
  "Man  ", 
  "A-Amb", 
  "A-Htr"
};
const char *heaterModesLong [] = {
  "Off         ", 
  "Manual      ", 
  "Auto-Ambient", 
  "Auto-Heater "
};

// heaterParams - is also for level2 param menu: ID = 2
const int numHeaterParams = 5;
const char *heaterParams [] = {
  "Man-Pwr     ", 
  "A-Amb-MaxPwr", 
  "A-Amb-Thresh",
  "A-Htr-Target", 
  "TempCutOff  ", 
};
const char *heaterParamsLong [] = {
  "Manual-Power       ", 
  "Auto-Ambient-MaxPwr", 
  "Auto-Ambient-Thresh",
  "Auto-Heater-Target ", 
  "Temp-CutOff        ", 
};

// --------------------------------------------------------------------------- 
// functions if display used

#ifdef MODEBUTTON
  // char array for level1 main menu: ID = 0
  const char *menuMain[] = {
    "Change Mode   ", 
    "Change Params ", 
    "Display Params"
  };
  const int numMenuMain = 3;
#endif

// channel cycle variables
int       outputPower[maxChannels];       // % power to heater: 0 - 100 %
int       chanCycleMode[maxChannels];     // the mode at each heater during current cycle. 
                                          //    - could be different to set mode (chanMode) if sensor read error OR in cut-off
                                          //    - this is just to tell the PC

// communication data variables
bool        PCconnected = false;                // is a PC connected
char        startCMD = ':';                     // start of command delimiter, ASCOMlike
char        endCMD = '#';                       // end of command delimiter, ASCOMlike 
const byte  numCOMChars = 16;                   // max number of characters in a command
char        receivedCOMChars[numCOMChars];      // an array to store the received data (command)
int         inCMD;                              // the array number of command received from computer
int         inCMDval;                           // the number associated with this command - NB only those starting with 'n' or 'c'
const int   numOfCMDs = 19;
int         activeChannel;                      // the current channel: used for channelParam variable
bool        cycleDone;                          // the current cycle is completed

const char  *controllerCMDs[numOfCMDs] =  {   "rd",                                       
                      //  PC comms:           0=connect(0), disconnect(1), end of startup sending arduino settings(2)
                                              "at",       "ah",         "ad",
                      //  ambient:            1=temp      2=humidity    3=dewpoint (values x10 to remove decimal point & make an int)
                                              "md",       "mn",         "mt",                 "mx",                 "mc",               "mh",               "ms",
                      //  mode:               4=mode      5=man pwr     6=target temp auto-H  7= max Pwr auto-A     8=temp cutoff       9=thresh auto-A     10=save on arduino     
                                              "cn",       "cm",         "ct",                 "cp",                 "cs",               "pg",               "cd",
                      //  chans (curr cycle)  11=number   12=mode       13=heater temp        14=cycle pwr          15=setpoint auto-H  16= PID gain        17=cycle done
                                              "tc"};
                      //                      18= number channels

const int   errorValue = -127;                   // value to send in error reading sensor (ambient / heaters)
                                                // use -90 or -127 (this is dallas temp error) ??

// timing stuff
bool        ambientChecked;                     // ambient sensor checked & read. This is the start of each cycle in the main loop (after checking PC)
bool        channelSet[maxChannels];            // heater channel set & sent to PC
const int   displayDelay = 500;                 // delays between displaying items 0.5s
const unsigned long   cycleDuration = 20000;    // 20 sec delay between cycles
                                                // A cycle = 1 PC check, 2 ambient sensor, 3 heater channels set
const int   comDelay = 50;                      // delay for PC coms - had 200ms, trying shorter - 150, 100, 50. %50 seems okay
const int   btnDelay = 25;                      // delay for button press
                                                // these are for loop timing
unsigned long         currentMillis;            // current time in millisecs
unsigned long         lastAmbientCheck;         // time of last ambient sensor check

// some compile error messages, just cause I really goof things up somethimes
#ifdef BME280_ON
  #ifdef AM2320_ON
    #error "Can't define both AM2320 & BME280 ambient sensors. Change the unused one to xxxxxx_OFF"
  #endif
#endif

#ifdef MODEBUTTON
  #ifdef PC_CONTROL
    #error "Can't have PC_CONTROL && MODEBUTTON"
  #endif
#endif

#ifdef MODEBUTTON
  #ifndef DISPLAY_ON
    #error "If using MODEBUTTON, must have a display defined"
  #endif
#endif

