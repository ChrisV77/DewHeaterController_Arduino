/* ---------------------------------------------------------------------------------------------------------------------------------------------------
THESE ARE THE ESSENTIAL THINGS YOU NEED TO SETUP
- they are the basics: number of heater channels, control type & display
*/

// NUMBER OF HEATER CHANNELS. 
// OPTIONS: from 1 - 4 channels/heaters
const int   numChannels = 2;

// AMBIENT TEMP/HUMIDITY SENSOR: AM2320 or BME280
// options: AM2320_ON, BME280_ON. NB: I2C pins on Nano: SDA = A4, SCL = A5
#define AM2320_ON

// MODE CONTROL: options = PC_CONTROL, MODEBUTTON
//  - PC_CONTROL: remote via PC (Controller = DewHeaterPCinterface.exe). Can also define a DISPLAY TYPE (but can't have mode button)
//  - MODEBUTTON: LOCAL with button. if you use this, you must define a DISPLAY TYPE line 30
#define PC_CONTROL
                                        
// DISPLAYS: OLED (OLED1306=SSD1306 display; using ssd1306ascii driver), LCD (LCD1602=16*2 LCD1604=16*4 LCD2004=20*4).
// options: OLED1306, LCD1602, DISPLAY_OFF (these 3 are the most common). LCD1604, LCD2004, OR DISPLAY_OFF are other options
// - see comments lines 16-21
#define DISPLAY_OFF

/* ---------------------------------------------------------------------------------------------------------------------------------------------------
These are some more params than can be altered
- if you are not sure, just leave them as is
*/
const bool  blankHeaterDuringRead = true;       // options: true, false: blank heater during heater temperature read
                                                // - Use it if getting strange values. Heater PWM output can interfere with DS18B20 readings if wires are long and close to each other
const bool  dewPointComplexCalc = true;         // options: complex (true) or simple (false) dew point calculation. Default = true (doesn't really add much processing time)
const bool  outputFlashMode = true;             // options: true/false (use if have LEDs across heater output). Will flash
                                                // - On startup: number of channels
                                                // - End of each cycle: heater mode (1,2,3,4 = off, manual, auto-ambient, autoheater), then at the heater power level

/* ---------------------------------------------------------------------------------------------------------------------------------------------------
These are gain factors for PID tuning
- if you are not sure, set them all to 2. You have to set all 4 values
*/
// For my current setup, heater Ch1 = 50/30mm guidescope, Ch2 = 80mm refractor; these are gainPID[0] & [1]
int gainPID[] {2, 1, 1, 1};                     // gain factor for PID Kp/i/d for channels 1-4 (using base Kp, Ki, Kd values of 10, 10, 0)
                                                // sensible values are 0.5, 1, 2, 4, 6, 8 (& maybe lower/higher).
                                                