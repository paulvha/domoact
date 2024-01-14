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
  Parts are taken from the Udp NTP Client

  Get the time from a Network Time Protocol (NTP) time server
  Demonstrates use of UDP sendPacket and ReceivePacket
  For more on NTP time servers and the messages needed to communicate with them,
  see http://en.wikipedia.org/wiki/Network_Time_Protocol

  created 4 Sep 2010
  by Michael Margolis
  modified 9 Apr 2012
  by Tom Igoe
  modified May, 4th 2023
  by Daniele Aimo
  modified Dec, 7th, 2023
  by paulvha

  This code is in the public domain.

  Find the full UNO R4 WiFi Network documentation here:
  https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#wi-fi-udp-ntp-client

  sunset library taken from
  https://github.com/probonopd/TimeLord
 */

unsigned int localPort = 2390;        // local port to listen for UDP packets

IPAddress timeServer(162, 159, 200, 123); // pool.ntp.org NTP server

const int NTP_PACKET_SIZE = 48;       // NTP timestamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE];   // buffer to hold incoming and outgoing packets

#define MAX_RETRY 5                   // retry times to connect to NTP server

unsigned long EpochTime;              // holds UNIX time

// leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)  ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
int U_sec,U_min,U_hour, U_wd, U_year, U_mon, U_day;

/**
 * Set parameters for correct sunset times
 */
void InitSunset() {
  //// please enter the correct LATITUDE, LONGITUDE and UTC_OFFSET in DoMoAct.h
  tardis.TimeZone(UTC_OFFSET * 60);     // tell TimeLord what timezone your RTC is synchronized to.
  tardis.Position(LATITUDE, LONGITUDE); // tell TimeLord where in the world we are
}

/**
 * calculate sunrise / sunset times
 */
void UpdateSunTimes()
{
  // need to add 1 to month as it starts with month 0 = January
  byte today[] = { calcSeconds(), calcMinutes(), calcHours(), calcDay(), calcMonth() + 1, calcYear() }; // store today's date (at noon) in an array for TimeLord to use (format SS, MM, HH, DD, MM, YY)

  if (tardis.SunRise(today)) // if the sun will rise today (it might not, in the [ant]arctic)
  {
    sunrise = ((int) (today[tl_hour] + CF_DoMoAct.DST)* 60) + (int) today[tl_minute]; // adjust Daylight Saving Time
  }
   
  if (tardis.SunSet(today)) // if the sun will set today (it might not, in the [ant]arctic)
  {
    sunset = ((int) (today[tl_hour] + CF_DoMoAct.DST)* 60) + (int) today[tl_minute];  // adjust Daylight Saving Time
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
 * read NTP Server UNIX time and update the RTC
 * return:
 *  true:  OK
 *  false: problems
 */
bool UpdateTime() 
{
  if (TimeDebug) Serial.println("\nStarting connection to UDP server...");
  
  for (int i = 0 ; i < MAX_RETRY; i++)
  {
    // get time since 1-1-1970
    EpochTime = getNtpTime();
    if (EpochTime > 0) break;
  }
  
  if (EpochTime == 0)
  {
    Serial.println("ERROR : could not update time");
    return false;
  }
  
  // to date
  UnixTimeToDate();
  
  // adjust for Day light Saving
  DaylightSaving();
  
  // update RTC time
  // Set time (day, month, year, hours, minutes, seconds, dayofweek, daylight saving)
  // Daylight saving setting is only stored.. DOES NOTHING !! hence it is handled in this code
  RTCTime mytime(calcDay(), (Month) calcMonth(), calcYear(), calcHours(), calcMinutes(), calcSeconds(), (DayOfWeek) calcDayWeek(),SaveLight::SAVING_TIME_INACTIVE);
  RTC.setTime(mytime);

  if (TimeDebug)
  {
    Serial.print("Unix time = ");
    Serial.println(EpochTime);

    Serial.print("The year is "); 
    Serial.println(calcYear());

    Serial.print("The month is "); 
    Serial.println(calcMonth());

    Serial.print("The day is "); 
    Serial.println(calcDay());

    // print the hour, minute and second:
    Serial.print("The local time is ");
    Serial.print(twoDigits(calcHours())); 
    Serial.print(':');
    Serial.print(twoDigits(calcMinutes())); 
    Serial.print(':');
    Serial.println(twoDigits(calcSeconds())); // print the second
  
    // Print the date and time from the RTC
    RTCTime currentTime;
    RTC.getTime(currentTime);
    Serial.println("The RTC was just set to: " + String(currentTime));
  }
  
  // set call back at 04.00 in the night to update time and sunset time
  RTCTime alarmTime;
  alarmTime.setHour(4);

  AlarmMatch matchTime;
  matchTime.addMatchHour();
    
  //sets the alarm callback
  RTC.setAlarmCallback(alarmCallback, alarmTime, matchTime);

  TriggerUpdateTime = false;
  
  return true;
}

/**
 * Calculate daylight saving for Western Europe
 * 
 * https://www.instructables.com/The-Arduino-and-Daylight-Saving-Time-Europe/
 * 
 */
void DaylightSaving()
{  
////////////// turn ON Daylightsaving
#if DAYLIGHTSAVING == 1 // EU
  // DST starts last sunday in March at 2.00AM. Move the clock 1 hour forward
  //       sunday              March (jan = 0)      more than 25th (last sunday)    > 2.00   AND NOT set yet
  if (calcDayWeek() == 0  && calcMonth() == 2 && calcDay() >= 25 && calcHours() > 2 && CF_DoMoAct.DST==0)

#elif DAYLIGHTSAVING == 2 // USA
  // DST start second sunday in march at 2.00AM. Nove the clock 1 hour forward
  //       sunday              March            more than 7th (second sunday)  > 2.00   AND NOT set yet
  if (calcDayWeek() == 0  && calcMonth() == 2 && calcDay() >= 7 && calcHours() > 2 && CF_DoMoAct.DST==0)
#else
#error: current DAYLIGHTSAVING setting NOT supported (check in tab DoMoAct.h)
#endif
  {
    CF_Set_DST(1);    // set daylight saving
    
    // apply changes for today. Following days it will be handled in the return from getNtpTime()
    EpochTime += 3600;
    UnixTimeToDate(); // recalculated with updated EpochTime
  }

////////////// turn off Daylightsaving
#if DAYLIGHTSAVING == 1 // EU
  // DST ends last sunday in October at 3.00AM. Move the clock 1 hour backward
  //       sunday              october (jan = 0)        more than 25th (last sunday)  3.00     AND not set yet
  if (calcDayWeek() == 0 && calcMonth() == 9 && calcDay() >= 25 && calcHours() > 3 && CF_DoMoAct.DST==1)
  
#elif DAYLIGHTSAVING == 2 // USA
  // DST ends FIRST sunday in November at 3.00AM. Move the clock 1 hour backward
  //       sunday              November            more than 25th (last sunday)  3.00     AND not set yet
  if (calcDayWeek() == 0 && calcMonth() == 10  && calcHours() > 3 && CF_DoMoAct.DST==1)
#endif
  {
    CF_Set_DST(0);    // remove daylight saving
    
    // apply changes for today. Following days it will be handled in the return from getNtpTime()
    EpochTime -=  3600;
    UnixTimeToDate(); // recalculated with updated EpochTime
  }
}

/**
 *  this function if called at 01.00
 */
void alarmCallback() {
  TriggerUpdateTime = true; 
}

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

/**
 * connect with NTPserver
 * 
 * return UNIX-Time
 */
unsigned long getNtpTime()
{
  Udp.begin(localPort);
  
  while (Udp.parsePacket() > 0) ; // discard any previously received packets

  sendNTPpacket(timeServer);
  
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      if(TimeDebug > 1) Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];

      if (TimeDebug > 1){
          Serial.print("secsSince1900 time = ");
          Serial.println(secsSince1900);
      }
      Udp.stop();
      return secsSince1900 - 2208988800UL + ((UTC_OFFSET + CF_DoMoAct.DST) * 3600);
    }
  }
  
  Serial.println("No NTP Response :-(");
  Udp.stop();
  return 0; // return 0 if unable to get the time
}

