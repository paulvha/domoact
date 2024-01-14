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

  sunset library taken from
  https://github.com/probonopd/TimeLord

  time.h information taken from
  https://sourceware.org/newlib/libc.html#Timefns

  Timezone info from
  https://werner.rothschopf.net/microcontroller/202103_arduino_esp32_ntp_en.htm
  
 */
 
const char* ntpServer = "pool.ntp.org";
#define MAX_RETRY 5                   // retry times to connect to NTP server
bool WaitTimeUpdate = true;
struct tm t;

/**
 * Set parameters for correct sunset times
 */
void InitSunset() {
  //// please enter the correct LATITUDE, LONGITUDE and UTC_OFFSET in DoMoAct.h
  tardis.TimeZone(UTC_OFFSET * 60);     // tell TimeLord what timezone your time is synchronized to.
  tardis.Position(LATITUDE, LONGITUDE); // tell TimeLord where in the world we are
}

/**
 * calculate sunrise / sunset times
 */
void UpdateSunTimes()
{
  // store today's date (at noon) in an array for TimeLord to use (format SS, MM, HH, DD, MM, YY)
  // added 1 to the day as the calculation is not perfect and this makes the result closer
  // added 1 to month as month starts with 0
  // remove 2000 from year to fit uint8_t.
  // seconds, minute, hour are NOT used in calculation.
  // year is only relevant if difference to 
  uint8_t today[] = { GetFromRTC(R_SECOND), GetFromRTC(R_MINUTE), GetFromRTC(R_HOUR), GetFromRTC(R_DAY)+1, GetFromRTC(R_MONTH)+1, GetFromRTC(R_YEAR)- 2000 }; 

  if (tardis.SunRise(today)) // if the sun will rise today (it might not, in the [ant]arctic)
  {
    sunrise = ((int) today[tl_hour] * 60) + (int) today[tl_minute];
  }
  
  // added 1 to the day as the calculation is not perfect and this makes the result closer
 // uint8_t today1[] = { GetFromRTC(R_SECOND), GetFromRTC(R_MINUTE), GetFromRTC(R_HOUR), GetFromRTC(R_DAY)+1, GetFromRTC(R_MONTH)+1, GetFromRTC(R_YEAR)- 2000 }; 
   
  if (tardis.SunSet(today)) // if the sun will set today (it might not, in the [ant]arctic)
  {
    sunset = ((int) today[tl_hour] * 60) + (int) today[tl_minute]; 
  }

  if (TimeDebug) {
    Serial.print("Sunrise at ");
    Serial.print(sunrise/60);
    Serial.print(":");
    Serial.print(twoDigits(sunrise%60));
    Serial.print("am, Sunset at ");
    Serial.print(sunset/60);
    Serial.print(":");
    Serial.print(twoDigits(sunset%60));
    Serial.println("pm");
  }
}

/**
 * read NTP Server 
 * return:
 *  true:  OK
 *  false: problems
 */
bool UpdateTime() 
{
  if (TimeDebug) Serial.println("\nStarting connection to NTP server...");
  int i;
  
  for (i = 0 ; i < MAX_RETRY; i++)
  {
    // Init and get the time
    configTime(0, 0, ntpServer);      // 0, 0 because we will use TZ from tzset()
    setenv("TZ", MY_TZ, 1);           // Set environment variable with your time zone (DoMoAct.h)
    tzset();
    
    if (getLocalTime(&t)) break;
    Serial.print("Failed to obtain time from NTP.");

    if (i != MAX_RETRY - 1) Serial.println("Retry");
    else Serial.println("Stopped trying");
  }

  // Didn't we get NTP connection ?
  if (i == MAX_RETRY) return false;

  if (TimeDebug) printLocalTime();
 
  return true;
}

void printLocalTime(){

  if(!getLocalTime(&t)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&t, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&t, "%A");
  Serial.print("Month: ");
  Serial.println(&t, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&t, "%d");
  Serial.print("Year: ");
  Serial.println(&t, "%Y");
  Serial.print("Hour: ");
  Serial.println(&t, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&t, "%I");
  Serial.print("Minute: ");
  Serial.println(&t, "%M");
  Serial.print("Second: ");
  Serial.println(&t, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &t);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &t);
  Serial.println(timeWeekDay);
  Serial.println();
}

/**
 * Try to update times and set backup every night at 04.00
 * called from loop
 * @param force : true will trigger update always
 */
void CheckForTimeUpdate(bool force) {

  // every night at 04.00
  if (GetFromRTC(R_HOUR) == 4 || force) {
     
    // update not done yet 
    if (WaitTimeUpdate || force) {     // WaitTimeUpdate will cause an update only once at 04.00
    
      if (UpdateTime()) {
        UpdateSunTimes();             // calculate today's Sunset and sunrise
        CF_WriteBackUpConf(false);    // Parameters to backup location if changed
        WaitTimeUpdate = false;
      }
    }
  }
  else {  // if not 04.00 reset for coming 04.00
    WaitTimeUpdate = true;
  }  
}

/**
   get part of time from RTC or simulated RTC
   2023-11-07T16:01:18
*/
int GetFromRTC(int v)
{
  int ret = -1;
  if(!getLocalTime(&t)){
    Serial.println("Failed to obtain time");
    return ret;
  }

  switch (v) {
    case R_WDAY:
      ret =  t.tm_wday;
      break;
    case R_DAY:
      ret = t.tm_mday;
      break;
    case R_MONTH:
      ret = t.tm_mon;
      break;
    case R_YEAR:
      ret = t.tm_year+1900;
      break;
    case R_HOUR:
      ret = t.tm_hour;
      break;
    case R_MINUTE:
      ret = t.tm_min;
      break;
    case R_SECOND:
      ret = t.tm_sec;
      break;
    case R_DST:              //get daylight saving 1 = yes, 0 = no
      ret = t.tm_isdst;
      break;
  }

  return ret;
}
