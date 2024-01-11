/**
  Written by Paulvha January 2024
  Released under MIT licence.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  ===========//================//===========//================//=============//=============

  This tab contains the function to perform Port or group switch based on the timing set
  or whether a sensor has triggered that a port needs to change / switch.
*/

int SwitchTime, curWeekDay, NextSwitchTime = TIME_NOTASSIGNED;
int LastCheckTime = TIME_NOTASSIGNED;

//////////////////////////////////////////////////////////////////////////////////////////
// needs to be called each minute
/*
 * It will check whether it is now time for a port or group that needs to switch based on 
 * the time setting. It will also check whether a port needs to switch based on a sensor
 * channel setting.
 */
//////////////////////////////////////////////////////////////////////////////////////////
void DoSwitchOnOff()
{
  int j;
  
  // obtain current time variables
  int curHour = GetFromRTC(R_HOUR);
  int curMin = GetFromRTC(R_MINUTE);
  curWeekDay = GetFromRTC(R_WDAY);

  // translate to same format as sunset
  CurTime = (curHour * 60) + curMin;

  CheckSwitchSensors();
  
  // compare current time against a NEXT calculated time to switch on a port or group
  // FastCheckEnable is false when an update has been made to configuration /SPIFF since last DoSwitchOnOff()
  if (NextSwitchTime != TIME_NOTASSIGNED && FastCheckEnable) {
    if (CurTime != NextSwitchTime) return;
  }
  
  // We only check switch on time once per minute (why would you do more ?)
  if (LastCheckTime == CurTime) return;

#ifdef ENABLELCD == 1      // show time every minute when LCD is enabled
  String t = twoDigits(curHour) + (String) ':' + twoDigits(curMin);
  MessageLCD(t,M_UP);
#endif

  // save this check time
  LastCheckTime = CurTime;

  NextSwitchTime = TIME_NOTASSIGNED;
  
  // check the ports
  for (j = 0; j < CF_NUMPORTS; j++) {
  
    // if port is enabled and also enabled for this weekday 
    if (CF_DoMoAct.ports[j].Enable && ((CF_DoMoAct.ports[j].Day >> curWeekDay) & 0x1) == 0x1) {
   
      SwitchTime = TIME_NOTASSIGNED;
      
      //////////// ON //////////////
      if (CF_DoMoAct.ports[j].On_sunset) {   // switch on_sunset?
        
        SwitchTime = sunset;
        
        if (CF_DoMoAct.ports[j].On_minute != CF_NOTASSIGNED) {
          // subtract the minutes to switch on earlier
          SwitchTime = SwitchTime - CF_DoMoAct.ports[j].On_minute;
        }
      }
      else {        // fixed switch on time set ?
        if (CF_DoMoAct.ports[j].On_minute != CF_NOTASSIGNED && CF_DoMoAct.ports[j].On_hour != CF_NOTASSIGNED) {
          SwitchTime = (CF_DoMoAct.ports[j].On_hour * 60) + CF_DoMoAct.ports[j].On_minute;
        }
      }
  
      // if SwitchTime was NOT set on port, it must be handled manual or by group or sensor
      if (SwitchTime != TIME_NOTASSIGNED){
        
        if (SwitchTime == CurTime) {
          
          // switch ON if OFF
          if(! PortStatus[j])   SetPort(j, true);
         
        }
        else {  // So it was NOT the right time to switch for this port
          DetermineNextSwitchTime();
        }
      }
    }
    
    SwitchTime = TIME_NOTASSIGNED;
    
    // if port is enabled AND the port is switched ON
    if (CF_DoMoAct.ports[j].Enable && PortStatus[j]) {
      
      //////////// OFF //////////////
      if (CF_DoMoAct.ports[j].Off_sunrise) {
        
        SwitchTime = sunrise;
        
        if (CF_DoMoAct.ports[j].Off_minute != CF_NOTASSIGNED) {
          // add off_minute to On_sunrise
          SwitchTime = SwitchTime + CF_DoMoAct.ports[j].Off_minute;
        }
      }
      else {        // fixed switch on time set ?
 
        if (CF_DoMoAct.ports[j].Off_minute != CF_NOTASSIGNED && CF_DoMoAct.ports[j].Off_hour != CF_NOTASSIGNED){
         SwitchTime = (CF_DoMoAct.ports[j].Off_hour * 60) + CF_DoMoAct.ports[j].Off_minute;
        }
      }

      // if SwitchTime was NOT set on port, it must be handled by the group or sensor assigned or manual
      if (SwitchTime != TIME_NOTASSIGNED){
        
        if (SwitchTime == CurTime) {
          
          // switch OFF if ON
          if (PortStatus[j])  SetPort(j, false);
          
        }
        else {  // So it was NOT the right time to switch for this port
          DetermineNextSwitchTime();
        }
      }
    }
  }

  // check if a group is ready switch
  for (j = 0; j < CF_NUMGROUPS; j++) {

    SwitchTime = TIME_NOTASSIGNED;
    
    // if port is enabled and also enabled for this weekday
    if (CF_DoMoAct.groups[j].Enable && ((CF_DoMoAct.groups[j].Day >> curWeekDay) & 0x1) == 0x1) {
        
      //////////// ON //////////////
      if (CF_DoMoAct.groups[j].On_sunset) {

        SwitchTime = sunset;
        
        if (CF_DoMoAct.groups[j].On_minute != CF_NOTASSIGNED) {
          // subtract the minutes to switch on earlier
          SwitchTime = SwitchTime - CF_DoMoAct.groups[j].On_minute;
        }
      }
      else {        // fixed switch on time set ?
        if (CF_DoMoAct.groups[j].On_minute != CF_NOTASSIGNED && CF_DoMoAct.groups[j].On_hour != CF_NOTASSIGNED){
          SwitchTime = (CF_DoMoAct.groups[j].On_hour * 60) + CF_DoMoAct.groups[j].On_minute;
         }
      }

      // if nothing was set, nothing happens
      if (SwitchTime != TIME_NOTASSIGNED) {
        
        // turn ON group and ports on group
        if (SwitchTime == CurTime) SwitchPorts(j,true);
        else DetermineNextSwitchTime(); // So it was NOT the right time to switch
        
      }
    }
    
    SwitchTime = TIME_NOTASSIGNED;
    
    // if port is enabled AND Group is switched ON
    if (CF_DoMoAct.groups[j].Enable && GroupStatus[j]) {      
      //////////// OFF //////////////
      if (CF_DoMoAct.groups[j].Off_sunrise) {
        
        SwitchTime = sunrise;
        
        if (CF_DoMoAct.groups[j].Off_minute != CF_NOTASSIGNED) {
          // add off_minute to On_sunrise
          SwitchTime = SwitchTime + CF_DoMoAct.groups[j].Off_minute;
        }
      }
      else {        // fixed switch on time set ?
        if (CF_DoMoAct.groups[j].Off_minute != CF_NOTASSIGNED && CF_DoMoAct.groups[j].Off_hour != CF_NOTASSIGNED){
          SwitchTime = (CF_DoMoAct.groups[j].Off_hour * 60) + CF_DoMoAct.groups[j].Off_minute;
        }
      }
    
      // if nothing was set, nothing happens
      if (SwitchTime != TIME_NOTASSIGNED) {
        
        // turn off group and ports on group
        if (SwitchTime == CurTime) SwitchPorts(j,false);
        else  DetermineNextSwitchTime();   // So it was NOT the right time to switch
      
      }
    }
  }
}