/** 
 *  send an NTP request to the time server at the given address
 */
void sendNTPpacket(IPAddress& address) {
 
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  if(TimeDebug > 1) Serial.println("1");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  
  if(TimeDebug > 1) Serial.println("2");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  
  if(TimeDebug > 1) Serial.println("3");
  Udp.endPacket();
  
  if(TimeDebug > 1) Serial.println("4");
}

/**
 * Unix time to Data conversion
 * based on https://github.com/PaulStoffregen/Time
 */
void UnixTimeToDate(){
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t) EpochTime;
  U_sec = time % 60;
  time /= 60; // now it is minutes
  U_min = time % 60;
  time /= 60; // now it is hours
  U_hour = time % 24;
  time /= 24; // now it is days
  U_wd = ((time + 4) % 7);  // Sunday is day 0 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  U_year = year + 1970; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  
  U_mon = month;    // jan is month 0  
  U_day = time + 1; // day of month
}

/**
 * get human readable values
 */
int calcYear()    {return(U_year);}
int calcMonth()   {return(U_mon);}     // 0 = January
int calcDay()     {return(U_day);} 
int calcDayWeek() {return(U_wd);}      // 0 is Sunday
int calcHours()   {return(U_hour);}
int calcMinutes() {return(U_min);}
int calcSeconds() {return(U_sec);}

/**
 * get part of time from RTC
 * 2023-11-07T16:01:18
 */
int GetFromRTC(int v)
{
  int ret;
  RTCTime currentTime;
  RTC.getTime(currentTime);

  switch(v) {
    case R_WDAY:
      ret = (int) currentTime.getDayOfWeek();
      break;
    case R_DAY:
      ret = currentTime.getDayOfMonth();
      break;
    case R_MONTH:
      ret = (int) currentTime.getMonth();
      break;
    case R_YEAR:
      ret = currentTime.getYear();
      break;
    case R_HOUR:
      ret = currentTime.getHour();
      break;
    case R_MINUTE:
      ret = currentTime.getMinutes();
      break;
    case R_SECOND:
      ret = currentTime.getSeconds();
      break;
  }
  return ret;
}
