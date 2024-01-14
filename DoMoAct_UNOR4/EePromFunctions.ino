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
*/

//////////////////////// Export / Import Functions//////////////////////
union data {
  MyObject domo;
  byte  d[sizeof(struct MyObject)];
} data;

#define CF_HEADER 0xEE    // Backup Headers
#define LINELENGTH 140    // each export / import line backup max line length
uint8_t BackUp[512];      // store import line received
uint16_t DomoLength = 0;  // length of received import line
int Data_p = 0;           // point to restore
uint8_t CF_CS = 0;        // checksum on total configuration during export/import
int  FoundHeader = 0;     // used during parsing line :1:

////////////////////////////////////////////////////////////////////////

int eeAddress = 0;                         // Location where the data is put
int BackUp_eeAddress = eeAddress + 0x400;  // backup location (1k up)
bool UpdateBackup = false;                 // true : Only update if original changed

/**
 * Set initial values
 * @param disp : true = display the values set
 * @param SetBack ; overwrite also backup location
 */
void CF_InitEeprom(bool disp, bool SetBack) {

  CF_DoMoAct = {0};                  // set all to zero

  CF_DoMoAct.DisplaySwitch = true;  // display switching on Serial and led matrix (if ENABLEMATRIX == 1)
  CF_DoMoAct.WebBackground = 1   ;  // default web background
  CF_DoMoAct.DST = 0;               // Day light Saving offset

  for(int x = 0; x < CF_NUMDASHBOARD; x++) {   // clear dashboard item
    CF_DoMoAct.DashBoard[x].PGS = CF_NOTASSIGNED;
  }
  
  String s = "Port";
  
  for (uint8_t p = 0 ; p < CF_NUMPORTS ; p++){
    
    // set name
    String N  = s + String(p);
    for (int x = 0 ; x < N.length() && x < CF_NAMELEN ; x++) {
      CF_DoMoAct.ports[p].Name[x] = N[x];
    }
    
    // port is off
    PortStatus[p] = false;
    
    // set for active high
    CF_DoMoAct.ports[p].ActiveHigh = true;
    
    // disable
    CF_DoMoAct.ports[p].Enable = false;
    PortStatus[p]= false;
    
    // enable all days
    CF_DoMoAct.ports[p].Day = 0x7F;

    // NO GPIO assigned
    CF_DoMoAct.ports[p].Gpio = CF_NOTASSIGNED;

    // Not assigned to group
    for(int j = 0; j < CF_NUMGROUPS;j++)
      CF_DoMoAct.ports[p].Group[j] = CF_NOTASSIGNED;

    // No time set
    CF_DoMoAct.ports[p].On_sunset = false;
    CF_DoMoAct.ports[p].On_minute = CF_NOTASSIGNED;
    CF_DoMoAct.ports[p].On_hour = CF_NOTASSIGNED;
    
    CF_DoMoAct.ports[p].Off_sunrise = false;    
    CF_DoMoAct.ports[p].Off_minute = CF_NOTASSIGNED;
    CF_DoMoAct.ports[p].Off_hour = CF_NOTASSIGNED;

    // optional
    CF_DoMoAct.ports[p].future1 = CF_NOTASSIGNED;
    CF_DoMoAct.ports[p].future2 = CF_NOTASSIGNED;
    CF_DoMoAct.ports[p].futureb1 = false;
    CF_DoMoAct.ports[p].futureb2 = false;
    CF_DoMoAct.ports[p].futuref1 = CF_NOTASSIGNED;
    CF_DoMoAct.ports[p].futuref2 = CF_NOTASSIGNED;
  }

  s = "Group";
  for (uint8_t g = 0 ; g < CF_NUMGROUPS ; g++){
    
    // set Groupname
    String N  = s + String(g);
    for (int x = 0 ; x < N.length() && x < CF_NAMELEN ; x++) {
      CF_DoMoAct.groups[g].Name[x] = N[x];
    }
    
    // Group is off
    GroupStatus[g] = false;

    // NO ports assigned
    CF_DoMoAct.groups[g].count = 0;
    
    // disable
    CF_DoMoAct.groups[g].Enable = false;
    GroupStatus[g]= false;
    
    // enable all days
    CF_DoMoAct.groups[g].Day = 0x7F;

    // No time set
    CF_DoMoAct.groups[g].On_sunset = false;
    CF_DoMoAct.groups[g].On_minute = CF_NOTASSIGNED;
    CF_DoMoAct.groups[g].On_hour = CF_NOTASSIGNED; 
    CF_DoMoAct.groups[g].Off_sunrise = false;    
    CF_DoMoAct.groups[g].Off_minute = CF_NOTASSIGNED;
    CF_DoMoAct.groups[g].Off_hour = CF_NOTASSIGNED;

    // optional
    CF_DoMoAct.groups[g].future1 = CF_NOTASSIGNED;
    CF_DoMoAct.groups[g].future2 = CF_NOTASSIGNED;
    CF_DoMoAct.groups[g].futureb1 = false;
    CF_DoMoAct.groups[g].futureb2 = false;
    CF_DoMoAct.groups[g].futuref1 = CF_NOTASSIGNED;
    CF_DoMoAct.groups[g].futuref2 = CF_NOTASSIGNED;
  }
  
  s = "Sensor";
  for (uint8_t g = 0 ; g < CF_NUMSENSORS ; g++){
    
    // set name
    String N  = s + String(g);
    for (int x = 0 ; x < N.length() && x < CF_NAMELEN ; x++) {
      CF_DoMoAct.sensors[g].Name[x] = N[x];
    }
    
    // port is off
    SensorStatus[g] = false;

    // not change set
    ChangeSensorStatus[g] = SENSOR_NOACT;
    
    // disable
    CF_DoMoAct.sensors[g].Enable = false;
    ChangeSensorStatus[g] = SENSOR_NOACT;
    SensorStatus[g] = false;
    
    // enable all days
    CF_DoMoAct.sensors[g].Day = 0x7F;

    // No ports assigned
    for(int j = 0; j < CF_NUMPORTS;j++)
      CF_DoMoAct.sensors[g].Ports[j] = CF_NOTASSIGNED;

    // optional
    CF_DoMoAct.sensors[g].UserInt1 = CF_NOTASSIGNED;
    CF_DoMoAct.sensors[g].UserInt2 = CF_NOTASSIGNED;
    CF_DoMoAct.sensors[g].UserBool1 = false;
    CF_DoMoAct.sensors[g].UserBool2 = false;
    CF_DoMoAct.sensors[g].UserFloat1 = CF_NOTASSIGNED;
    CF_DoMoAct.sensors[g].UserFloat2 = CF_NOTASSIGNED;
  }
  
  // update EEPROM
  CF_WriteToEEprom();
  
  // force update the backup
  if (SetBack) {
    CF_WriteBackUpToEEprom(true);
    CF_ReadBackUpEeprom(disp, false);
  }
}

