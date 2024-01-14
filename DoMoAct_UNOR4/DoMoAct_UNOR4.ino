/*
  Written by Paulvha January 2024
  Released under MIT licence.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ===========//================//===========//================//=============//=============

  DoMoAct is a system to can switch a GPIO's high/low by comparing the requested switch time
  against the current time. The current time is taken from the RTC. The RTC is initialized from
  an NTP server at start-up and synchronized every night after.
  The switch time can be set as a hard/fixed time or based on the daily sunset / sunrise time on your location.
  The sunset and sunrise times are calculated by DoMoAct

  You can also add sensors to switch a GPIO. One could measure the temperature and if too high, switch
  on a blower, or measure the waterlevel and switch a pump if level is too high or too low.
  As an example there is a temperature sensor DS1820 that can be enabled. (Sensor_example)

  Detailed information and how to get started is in the included DoMoAct-manual.odt. I strongly advice
  to read this document as it contains a ton of information about customization and configuring before compile.

  HARDWARE
  The processor board is the Arduino UNO-R4 WiFi. A board with relays can be connected to the GPIO's.
  Be aware that the UNO-R4 GPIO's can only handle 8mA / 5V.
  See the hardware section in the DoMoAct-manual.odt for more important information.

  todo : web SSL security-- whenever Arduino creates it for UNO-R4

  BEFORE POSTING:
  Clear EEPROM structure       V
  check manual                 V
  update version               V
  remove SSID and PASS         V
  disble username and password V
  disable debug messages       V
  disable example              V
*/

#include "WiFiS3.h"
#include "src/TimeLord.h"       // included in the src folder of sketch already:  https://github.com/probonopd/TimeLord for sunset/sunrise
#include "DoMoAct.h"
#include "RTC.h"
#include <EEPROM.h>

#if ENABLEMATRIX == 1
#include "ArduinoGraphics.h"    // Click here to get the library: http://librarymanager/All#ArduinoGraphics
#include "Arduino_LED_Matrix.h"
#endif

/////// please enter your sensitive data in DoMoAct.h ///////////////////
char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;      // your network password (use for WPA, or use as key for WEP)

char Acc_Name[] = DoMoAct_NAME; // Option to protect access to DoMoAct
char Acc_Pswd[] = DoMoAct_PASS;

/////////////// global variables ////////////
// Set at 04.00AM by Alarm to trigger updatetime, Sunset / sunrise & perform an EEPROM backup
volatile bool TriggerUpdateTime = false;

int  sunrise = 0;               // holds sunrise time
int  sunset = 0;                // holds sunset time
int  CurTime = 0;               // holds current time
int  led = LED_BUILTIN;         // led to provide some progress info
bool TimeDebug = false;         // enable debug message for time function (menu controlled)
bool WebDebug = false;           // enable debug message for Web function (menu controlled)
bool FastCheckEnable = false;   // is false when update has been made to EEPROM
bool AskLogin = false;          // true : DoMoAct needs login of username & password are set

MyObject CF_DoMoAct = {0};      // Parameter structure stored in EEPROM

////////////// instances ////////////////////
WiFiServer server(80);
WiFiClient client;
WiFiUDP Udp;                    // A UDP instance to let us send and receive NTP packets over UDP
TimeLord tardis;                // instance to calculate sunset & sunrise

#ifdef ENABLEMATRIX == 1        // include UNO-R4 WiFi Led matrix
ArduinoLEDMatrix matrix;
#endif