/**
 * check whether a sensor has set an alarm
 */
void CheckSwitchSensors()
{
  // regular trigger sensors
  CheckSensors();
  
  // check if each sensor is now ready switch
  for (j = 0; j < CF_NUMSENSORS; j++) {

    bool sp = false;
    
    // if sensor did not indicate it wants to change status
    if (ChangeSensorStatus[j] == SENSOR_NOACT) continue;
    
    // if sensor is enabled and also enabled for this weekday
    if (CF_DoMoAct.sensors[j].Enable && ((CF_DoMoAct.sensors[j].Day >> curWeekDay) & 0x1) == 0x1) {

      if ( ChangeSensorStatus[j] == SENSOR_ON ) {
        if (! SensorStatus[j]) sp = true;     // was off then turn on. if ON already NO action?
        SensorStatus[j] = true;
      }
      else if (ChangeSensorStatus[j] == SENSOR_OFF ) {
        if (SensorStatus[j]) sp = true;        // was ON then turn off. if OFF already NO action
        SensorStatus[j] = false;
      }
      else {
        SensorStatus[j] = ! SensorStatus[j];   // flip
        sp = true;
      }

      // reset indicator
      ChangeSensorStatus[j] = SENSOR_NOACT;

      // trigger ports if sensor not set already
      if (sp) {
        
        // store in circular buffer
        CB_UpdateBuffer(SENSOR, j, SensorStatus[j]);

        // check each port   
        SwitchSensor(j);     
      }
    }
  }
}