////////////////////// EEPROM handling ////////////////////////////////////
/**
 * read current values from EEPROM into a structure
 * 
 * @param disp : true = display retrieved information
 */
void CF_ReadEeprom(bool disp)
{
  EEPROM.get(eeAddress, CF_DoMoAct);
  if (disp) DispObjectInfo();
}

/**
 * write the parameters structure to EEPROM
 */
void CF_WriteToEEprom()
{
  CF_DoMoAct.Version = CF_VERSION;   // always add version
  EEPROM.put(eeAddress, CF_DoMoAct);
  UpdateBackup = true;               // indicate that EEPROM backup needs to be performed 
  FastCheckEnable = false;           // disable fast checking in DoSwitchOnOff() 
}

/**
 * read current values structure from backup location
 * 
 * @param disp   : true = display restored information
 * @param bcheck : true: Only check that backup is correct version, do not restore
 */
bool CF_ReadBackUpEeprom(bool disp, bool bcheck)
{
  // get from backup in EEPROM
  EEPROM.get(BackUp_eeAddress, data.domo);

  // check for good version number
  if (data.domo.Version != (float) CF_VERSION ){
    Serial.println("Invalid version. NOT saved");
    return false; 
  }
  
  // IF only check for correct version but do not restore is false
  // do not overwrite
  if (! bcheck) {  
    EEPROM.put(eeAddress, data.domo);  // restore backup to EEPROM normal position
    CF_ReadEeprom(disp);               // read it and display (if requested)
  }
  
  return(true);
}

/**
 * write the parameters structure to EEPROM backup location
 * @param force : will update backup even original has not be written.
 */
void CF_WriteBackUpToEEprom(bool force)
{
  if (force || UpdateBackup ){
    CF_DoMoAct.Version = CF_VERSION;   // always add version
    EEPROM.put(BackUp_eeAddress, CF_DoMoAct);
    UpdateBackup = false;
  }
}

/** 
 *  clear all the EEPROM values
 */
void CF_ClearEeprom()
{
  CF_DoMoAct = {0};
  CF_WriteToEEprom();
}

/**
 * enable / disable display switching on serial and led matrix
 * Led matrix needs to be enabled seperately with ENABLEMATRIX in tab DoMoAct.h
 */
void CF_SetDisplaySwitch(bool Set)
{
  CF_ReadEeprom(false);
  CF_DoMoAct.DisplaySwitch = Set;
  CF_WriteToEEprom();
}

/**
 * set current WebBackground
 */
void CF_Set_WebBackGround(int Set)
{
  CF_ReadEeprom(false);
  CF_DoMoAct.WebBackground = Set;
  CF_WriteToEEprom();
}

/**
 * set Daylight Saving Time
 */
void CF_Set_DST(int Set)
{
  CF_ReadEeprom(false);
  CF_DoMoAct.DST = Set;
  CF_WriteToEEprom();
}

/**
 * set Dashboard
 * 
 * @param d : PORTS, GROUP OR SENSOR
 * @param n : number
 * @param Set: add (true) or remove
 * return:
 * true : all OK
 * false : table is full
 * 
 */