void setup() {

  Serial.begin(115200);         // initialize serial communication
  while (! Serial);

  init_matrix();

  if (ssid[0] == '*' || pass[0] == '*') {
    Serial.println("ERROR : The network SSID has not been set. Freeze !");
    Serial.println("In tab DoMoAct.h make sure to define SECRET_SSID and SECRET_PASS first.");
    Serial.println("Also update longitude, latitude and TZ and daylight saving information");

    MessageMatrix("E1",M_NO);
    while(1);
  }

  if (Acc_Name[0] != '*' && Acc_Pswd[0] != '*') // DoMoAct menu login required ?
      AskLogin = true;

  pinMode(led, OUTPUT);        // set the LED pin mode
  digitalWrite(led,LOW);

  MessageMatrix("S1",M_NO);

  ConnectToWifi();

  MessageMatrix("S2",M_NO);

  digitalWrite(led,HIGH);     // Wifi is connected

  RTC.begin();

  if (UpdateTime()){          // try to update / set time first time
    InitSunset();             // initialize TimeLord parameters for daily sunset / sunrise
    UpdateSunTimes();
  }
  else {
    Serial.println("FATAL ERROR: Can not obtain time from the NTP server. Freeze !");
    MessageMatrix("E3",M_NO);

    while(1) {
      delay(1000);
      digitalWrite(led,LOW);
      delay(1000);
      digitalWrite(led,HIGH);
    }
  }

  MessageMatrix("S3",M_NO);

  CF_ReadEeprom(false);      // restore parameters (do not display)

  InitSensors();             // Initialize Sensor variables and any sensor needed

  MessageMatrix("S4",M_NO);

  StartServer();             // Start server

  printWifiStatus();         // you're connected now, so print out the status

  MessageMatrix("OK",M_UP);

  int l = 10;
  while(l--) {
    delay(100);
    digitalWrite(led,HIGH);
    delay(100);
    digitalWrite(led,LOW);
  }
}

void loop()
{
  // Alarm was set, time to update
  if (TriggerUpdateTime) {
    UpdateTime();                     // Try to connect to NTP and update RTC
    UpdateSunTimes();                 // calculate today's Sunset and sunrise
    CF_WriteBackUpToEEprom(false);    // Parameters to backup location if changed
    TriggerUpdateTime = false;        // reset need to update
  }

  DoSwitchOnOff();                    // check whether a port / group / sensor needs a switch

  CheckForClient();                   // check for Web Client

  if (Serial.available()){            // check to open Serial menu after pressing <enter>
    StartMenu();
    while (Serial.available()) Serial.read(); // clear out any pending characters
  }
}

//////////////////////////// sensor functions ///////////////////////////////////////////
/**
 * This function is called from setup() to initialize the Sensor variables
 * From here call the routines to initialize different sensors (like InitDallas example)
 */
void InitSensors()
{
  for (byte j = 0; j < CF_NUMSENSORS; j++) {
    ChangeSensorStatus[j] = SENSOR_NOACT;
    SensorStatus[j] = false;
  }

  // add sensors to initialize here
#if SENSOREXAMPLE == 1
  InitDallas(); // example
#endif
}

/**
 * This routine is called to regular trigger / check on sensors if needed
 * like if this was the loop() of the original sensor sketch
 * called from CheckSwitchSensors()
 */
void CheckSensors()
{
  // add sensors that need a regular check
#if SENSOREXAMPLE == 1
  checkAlarm();   // example
#endif
}

//////////////////////////// Matrix functions ///////////////////////////////////////////

/**
 *  Initialize the matrix
 *
 * Scrollspeed 50 : each character takes ~275mS to display
 * Scrollspeed 100: each character takes ~550mS to display
 *
 * https://www.arduino.cc/reference/en/libraries/arduinographics/
 */
void init_matrix()
{
#if ENABLEMATRIX == 1
  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);    // stroke color of drawing operations
  matrix.textScrollSpeed(50);
  matrix.textFont(Font_5x7);    // will fit 2 characters.
#endif
}

/**
 * Display String text on LED matrix
 *
 * @param txt : string to display
 * @param scroldir: scroll direction
 *  M_NO    no scroll
 *  M_LEFT  scroll left
 *  M_RIGHT scroll right
 *  M_UP    scroll up
 *  M_DOWN  scroll down
 *
 * if txt is empty only a clear matrix will be performed
 */
void MessageMatrix(String txt, int scroldir)
{
#if ENABLEMATRIX == 1
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.flush();
  matrix.clear();
  if (txt.length() > 0) {
    matrix.print(txt);
    matrix.endText(scroldir);
  }
  matrix.endDraw();
#endif
}