/**
 * Switch all ports associated with the group
 * 
 * @param g : group number (must be valid and enabled)
 * @param SwitchOn : True is switch ON, else switch OFF
 * 
 * return :
 * -1 if not enabled
 * else the port counts
 */
int SwitchPorts(int g, bool SwitchOn)
{
  int p;
  if (! CF_DoMoAct.groups[g].Enable && !((CF_DoMoAct.groups[g].Day >> curWeekDay) & 0x1) == 0x1) 
    return -1;
  
  // get number of ports part of this groups        
  uint8_t gc = CF_DoMoAct.groups[g].count;

  // set requested status
  GroupStatus[g] = SwitchOn;
  
  // update circular buffer
  CB_UpdateBuffer(GROUP, g, SwitchOn);
  
  if (gc > 0) {
    
    // check each port       
    for (p = 0; p < CF_NUMPORTS && gc > 0; p++) {

      // check for port is part of group
      for (uint8_t i = 0; i < CF_NUMGROUPS; i++) {

        if (CF_DoMoAct.ports[p].Group[i] == g) {

          // check that port is enabled
          if (CF_DoMoAct.ports[p].Enable)
          {
            SetPort(p, SwitchOn);
            gc--;
          } // port enabled
        } // port is part of group
      } // for each group associated with the port      
    } // for each port
  } // port count of group

  return (CF_DoMoAct.groups[g].count);
}

/**
 * switch all assigned to a sensor
 * @param se : sensor number
 * 
 * return:
 *  -1 if sensor not enabled
 *  else number of (enabled) ports switched
 */
int SwitchSensor(int se)
{
  int ret = 0;
  
  // if sensor is enabled and also enabled for this weekday
  if (CF_DoMoAct.sensors[se].Enable && ((CF_DoMoAct.sensors[se].Day >> curWeekDay) & 0x1) == 0x1) {
  
    // check each port       
    for (p = 0; p < CF_NUMPORTS ; p++) {
      if (CF_DoMoAct.sensors[se].Ports[p] != CF_NOTASSIGNED) {
          // set port and count, if enabled
          if (SetPort(CF_DoMoAct.sensors[se].Ports[p], SensorStatus[se]) != -1) ret++;
      }
    }
  }
  else {
    ret = -1;   // not enabled
  }
  
  return (ret);
}

/** 
 *  set a port 
 *  @param p : port number
 *  @param c : true : on / false off
 *  
 *  Return true : 
 *  0. when set OFF
 *  1. when set on
 *  -1  NOT set as it was disabled
 */
int SetPort(int p, bool c)
{
  // Check enabled and enabled for this weekday
  if (CF_DoMoAct.ports[p].Enable && ((CF_DoMoAct.ports[p].Day >> curWeekDay) & 0x1) == 0x1) {
          
    pinMode(CF_DoMoAct.ports[p].Gpio,OUTPUT);
    
    if (c) digitalWrite(CF_DoMoAct.ports[p].Gpio, CF_DoMoAct.ports[p].ActiveHigh);
    else   digitalWrite(CF_DoMoAct.ports[p].Gpio,!CF_DoMoAct.ports[p].ActiveHigh);
    
    // add to circular buffer
    CB_UpdateBuffer(PORTS,p,c);

    // set port status
    PortStatus[p] = c;
    
    if (c) return 1;
    else return 0;
  }
  
  return -1;    // disabled
}

/**
 * detetermine what the next switch time is.
 */
void DetermineNextSwitchTime()
{
  // first check only
  if (NextSwitchTime == TIME_NOTASSIGNED && SwitchTime > CurTime) {
    NextSwitchTime = SwitchTime;
    return;
  }
  
  if (CurTime == 1439) { // 23 * 60 + 59 = 23:59
    // obtain the LOWEST SwitchTime as the next moment as it is on the next day
    if (NextSwitchTime > SwitchTime) NextSwitchTime = SwitchTime;
  }
  else {
    // obtain the Next (higher) SwitchTime as the next moment on the same day
    // if the detected switchtime is later today 

    if (SwitchTime > CurTime) {
      
      // if the NextSwitchTime is later today
      if (NextSwitchTime > CurTime) {
        // get the earliest after the current time
        if (SwitchTime < NextSwitchTime) {
          NextSwitchTime = SwitchTime;
        }
      }
      else {
        NextSwitchTime = SwitchTime;
      }
    }
  }
  
  FastCheckEnable = true;
}