bool CF_SetDashboard (int d, int n, bool Set)
{
  int h;
  CF_ReadEeprom(false);

  // check whether it is in Dashboard already
  for(h = 0; h < CF_NUMDASHBOARD; h++) {
    
    if ( CF_DoMoAct.DashBoard[h].PGS == d && CF_DoMoAct.DashBoard[h].num == n ){
      
      if (! Set) {            //remove requested
        CF_DoMoAct.DashBoard[h].PGS = CF_NOTASSIGNED;
        CF_WriteToEEprom();
      }
      
      return true;
    }
  }
  
  // At this point it was NOT already in Dashboard or was removed on request//

  // if remove was requested, we are done.
  if (!Set) return true;

  // find the first empty slot
  for(h = 0; h < CF_NUMDASHBOARD; h++) {
    
    if ( CF_DoMoAct.DashBoard[h].PGS == CF_NOTASSIGNED ){
      CF_DoMoAct.DashBoard[h].PGS = d;
      CF_DoMoAct.DashBoard[h].num = n;
      break;
    }
  }
  
  // no empty slot
  if (h == CF_NUMDASHBOARD) return false;
  
  CF_WriteToEEprom();
  
  return true;
}
//////////////////////// Display functions ////////////////////////////////
/**
 * display complete CF_DoMoAct / EEPROM information
 */
void DispObjectInfo()
{
  DispGeneralinfo();
  
  for(int i = 0; i < CF_NUMPORTS; i++) 
    DispPortInfo(i,true);

  for(int i = 0; i < CF_NUMGROUPS; i++) 
    DispGroupInfo(i,true);

  for(int i = 0; i < CF_NUMSENSORS; i++) 
    DispSensorInfo(i,true);
}

/**
 * Display the generic DoMoAct parameters
 */
void DispGeneralinfo()
{
  Serial.print("Display object from EEPROM. Version: ");
  Serial.println(CF_DoMoAct.Version);

  Serial.print("Display switching on Serial and led matrix: ");
  if (CF_DoMoAct.DisplaySwitch) Serial.println("Enabled");
  else Serial.println("Disabled");
  
  Serial.print("Display Web Background number: ");
  Serial.println(CF_DoMoAct.WebBackground);

  Serial.print("DayLight Saving time: ");
  if (CF_DoMoAct.DST) Serial.println("Active");
  else Serial.println("Inactive");

  if(ShowDashboard() == 0) {
    Serial.println("Dashboard is empty");
  }
}

/**
 * display a single group info
 * @param i: group number
 * @param d: true display group number
 */
void DispGroupInfo(int i, bool d)
{
  if (d) {
    Serial.print("Group: ");
    Serial.println(i);
  }
  
  Serial.print("\tmembercount: ");
  Serial.print(CF_DoMoAct.groups[i].count);
  Serial.print("\tEnabled: ");
  if (CF_DoMoAct.groups[i].Enable) Serial.print("Yes");
  else Serial.print("no");
  Serial.print("\tName: ");
  Serial.println(CF_DoMoAct.groups[i].Name);
  
  Serial.print("\tOnHour: ");
  if (CF_DoMoAct.groups[i].On_hour == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.groups[i].On_hour);      // Which hours ON
  Serial.print("\tOnMinute: ");
  if (CF_DoMoAct.groups[i].On_minute == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.groups[i].On_minute);    // Which minute ON
  Serial.print("\tOn_sunset: ");
  Serial.println(CF_DoMoAct.groups[i].On_sunset);
  
  Serial.print("\tOffHour: ");
  if (CF_DoMoAct.groups[i].Off_hour == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.groups[i].Off_hour);     // Which hours OFF
  Serial.print("\tOffMinute: ");
  if (CF_DoMoAct.groups[i].Off_minute == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.groups[i].Off_minute);   // Which minute OFF
  Serial.print("\tOff_sunrise: ");
  Serial.println(CF_DoMoAct.groups[i].Off_sunrise); 
  
  Serial.print("\tStatus: ");                       // true = on, false = off
  if (GroupStatus[i]) Serial.println("on");
  else Serial.println("off");
        
  DisplayWorkDays(CF_DoMoAct.groups[i].Day,false);  // which days (bit 0 = sunday, 1 = ON, 0 = OFF)
  Serial.println();
}

/**
 * display a single port
 * @param i: portnumber
 * @param d: true display port number
 */
