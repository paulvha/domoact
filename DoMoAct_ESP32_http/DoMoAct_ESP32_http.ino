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
  The processor board is the ESP32 MicroMod on a MicroMod ATP.
  See the hardware section in the DoMoAct-manual.odt (in extras) for more important information.

  In this case I have also used a 16 x 2 LCD, because I just still had it around.
  More information about this "cheap" LCD can be found on
  https://arduinogetstarted.com/tutorials/arduino-lcd-i2c?utm_content=cmp-true
  The library: http://librarymanager/All#liquidcrystal_I2C by Marco Schwartz / Frank de Brabander

  Created January 2024 / version 1.1 for ESP32
  
  todo : scramble DoMoAct password on Web-interface

  BEFORE POSTING
  Clear Spiff structure
  check manual
  update version
  remove SSID and PASS
  remove username and password
  disable debug messages
  disable example
  disable LCD
*/

#include "time.h"               // keep track of time
#include "src/TimeLord.h"       // included in the src folder of sketch already:  https://github.com/probonopd/TimeLord for sunset/sunrise
#include "DoMoAct.h" 
#include "FS.h"                 // for save configuration and backup
#include "SPIFFS.h"             // for save configuration and backup
#include <WiFi.h>               // all included in ESP32 library
#include <WiFiClient.h>
#include <WebServer.h>

#if ENABLELCD == 1
#include <LiquidCrystal_I2C.h>  // Click here to get the library: http://librarymanager/All#liquidcrystal_I2C by Marco Schwartz / Frank de Brabander
#endif

/////// please enter your sensitive data in DoMoAct.h ///////////////////
char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;      // your network password (use for WPA, or use as key for WEP)

char Acc_Name[] = DoMoAct_NAME; // Option to protect access to DoMoAct
char Acc_Pswd[] = DoMoAct_PASS;

/////////////// global variables ////////////

int  sunrise = 0;               // holds sunrise time
int  sunset = 0;                // holds sunset time
int  CurTime = 0;               // holds current time
int  led = LED_BUILTIN;         // led to provide some progress info
bool TimeDebug = false;         // enable debug message for time function (menu controlled)
bool WebDebug = false;          // enable debug message for Web function (menu controlled)
bool FastCheckEnable = false;   // is false when update has been made to configuration
bool AskLogin = false;          // true : DoMoAct needs login as username & password are set
uint8_t ResponseCode = 0;       // Response code to send back to client

MyObject CF_DoMoAct = {0};      // Parameter structure for configuration

////////////// instances ////////////////////
WebServer server(80);
TimeLord tardis;                // instance to calculate sunset & sunrise

#if ENABLELCD == 1              // include LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
#endif

void setup() {
  
  Serial.begin(115200);         // initialize serial communication
  while (! Serial);

  init_LCD();

  if (ssid[0] == '*' || pass[0] == '*') {
    Serial.println("ERROR : The network SSID has not been set. Freeze !");
    Serial.println("In tab DoMoAct.h make sure to define SECRET_SSID and SECRET_PASS first.");
    Serial.println("Also update longitude, latitude and TZ and daylight saving information");

    MessageLCD("E1",M_NO);
    while(1);
  }

  if (Acc_Name[0] != '*' && Acc_Pswd[0] != '*') // DoMoAct menu login required ?
      AskLogin = true;
  
  pinMode(led, OUTPUT);        // set the LED pin mode 
  digitalWrite(led,LOW);
  
  MessageLCD("S1",M_NO);

  if (! ConnectToWifi()) {
    Serial.println("could not connect to WIFI. Freeze");
    MessageLCD("E2",M_NO);
    while(1){
      delay(1000);
      digitalWrite(led,LOW);  
      delay(1000);
      digitalWrite(led,HIGH);  ;
    }
  }

  MessageLCD("S2",M_NO);

  digitalWrite(led,HIGH);     // Wifi is connected
  
  if (UpdateTime()){          // try to update / set time first time
    InitSunset();             // initialize TimeLord parameters for daily sunset / sunrise
    UpdateSunTimes();
  }
  else {
    Serial.println("FATAL ERROR: Can not obtain time from the NTP server. Freeze !");
    MessageLCD("E3",M_NO);

    while(1) {
      delay(1000);
      digitalWrite(led,LOW);  
      delay(1000);
      digitalWrite(led,HIGH);  
    }
  }

  MessageLCD("S3",M_NO);

  if(! CF_ReadConf(false))    // restore parameters (do not display)
  {
    Serial.println("WARNING ! WARNING ! COULD NOT READ CONFIGURATION");
  }
 
  InitSensors();             // Initialize Sensor variables and any sensor needed
  
  MessageLCD("S4",M_NO);
  
  StartServer();             // Start server

  printWifiStatus();         // you're connected now, so print out the status

  MessageLCD("OK",M_NO);

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
  CheckForTimeUpdate(false);  // update times once per day, do not force !
  
  DoSwitchOnOff();            // check whether a port / group / sensor needs a switch
  
  server.handleClient();      // This call will let the server do its work

  if (Serial.available()){    // check to open Serial menu after pressing <enter>
    StartMenu();
    while (Serial.available()) Serial.read(); // clear out any pending characters
  }
}

//////////////////////////// General functions ///////////////////////////////////////////
/** 
 *  utility function adds leading 0 if necessary
 */
String twoDigits(int digits)
{
  if (digits < 10) {
    String i = '0'+String(digits);
    return i;
  }
  else {
    return String(digits);
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
  InitDallas(); // Sensor_example
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
  checkAlarm();   // Sensor_example
#endif
}

//////////////////////////// LCD functions ///////////////////////////////////////////

/** 
 *  Initialize the LCD
 * 
 * https://arduinogetstarted.com/tutorials/arduino-lcd-i2c?utm_content=cmp-true
 */
void init_LCD()
{
#if ENABLELCD == 1
  lcd.init();         // initialize the lcd
  lcd.clear();        // clear display
  lcd.backlight();    // set backlight
#endif
}

/**
 * Display String text on LCD
 * 
 * @param txt : string to display
 * @param scroldir: scroll direction (reserved for future !!)
 *  M_NO    no scroll
 *  M_LEFT  scroll left  
 *  M_RIGHT scroll right
 *  M_UP    scroll up        used to display message on first line (used for time)
 *  M_DOWN  scroll down
 * 
 * if txt is empty only a clear matrix will be performed 
 */

void MessageLCD(String txt, int scroldir)
{
#if ENABLELCD == 1

  lcd.setCursor(0, 0);          // move cursor to   (0, 0)
  lcd.print("DoMoAct");         // print message at (0, 0)
  
  // clear bottom line
  lcd.setCursor(2, 1);          // move cursor to   (2, 1)
  lcd.print("                ");
  lcd.setCursor(2, 1);
  
  if (scroldir == M_UP) {       // add (new) text after DoMoAct on first line ?
    lcd.setCursor(10, 0);
    lcd.print("    ");
    lcd.setCursor(10, 0);
  }
  
  lcd.print(txt);
#endif
}