void DispPortInfo(int i, bool d)
{
  if (d) {
    Serial.print("Port: ");
    Serial.println(i);
  }
  Serial.print("\tGpio: ");
  if (CF_DoMoAct.ports[i].Gpio == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.ports[i].Gpio);  
  if(CF_DoMoAct.ports[i].Gpio < 10) Serial.print("\t"); // to align output
  
  Serial.print("\tEnabled: ");
  if (CF_DoMoAct.ports[i].Enable) Serial.print("Yes");
  else Serial.print("no");
  Serial.print("\tName: ");
  Serial.println(CF_DoMoAct.ports[i].Name);
  
  Serial.print("\tOnHour: ");
  if (CF_DoMoAct.ports[i].On_hour == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.ports[i].On_hour);      // Which hours ON
  Serial.print("\tOnMinute: ");
  if (CF_DoMoAct.ports[i].On_minute == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.ports[i].On_minute);    // Which minute ON
  Serial.print("\tOn_sunset: ");
  Serial.println(CF_DoMoAct.ports[i].On_sunset);
  
  Serial.print("\tOffHour: ");
  if (CF_DoMoAct.ports[i].Off_hour == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.ports[i].Off_hour);     // Which hours OFF
  Serial.print("\tOffMinute: ");
  if (CF_DoMoAct.ports[i].Off_minute == CF_NOTASSIGNED) Serial.print("---");
  else Serial.print(CF_DoMoAct.ports[i].Off_minute);   // Which minute OFF
  Serial.print("\tOff_sunrise: ");
  Serial.println(CF_DoMoAct.ports[i].Off_sunrise); 
  
  Serial.print("\tStatus: ");
  if (PortStatus[i]) Serial.print("on");
  else Serial.print("off");       // true = on, false = off

  Serial.print("\tActive_Level: "); 
  if (CF_DoMoAct.ports[i].ActiveHigh)  Serial.println("HIGH");
  else Serial.println("LOW");
  
  DisplayWorkDays(CF_DoMoAct.ports[i].Day,false); // which days (bit 0 = sunday, 1 = ON, 0 = OFF)

  for(int j=0; j < CF_NUMGROUPS ; j++) {
    
    if (CF_DoMoAct.ports[i].Group[j] != CF_NOTASSIGNED){
        Serial.print("\n\tGroup: ");
        Serial.print(CF_DoMoAct.ports[i].Group[j], HEX); // part of a group
        Serial.print(" name: ");
        Serial.println(CF_DoMoAct.groups[CF_DoMoAct.ports[i].Group[j]].Name);
    }
  }
  Serial.println();
}
/**
 * display a single sensor
 * @param i: sensor number
 * @param d: true display sensor number
 */
void DispSensorInfo(int i, bool d)
{
  if (d) {
    Serial.print("Sensor channel: ");
    Serial.println(i);
  }

  Serial.print("\tEnabled: ");
  if (CF_DoMoAct.sensors[i].Enable) Serial.print("Yes");
  else Serial.print("no");

  Serial.print("\tName: ");
  Serial.println(CF_DoMoAct.sensors[i].Name);
  
  Serial.print("\tStatus: ");
  if (SensorStatus[i]) Serial.print("on");
  else Serial.print("off");       // true = on, false = off

  Serial.print("\tChangeSensorStatus: ");
  if (ChangeSensorStatus[i] == SENSOR_ON) Serial.println("switch ON");
  else if (ChangeSensorStatus[i] == SENSOR_OFF) Serial.println("switch OFF");
  else if (ChangeSensorStatus[i] == SENSOR_FLIP) Serial.println("switch FLIP");
  else if (ChangeSensorStatus[i] == SENSOR_NOACT) Serial.println("No action");
  
  DisplayWorkDays(CF_DoMoAct.sensors[i].Day,false); // which days (bit 0 = sunday, 1 = ON, 0 = OFF)
  
  for(int j = 0; j < CF_NUMPORTS ; j++) {
    
    if (CF_DoMoAct.sensors[i].Ports[j] != CF_NOTASSIGNED){
        Serial.print("\n\tPort: ");
        Serial.print(CF_DoMoAct.sensors[i].Ports[j], HEX); // part of sensor
        Serial.print(" name: ");
        Serial.println(CF_DoMoAct.ports[CF_DoMoAct.sensors[i].Ports[j]].Name);
    }
  }
  Serial.println();
}

//////////////////////// PORT handling ////////////////////////////////////////////
/**
 * enable / disable day to trigger on or off
 *  @param e : true: enable day / false : disable day
 *  @param d : which day (sunday = 0)
 *  &param p : port to set
 */
void CF_PT_SetDay(bool e, uint8_t d, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  
  if (e)  CF_DoMoAct.ports[p].Day |= 1 << d;
  else CF_DoMoAct.ports[p].Day &= ~(1 << d);
  
  CF_WriteToEEprom(); 
}

/**
 *  set/ reset Off_sunrise of port in EEPROM
 *  
 *  @param s : true: use off_minutes to switch OFF AFTER before sunrise /  false: use Off_hour / Off_minute to switch off
 *  &param p : port to set
 */
void CF_PT_Offsunrise(bool s, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].Off_sunrise = s;
  CF_WriteToEEprom();
}

/**
 *  set the GPIO of port HIGH or LOW in active state
 *  
 *  @param s : true: use HIGH active /  false: use LOW active
 *  &param p : port to set
 */
void CF_PT_ActiveLevel(bool s, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].ActiveHigh = s;
  CF_WriteToEEprom();
}

/**
 *  set / reset On_sunset of port in EEPROM
 *  
 *  @param s : true: use on_minutes to switch ON BEFORE before sunset / false: use On_hour/ On_minute to switch on
 *  &param p : port to set
 */
void CF_PT_Onsunset(bool s, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].On_sunset = s;
  CF_WriteToEEprom();
}

/**
 *  set / reset Enable of port in EEPROM
 *  @param s : true: enable port / false : disable
 *  @param p : port to set
 */
void CF_PT_Enable(bool s, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);

  // if disable port and currently ON, reset GPIO
  if(!s && PortStatus[p]){
    SetPort(p,s);
  }
  
  CF_DoMoAct.ports[p].Enable = s;
  CF_WriteToEEprom();
  return;
}

/**
 * update name of port in EEPROM
 *  @param s : name string
 *  &param p : port to set
 */
void CF_PT_Name(String s, uint8_t p)
{
  int i, j;
  
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  
  for (i = 0, j = 0 ; i < s.length() && i < CF_NAMELEN ; i++) {
    if (s[i] > 0x1f && s[i] != 0xd && s[i] != 0xa)  CF_DoMoAct.ports[p].Name[j++] = s[i];
  }

  // truncate with zero's
  while (j < CF_NAMELEN)  CF_DoMoAct.ports[p].Name[j++] = 0x0;
  CF_WriteToEEprom();
}

/**
 * update GPIO of port in EEPROM
 *  @param g : Gpio to use
 *  &param p : port to set
 */
void CF_PT_Gpio(int g, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].Gpio = g;
  CF_WriteToEEprom();
}

/**
 * update OnHour of port in EEPROM
 *  @param h : Hour to turn on
 *  &param p : port to set
 */
void CF_PT_OnHour(int h, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].On_hour = h;
  CF_WriteToEEprom();
}

/**
 * update onMinute of port in EEPROM
 *  @param m : minutes to turn on
 *  &param p : port to set
 */
void CF_PT_OnMin(int m, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].On_minute = m;
  CF_WriteToEEprom();
}

/**
 * update OffHour of port in EEPROM
 *  @param h : Hour to turn off
 *  &param p : port to set
 */
void CF_PT_OffHour(int h, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].Off_hour = h;
  CF_WriteToEEprom();
}

/**
 * update offminute of port in EEPROM
 * 
 *  @param m : minutes to turn off
 *  &param p : port to set
 */
void CF_PT_OffMin(int m, uint8_t p)
{
  if (p > CF_NUMPORTS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.ports[p].Off_minute = m;
  CF_WriteToEEprom();
}
/////////////////////////////////// GROUP handling /////////////////////////////////////
/**
 *  update a group with ports
 * 
 *  @param g : assign/remove group g
 *  &param p : port to set
 *  @param s : true : add to group / false : remove from group
 *  
 */
bool CF_GR_Ports(int g, uint8_t p, bool s)
{
  int j;
  if (p > CF_NUMPORTS) return false;
  if (g > CF_NUMGROUPS) return false;
  
  CF_ReadEeprom(false);

  if (s) {  // add the port to group
    
    for(j = 0; j < CF_NUMGROUPS;j++) {

      // if already part of this group
      if (CF_DoMoAct.ports[p].Group[j] == g) return true;
    }

    for(j = 0; j < CF_NUMGROUPS;j++) {

      // find open slot another group 
      if (CF_DoMoAct.ports[p].Group[j] == CF_NOTASSIGNED) {
        
        // add to port to group and update group count  
        CF_DoMoAct.ports[p].Group[j] = g;
        CF_DoMoAct.groups[g].count++;
        break;
      }
   }
   // not empty slot.
   if (j == CF_NUMGROUPS) return false;
  }
  else // remove port from any assigned group
  {
    for(j = 0; j < CF_NUMGROUPS; j++) {

      // if part of this group
      if (CF_DoMoAct.ports[p].Group[j] == g) {
       
        // remove from current group  
        if (CF_DoMoAct.groups[CF_DoMoAct.ports[p].Group[j]].count > 0)
          CF_DoMoAct.groups[CF_DoMoAct.ports[p].Group[j]].count--;

        // indicate NOT assigned slot   
        CF_DoMoAct.ports[p].Group[j] = CF_NOTASSIGNED;
      }
    }
  }
   
  CF_WriteToEEprom();

  return true;
}

/**
 * update name of group in EEPROM
 *  @param s : name string
 *  &param g : Group to set
 */
void CF_GR_Name(String s, uint8_t g)
{
  int i,j;
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  for (i = 0, j = 0 ; i < s.length() && i < CF_NAMELEN ; i++) {
    if (s[i] > 0x1f && s[i] != 0xd && s[i] != 0xa)  CF_DoMoAct.groups[g].Name[j++] = s[i];
  }
  // truncat with zero's
  while (j < CF_NAMELEN)  CF_DoMoAct.groups[g].Name[j++] = 0x0;
  CF_WriteToEEprom();
}

/**
 * enable / disable day to trigger on or off
 *  @param e : true: enable day / false : disable day
 *  @param d : which day (sunday = 0)
 *  &param g : group to set
 */
void CF_GR_SetDay(bool e, uint8_t d, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  
  CF_ReadEeprom(false);
  
  if (e) CF_DoMoAct.groups[g].Day |= 1 << d;
  else CF_DoMoAct.groups[g].Day &= ~(1 << d);
 
  CF_WriteToEEprom(); 
}

/**
 *  set / reset Off_sunrise of group in EEPROM
 *  
 *  @param s : true: use off_minutes to switch OFF AFTER before sunrise /  false: use Off_hour / Off_minute to switch off
 *  &param p : port to set
 */
void CF_GR_Offsunrise(bool s, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].Off_sunrise = s;
  CF_WriteToEEprom();
}

/**
 *  set / reset On_sunset of group in EEPROM
 *  
 *  @param s : true: use on_minutes to switch ON BEFORE before sunset / false: use On_hour/ On_minute to switch on
 *  &param p : port to set
 */
void CF_GR_Onsunset(bool s, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].On_sunset = s;
  CF_WriteToEEprom();
}

/**
 *  set/  reset Enable of group in EEPROM
 *  @param s : true: enable group / false : disable
 *  &param g : group to set
 */
void CF_GR_Enable(bool s, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  // if disable group, switch OFF if ON
  if (!s && GroupStatus[g]){
    SwitchPorts(g, s);
  }
  CF_DoMoAct.groups[g].Enable = s;
  CF_WriteToEEprom();
  return;
}

/**
 * update OnHour of GROUP in EEPROM
 *  @param h : Hour to turn on
 *  &param p : port to set
 */
void CF_GR_OnHour(int h, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].On_hour = h;
  CF_WriteToEEprom();
}

/**
 * update onMinute of group in EEPROM
 *  @param m : minutes to turn on
 *  &param p : port to set
 */
void CF_GR_OnMin(int m, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].On_minute = m;
  CF_WriteToEEprom();
}

/**
 * update OffHour of group in EEPROM
 *  @param h : Hour to turn off
 *  &param p : port to set
 */
void CF_GR_OffHour(int h, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].Off_hour = h;
  CF_WriteToEEprom();
}

/**
 * update offminute of group in EEPROM
 * 
 *  @param m : minutes to turn off
 *  &param p : port to set
 */
void CF_GR_OffMin(int m, uint8_t g)
{
  if (g > CF_NUMGROUPS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.groups[g].Off_minute = m;
  CF_WriteToEEprom();
}

/////////////////////// Sensor channel //////////////////
/**
 *  update ports on a sensor channel
 * 
 *  @param g : sensor number
 *  &param p : port to set
 *  @param s : true : add to sensor / false : remove from sensor
 *  
 */
bool CF_SN_Ports(int g, uint8_t p, bool s)
{
  int j;
  if (p > CF_NUMPORTS) return false;
  if (g > CF_NUMSENSORS) return false;
  
  CF_ReadEeprom(false);

  if (s) {  // add the port to sensor
    
    for(j = 0; j < CF_NUMPORTS;j++) {

      // if already part of this group
      if (CF_DoMoAct.sensors[g].Ports[j] == p) return true;
    }

    for(j = 0; j < CF_NUMPORTS;j++) {

      // find open slot
      if (CF_DoMoAct.sensors[g].Ports[j] == CF_NOTASSIGNED) {
        
        // add to port to group and update group count  
        CF_DoMoAct.sensors[g].Ports[j] = p;
        break;
      }
    }
    // no slot found
    if (j == CF_NUMPORTS)     return false;
  }
  
  else // remove port from any assigned group
  {
    for(j = 0; j < CF_NUMPORTS; j++) {

      // if part of this group
      if (CF_DoMoAct.sensors[g].Ports[j] == p) {
            
        // indicate NOT assigned slot   
        CF_DoMoAct.sensors[g].Ports[j] = CF_NOTASSIGNED;
      }
    }
  }
   
  CF_WriteToEEprom();

  return true;
}

/**
 *  set/ reset Enable of senso in EEPROM
 *  @param s : true: enable sensor / false : disable
 *  &param g : sensors to set
 */
void CF_SN_Enable( bool s, uint8_t g)
{
  if (g > CF_NUMSENSORS) return;
  CF_ReadEeprom(false);
  CF_DoMoAct.sensors[g].Enable = s;
  CF_WriteToEEprom();
  return;
}

/**
 * update name of group in EEPROM
 *  @param s : name string
 *  &param g : sensor to set
 */
void CF_SN_Name(String s, uint8_t g)
{
  int i,j;
  if (g > CF_NUMSENSORS) return;
  CF_ReadEeprom(false);
  for (i = 0, j = 0 ; i < s.length() && i < CF_NAMELEN ; i++) {
    if (s[i] > 0x1f && s[i] != 0xd && s[i] != 0xa)  CF_DoMoAct.sensors[g].Name[j++] = s[i];
  }
  
  while (j < CF_NAMELEN)  CF_DoMoAct.sensors[g].Name[j++] = 0x0;
  CF_WriteToEEprom();
}

/**
 * enable / disable day to trigger on or off
 *  @param e : true: enable day / false : disable day
 *  @param d : which day (sunday = 0)
 *  &param g : sensor to set
 */
void CF_SN_SetDay(bool e, uint8_t d, uint8_t g)
{
  if (g > CF_NUMSENSORS) return;
  
  CF_ReadEeprom(false);
  
  if (e) CF_DoMoAct.sensors[g].Day |= 1 << d;
  else CF_DoMoAct.sensors[g].Day &= ~(1 << d);
 
  CF_WriteToEEprom(); 
}

/////////////////////// Import export functions //////////////////
/**
 * export data to screen.
 * 
 * This can be captured / save on computer for later import
 */
void CF_Export()
{
  CF_CS = 0;    // reset check sum

  EEPROM.get(eeAddress, data.domo);

  // set first line header
  uint8_t xx = 1;
  Serial.print(":");
  Serial.print(xx++);
  Serial.print(":");
  
  Serial.print(CF_HEADER, HEX);

  // add total length
  uint8_t h = sizeof(data.domo) >> 8 & 0xff;
  if (h < 0x10) Serial.print("0");
  Serial.print(h, HEX);  // msb
  
  h = sizeof(data.domo) & 0xff;
  if (h < 0x10) Serial.print("0");
  Serial.print(h, HEX); // lsb

  // add length of line
  if (LINELENGTH < 0x10) Serial.print("0");
  Serial.print(LINELENGTH, HEX); 

  // add data
  for (int16_t i = 0; i < sizeof(data.domo); i++) {

    if(i % LINELENGTH == 0 && i > 0) {
     
      Serial.print("\n\n:");
      Serial.print(xx++);
      Serial.print(":");
      
      //add length of new line
      if (sizeof(data.domo) - i > LINELENGTH)  h = LINELENGTH; 
      else h = sizeof(data.domo) - i;
      if (h < 0x10) Serial.print("0"); 
      Serial.print(h, HEX); 
    }
    
    if ( (data.d[i] & 0xff) < 0x10) Serial.print("0");
    Serial.print( (data.d[i] & 0xff), HEX);
    CF_CS = update_crc_8(CF_CS, data.d[i]);
    
  }
  
  // add check sum
  if (CF_CS < 0x10) Serial.print("0");
  Serial.println(CF_CS,HEX);
      
  Serial.println("\nCopy & Paste the lines above to a file so you can use it later to restore\n");
}

/**
 * This will import a line that previous exported to serial monitor
 * @param x : Line serial number
 * 
 * As the expert happens with multiple lines, it will check that the correct line header :x:
 * Return:
 * -1 = error or timeout
 * else length
 */
int16_t Getline(uint8_t x)
{
  uint16_t i = 0;
  int WaitCount = 0;

  Serial.print("Provide line :");
  Serial.print(x);
  Serial.print(":. Maximum wait is ");
  Serial.print(CF_IMPORTDELAY / 2);
  Serial.println(" seconds.");
  
  while (!Serial.available()) {
    
    if ( WaitCount++ < CF_IMPORTDELAY) delay(500);   // wait max CF_IMPORTDELAY * 500ms => 20 seconds
    else {
      Serial.println("Timeout. Cancel Import");
      return -1;
    }
  }
  
  // get all the data
  while(Serial.available()) {
    
    char c = Serial.read();
    
    if (i == 0 || i == 2) 
    {
      if (c != ':') {
        Serial.println("Invalid line header. Format must be :x:");
        return -1;
      }
    }
    else if (i == 1) {
      if (c -'0' != x) {
        Serial.print("Invalid serial number in header. Expected ");
        Serial.print(x);
        Serial.print(" got ");
        Serial.println(c);
        return -1;
      }
    }
    else {
      BackUp[i] = (char) c; // store all received content in buffer
    }
    
    i++;                    // next buffer entry
    delay(1);               // give time to get next character (USB works unbuffered !)
  }
  
  if (i > 0 ) i--;          // if anything received remove the last pointer increase
  return (i);               // return size of line read
}

/**
 * Parse received line
 * 
 * @param ll: line Length 
 * @param x : line serial number
 * 
 * return:
 * -1 line to short
 * -2 checksum error
 *  1 continue with next line
 *  2 checksom OK, completed
 *  
 *  FoundHeader only used on line serial 1.
 *  0 start position
 *  1 one 'E' detected
 *  2 second 'E' detected
 *  3 got MSB of length
 *  4 got LSB of length and header is complete
 */
int16_t Storeline(int16_t ll, uint8_t x)
{
  int  jp = 0;
  char p[2];
  int16_t i;
  uint16_t LineLength = 0;
  
  // skip line header :x:
  for (i = 3 ; i < ll; i++)
  {
    if (x == 1 && FoundHeader < 4)
    { 
      // :x:EEXXYYZZUU EE = Header  XX = Total length MSB YY = Total length LSB ZZ= line length MSB UU= linelength LSB
      if (ll < 11) {
        Serial.println("Not enough characters for first line.");
        return -1;
      }

      // skip everything up to header
      // assume for now that EE are next to each other
      if (BackUp[i] == 'E' && FoundHeader != 2) {
        FoundHeader++;
        CF_CS = 0;
        DomoLength = 0;
        Data_p = 0;   // start position to update union
        continue;
      }

      // ascii to hex
      p[jp++] = BackUp[i];
      if (jp != 2)  continue;   // get both nibbles from a byte for the length
      uint8_t b = Hto8(p);
      jp = 0;
        
      // get total length MSB
      if (FoundHeader == 2) {
        DomoLength = b << 8; //MSB
        FoundHeader++;
        continue;
      }
      // add total length LSB
      if (FoundHeader == 3) {
        DomoLength = DomoLength  | b; //LSB
        FoundHeader++;
        continue;
      }
    }

    // get length of data in current line
    if (LineLength == 0) {
      p[jp++] = BackUp[i];
      if (jp != 2)  continue;
      LineLength = Hto8(p) * 2;
      jp = 0; 
      break;   
    }
  }  
 
  i++;
  LineLength += i;  

  // read, translate and save in union all characters 
  for ( ; i < LineLength && Data_p < DomoLength; i++) {
      
    // ascii to hex
    p[jp++] = BackUp[i];
    if (jp != 2)  continue;
    uint8_t db = Hto8(p);
    jp = 0;

    data.d[Data_p++] = db;  // store the data  
    CF_CS = update_crc_8(CF_CS, db); // calculate checksum
  }

  // all data has been copied, get and check CRC
  if (Data_p == DomoLength)
  {
    // ascii to hex
    p[jp++] = BackUp[i++];
    p[jp] = BackUp[i];
    uint8_t cs = Hto8(p);
    jp = 0;

    if (CF_CS != cs){
      Serial.println("Checksum error. cancel");
      return -2;
    }
    else {
      Serial.println("Transfer completed correctly.");
      return 2;
    }
  }

  return 1; // continue with next line
}

/**
 * Import earlier exported EEPROM content
 */
void CF_Import()
{
  uint8_t x = 1;
  FoundHeader = 0;
  Serial.println("\nRestore EEPROM from  PC.");
  Serial.println("Copy the previous obtained export data and press <enter>.");
  
  while (1)
  {
    int16_t pl = Getline(x);            // read line from serial and check on line serial number

    if (pl == -1) return;               // time out or wrong Line serial number
  
    int8_t st = Storeline(pl, x++);

    if (st < 0) return;   // error

    if (st == 2) break;   // done
  }

  // save in EEPROM
  if (data.domo.Version != (float) CF_VERSION ){
    Serial.println("Invalid version. NOT saved");
    return; 
  }

  // update EEPROM
  EEPROM.put(eeAddress, data.domo);

  Serial.println("Backup restored.");
  
  // read and display
  CF_ReadEeprom(true);
}

/**
 * translate ascii-HEX input to uin8_t 
 */
uint8_t Hto8(char *p)
{
 uint8_t x = 0;
 
 for(int i = 0; i < 2; i++) {
   char c = *p;

   if (c >= '0' && c <= '9') {
      x *= 16;
      x += c - '0';
   }
   else if (c >= 'A' && c <= 'F') {
      x *= 16;
      x += (c - 'A') + 10;
   }
   else if (c >= 'a' && c <= 'f') {
      x *= 16;
      x += (c - 'a') + 10;
   }
   else break;
   p++;
 }
 return x;
}

/////////////////////////////// CRC calculation /////////////////////////

/*
 * static uint8_t sht75_crc_table[];
 *
 * The lookup table crc_table[] is used to recalculate the CRC. 
 */

static uint8_t sht75_crc_table[] = {

  0,   49,  98,  83,  196, 245, 166, 151, 185, 136, 219, 234, 125, 76,  31,  46,
  67,  114, 33,  16,  135, 182, 229, 212, 250, 203, 152, 169, 62,  15,  92,  109,
  134, 183, 228, 213, 66,  115, 32,  17,  63,  14,  93,  108, 251, 202, 153, 168,
  197, 244, 167, 150, 1,   48,  99,  82,  124, 77,  30,  47,  184, 137, 218, 235,
  61,  12,  95,  110, 249, 200, 155, 170, 132, 181, 230, 215, 64,  113, 34,  19,
  126, 79,  28,  45,  186, 139, 216, 233, 199, 246, 165, 148, 3,   50,  97,  80,
  187, 138, 217, 232, 127, 78,  29,  44,  2,   51,  96,  81,  198, 247, 164, 149,
  248, 201, 154, 171, 60,  13,  94,  111, 65,  112, 35,  18,  133, 180, 231, 214,
  122, 75,  24,  41,  190, 143, 220, 237, 195, 242, 161, 144, 7,   54,  101, 84,
  57,  8,   91,  106, 253, 204, 159, 174, 128, 177, 226, 211, 68,  117, 38,  23,
  252, 205, 158, 175, 56,  9,   90,  107, 69,  116, 39,  22,  129, 176, 227, 210,
  191, 142, 221, 236, 123, 74,  25,  40,  6,   55,  100, 85,  194, 243, 160, 145,
  71,  118, 37,  20,  131, 178, 225, 208, 254, 207, 156, 173, 58,  11,  88,  105,
  4,   53,  102, 87,  192, 241, 162, 147, 189, 140, 223, 238, 121, 72,  27,  42,
  193, 240, 163, 146, 5,   52,  103, 86,  120, 73,  26,  43,  188, 141, 222, 239,
  130, 179, 224, 209, 70,  119, 36,  21,  59,  10,  89,  104, 255, 206, 157, 172
};

/*
 * uint8_t update_crc_8( unsigned char crc, unsigned char val );
 *
 * Given a databyte and the previous value of the CRC value, the function
 * update_crc_8() calculates and returns the new actual CRC value of the data
 * comming in.
 */

uint8_t update_crc_8( unsigned char crc, unsigned char val ) {

  return sht75_crc_table[val ^ crc];

}  /* update_crc_8 */
