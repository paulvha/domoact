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

// global used parameter instead of defining in each subroutine to save space
int v,p,g,j;
bool s;
String n;

// automatically display configuration (set in menu)
bool DispPort = false;
bool DispGroup = false;
bool DispSensor = false;

// Enable for the selective display of port, group or sensor that a '*' can be entered
// to display ALL ports, groups or sensors
bool AllowAll = false;
#define STRENTER  99      // '*' was entered to indicate ALL

// Normally the checking for switching on/off, WebClient handling sensors is disabled to prevent to many message
// BUT it can be enabled in the main menu. It will increase the response time a 'little.'
bool CheckDuringMenu = false;

// When login username and password are provided, you need to login to the menu everytime you logged
// out. During configuring the system this can be annoying. So you toggle it off / on in the 
// system maintenance menu. This setting, for security reason, is not remembered after reboot.
bool AskMenuLogin = true;

// helper for dashboard
int rem[CF_NUMDASHBOARD] = {0};

// make nicer header
String header = " =========== ";

/**
 * entry point for the Serial menu
 */
void StartMenu()
{
  bool ret = true;
  Serial.setTimeout(INPUTDELAY * 1000); // set input timeout
  MessageLCD("Menu active",M_NO);
  
  // if DoMoAct login provided and not disabled in the system menu.
  if (AskLogin && AskMenuLogin) ret = LoginMenu();

  if (ret) MainMenu();
   
  Serial.println();
  Serial.print(header);
  Serial.print("bye bye exit menu");
  Serial.println(header);
  
  // clear LCD
  MessageLCD("",M_NO);
}

/**
 * try to validate username and password
 * return
 *   true : succeeded
 *   false: Invalid login
 */
bool LoginMenu()
{
  bool InValidLogin = true;

  while (InValidLogin){
    
    Serial.print(header);
    Serial.print(" DoMoAct Login ");
    Serial.println(header);
    
    while (Serial.available()) Serial.read();
    Serial.print("\nProvide the User name :");
    n = Serial.readStringUntil(0x0d);
    
    // time out or only enter
    if (n.length() < 2) return(false);
    Serial.println(n);

    // clear pending input
    while (Serial.available()) Serial.read();
    
    Serial.print("\nProvide the Password : ");
    String m = Serial.readStringUntil(0x0d);
    
    // time out or only enter
    if (m.length() < 2) return(false);

    // print how many characters in password
    for (j = 0 ; j < m.length() ; j++) Serial.print("*");
    Serial.println("\n");
    
    // check user name content
    for (j = 0 ; j < n.length() ; j++)
      if (Acc_Name[j] != n[j]) break;
    
    // check length
    if (j == strlen(Acc_Name)) InValidLogin = false;
    
    // check password only if name check was success       
    if (!InValidLogin) {
      
      for (j = 0 ; j < m.length() ; j++)
        if (Acc_Pswd[j] != m[j]) break;
      
      //if passwd length is incorrect 
      if (j != strlen(Acc_Pswd)) InValidLogin = true;
    }

    if (InValidLogin) Serial.println("Invalid Login provided. Retry\n");
    else break;
      
  }
  return(true);
}

void MainMenu()
{
  while(1) {
    Serial.print(header);
    Serial.print("MAIN menu");
    Serial.println(header);
    Serial.println("\n1\tSystem configuration");
    Serial.println("2\tSystem maintenance");
    Serial.println("3\tManual switch");
    Serial.println("4\tToggle checking for WebClient & switch during menu");
    Serial.println("5\tDashboard");
#ifdef USERSENSORMENU == 1
    Serial.println("6\tSensor menu");
#endif    
    Serial.println("9\tDone");
    v = GetInput(9);
    if (v < 0 || v == 9) break; 
    switch(v) {
      case 1:
        SysSetupMain();
        break;
      case 2:
        SysMaintenance();
        break;
      case 3:
        ManualSwitch();
        break;
      case 4:
        CheckDuringMenu = ! CheckDuringMenu;
        if (CheckDuringMenu) Serial.println("Checking enabled.  Menu reaction time is slower.");
        else Serial.println("Checking disabled.");
        break;
      case 5:
        SysDashboard();
        break;
        
#if USERSENSORMENU == 1
      case 6:         // sensor menu has been defined, part of Sensor_example-tab 
        SensorMenu();
        break;
#endif
      default:
        break;
    }
    Serial.println();
  }
}

//////////////////////////////////////////////////////////
//  DASHBOARD functions
/////////////////////////////////////////////////////////
/**
 * handle dashboard 
 */
void SysDashboard()
{
  int pgs, num, pointer;

  while(1)
  {
    Serial.print(header);
    Serial.print("Dashboard");
    Serial.println(header);
    Serial.println();
    
    pointer = ShowDashboard();
    
    if (pointer == 0) {   // empty, check to add
      ConfigDashBoard();
      return;
    }
    
    Serial.print(pointer);
    Serial.println("\tDone");
      
    Serial.print("Select entry");
    v = GetInput(pointer); 
    if(v < 0 || v == pointer) return;

    pgs = CF_DoMoAct.DashBoard[rem[v]].PGS;
    num = CF_DoMoAct.DashBoard[rem[v]].num;
    
    // do switching
    if (pgs == PORTS)  j = SetPort(num,!PortStatus[num]);
    else if (pgs == GROUP) j = SwitchPorts(num,!GroupStatus[num]);
    else if (pgs == SENSOR) {
      SensorStatus[num] = ! SensorStatus[num];
      j = SwitchSensor(num);
    }
    
    if (j < 0) Serial.println("NOT enabled");
  }
}

/**
 * show the information about the ports, groups or sensors that 
 * have been assigned to the dashboard
 * 
 * return :
 * number of entries on the dashboard
 */
int ShowDashboard()
{
  int pgs, num;
  int pointer = 0;
  
  for (j = 0; j < CF_NUMDASHBOARD; j++) {
  
    if (CF_DoMoAct.DashBoard[j].PGS == CF_NOTASSIGNED) continue;  // not used entry
    
    pgs = CF_DoMoAct.DashBoard[j].PGS;
    num = CF_DoMoAct.DashBoard[j].num;
    
    Serial.print(pointer);
    rem[pointer++] = j;           // remember location in dashboard structure
   
    if (pgs == PORTS) {
      Serial.print("\tPort\t");
      Serial.print(num);
      Serial.print("\t");
      Serial.print(CF_DoMoAct.ports[num].Name);
      if (CF_DoMoAct.ports[num].Enable) {
        Serial.print("\tEnabled");
        
        if (PortStatus[num]) Serial.println("\tON");
        else Serial.println("\tOFF");
      }
      else
        Serial.println("\tDisabled");
    }
    
    if (pgs == GROUP) {
      
      Serial.print("\tGroup\t");
      Serial.print(num);
      Serial.print("\t");
      Serial.print(CF_DoMoAct.groups[j].Name);
      if (CF_DoMoAct.groups[j].Enable) {
        Serial.print("\tEnabled");
        
        if (GroupStatus[num]) Serial.println("\tON");
        else Serial.println("\tOFF");
      }
      else
        Serial.println("\tDisabled");
    }

    if (CF_DoMoAct.DashBoard[j].PGS == SENSOR) {
      
      Serial.print("\tSensor\t");
      Serial.print(num);
      Serial.print("\t");
      Serial.print(CF_DoMoAct.sensors[num].Name);
      if (CF_DoMoAct.sensors[num].Enable) {
        Serial.print("\tEnabled");
        
        if (SensorStatus[num]) Serial.println("\tON");
        else Serial.println("\tOFF");
      }
      else
        Serial.println("\tDisabled");
    }
  }
  
  return(pointer);
}


/**
 * add / remove ports, groups or sensors to the dashboard
 */
void ConfigDashBoard() {
  int pgs = CF_NOTASSIGNED;
  int num = 0;
  int Sel;
  
  while(1) { 
    
    Serial.print(header);
    Serial.print("current Dashboard"); 
    Serial.println(header);

    int pointer = ShowDashboard();
  
    if (pointer == 0){
      Serial.println("Dashboard is empty");
      Serial.print("\nDo you want to 2 = add or 3 = cancel");
    }
    else
      Serial.print("\nDo you want to 1 = remove, 2 = add or 3 = cancel");

    v = GetInput(3); 
    if (v < 0 || v == 3) return;
  
    if (v == 1) {     // remove
      
      if (pointer == 0 ) {
        Serial.println("Nothing to remove.");
        continue;
      }
    
      Serial.print("Which entry number do you want to remove ? (");
      Serial.print(pointer);
      Serial.print(" = cancel)");
      v = GetInput(pointer); 
      if (v < 0 || v == pointer) return;
  
      CF_SetDashboard(CF_DoMoAct.DashBoard[rem[v]].PGS, CF_DoMoAct.DashBoard[rem[v]].num, false);
      Serial.println("Removed");
      continue;
    }  
  
    if (v == 2) {
      Serial.print("Do you want to add 1 = Port, 2 = Group or 3 = Sensor. 4 = Cancel");
      v = GetInput(4); 
      if (v < 0 || v == 4) return;

      if (v == 1 ) {    // port
        pgs = PORTS;
        Sel = CF_NUMPORTS;
        for (j = 0; j < CF_NUMPORTS;j++) {
          Serial.print(j);
          Serial.print("\tPort\t");
          Serial.print(CF_DoMoAct.ports[j].Name);
          if (CF_DoMoAct.ports[j].Enable) {
            Serial.print("\tEnabled");
            
            if (PortStatus[j]) Serial.println("\tON");
            else Serial.println("\tOFF");
          }
          else
            Serial.println("\tDisabled");
        }
      }
        
      else if (v == 2) {  // group
        pgs = GROUP;
        Sel = CF_NUMGROUPS;
        for (j = 0; j < CF_NUMGROUPS;j++) {
          Serial.print(j);
          Serial.print("\tGroup\t");
          Serial.print(CF_DoMoAct.groups[j].Name);
          if (CF_DoMoAct.groups[j].Enable) {
            Serial.print("\tEnabled");
            
            if (GroupStatus[j]) Serial.println("\tON");
            else Serial.println("\tOFF");
          }
          else
            Serial.println("\tDisabled");
        }
      }
      else if (v == 3) {  // sensor
        pgs = SENSOR;
        Sel = CF_NUMSENSORS;
        for (j = 0; j < CF_NUMSENSORS;j++) {
          Serial.print(j);
          Serial.print("\tSensor\t");
          Serial.print(CF_DoMoAct.sensors[j].Name);
          if (CF_DoMoAct.sensors[j].Enable) {
            Serial.print("\tEnabled");
            
            if (SensorStatus[j]) Serial.println("\tON");
            else Serial.println("\tOFF");
          }
          else
            Serial.println("\tDisabled");
        }
      }
      
      Serial.print("Which entry to add ? (");
      Serial.print(Sel);
      Serial.print(" = cancel)");
      v = GetInput(Sel); 
      if (v < 0 || v == Sel) return;
      
      if (CF_SetDashboard(pgs, v, true)) {
        Serial.println("added"); 
      }
      else
        Serial.println("Error: could not add."); 
    }
  }
}

/**
 * set Web menu background
 */
void ConfigBackground()
{
  Serial.print(header);
  Serial.print("Config Web Background"); 
  Serial.println(header);
  Serial.println("\n1\tStarfield.");
  Serial.println("2\tScrolling sand.");
  Serial.println("3\tSliding Diagonals.");
  Serial.println("4\tPure CSS Gradient.");
  Serial.println("5\tGradient waves.");
  Serial.println("6\tFireworks.");
  Serial.println("7\tStars.");
  Serial.println("8\tDone");
  v = GetInput(8);
  if (v < 0 || v == 8 || v == 0) return;
  CF_Set_WebBackGround(v);
}


//////////////////////////////////////////////////////////
//  SETUP
/////////////////////////////////////////////////////////
void SysSetupMain()
{
  while(1){
    Serial.print(header);
    Serial.print("System configure menu");
    Serial.println(header);
    Serial.println("\n1\tDisplay current configuration");
    Serial.println("2\tPort configuring");
    Serial.println("3\tGroup configuring");
    Serial.println("4\tSensor channel configuring");
    Serial.println("5\tForce / set initial setup");
    Serial.println("6\tEnable displaying switching");
    Serial.println("7\tDisable displaying switching");
    Serial.println("8\tDone");
    v = GetInput(8); 
    if(v < 0 || v == 8) break;
    
    switch(v) {
      case 1:
        CF_ReadConf(true);
        break;
      case 2:
        SysSetupPort();
        break;
      case 3:
        SysSetupGroup();
        break;
      case 4:
        SysSetupSensor();
        break;
      case 5:
        SysSetupInit();
        break;
      case 6:
        CF_SetDisplaySwitch(true);
        break;
      case 7:
        CF_SetDisplaySwitch(false);
        break;
      default:
        break;
    }
    Serial.println();
  }
}

void SysSetupPort()
{
  while(1) {
    Serial.print(header);
    Serial.print("Port configuring menu");
    Serial.println(header);
    Serial.println("\n1\tDisplay configuration of a port");  
    Serial.println("2\tSet port GPIO");
    Serial.println("3\tToggle switch OnSunset");
    Serial.println("4\tSet OnMinutes");
    Serial.println("5\tSet OnHour");
    Serial.println("6\tToggle switch OffSunrise");
    Serial.println("7\tSet OffMinutes");
    Serial.println("8\tSet OffHour");
    Serial.println("9\tToggle Active level");
    Serial.println("10\tAdd a port to a group");
    Serial.println("11\tRemove a port from a group");
    Serial.println("12\tEnable a port");
    Serial.println("13\tDisable a port");
    Serial.println("14\tUpdate port name");
    Serial.println("15\tSet Working days");
    Serial.println("16\tToggle display updated configuration");
    Serial.println("17\tClear time setting");
    Serial.println("18\tDisplay settings");
    Serial.println("19\tDone");
    v = GetInput(19); 
    if (v < 0  || v == 19) break;
    
    // get port
    if (v == 1) AllowAll = true;
    p = GetPortGroupSensor(PORTS);
    if (p < 0) return;
  
    switch(v) {
      case 1:
        if (AllowAll && p == STRENTER) {
          for (j =0; j < CF_NUMPORTS ; j++)  DispPortInfo(j,false);
        } else  DispPortInfo(p,false);
        AllowAll = false;
        break;
        
      case 2:
        Serial.print("Provide GPIO number. Max.");
        Serial.print(NUM_DIGITAL_PINS);
        v = GetInput(NUM_DIGITAL_PINS);  
        if (v < 0) continue;
        CF_PT_GPIO(v, p);
        break;
        
      case 3:
        s = !CF_DoMoAct.ports[p].On_sunset;
        CF_PT_Onsunset(s, p);
        Serial.print("Sunset is set ");
        if (s) Serial.println("ON");
        else {
          Serial.println("OFF");

          if (CF_DoMoAct.ports[p].Enable) {
            if ( ! PortValidEnable(p)) Serial.println("Port disabled");
          }
        }  
        break;
        
      case 4:
        Serial.print("Provide OnMinutes number. Max.");
        Serial.print(59);
        v = GetInput(59);  
        if (v < 0) continue;
        CF_PT_OnMin(v, p);
        break;
        
      case 5:
        Serial.print("Provide OnHour number. Max.");
        Serial.print(23);
        v = GetInput(23);  
        if (v < 0) continue;
        CF_PT_OnHour(v, p);
        break;
        
      case 6:
        s = !CF_DoMoAct.ports[p].Off_sunrise;
        CF_PT_Offsunrise(s, p);
        Serial.print("Sunrise is set ");
        if (s) Serial.println("ON");
        else {
          Serial.println("OFF");
          
          if (CF_DoMoAct.ports[p].Enable) {
            if ( ! PortValidEnable(p)) Serial.println("Port disabled");
          }
        }
        break;
      
      case 7:
        Serial.print("Provide OffMinutes number. Max.");
        Serial.print(59);
        v = GetInput(59);  
        if (v < 0) continue;
        CF_PT_OffMin(v, p);
        break;
      
      case 8:
        Serial.print("Provide OffHour number. Max.");
        Serial.print(23);
        v = GetInput(23);  
        if (v < 0) continue;
        CF_PT_OffHour(v, p);
        break;
        
      case 9:
        s = !CF_DoMoAct.ports[p].ActiveHigh;
        CF_PT_ActiveLevel(s, p);
        Serial.print("Active level is set to ");
        if (s) Serial.println("HIGH");
        else Serial.println("LOW");
        break;
      
      case 10:  // Add a port to a group
        v = GetPortGroupSensor(GROUP);
        if (v < 0) continue;
        if (! CF_GR_Ports(v, p, true))
          Serial.println("Error during adding");
        else {
          Serial.print("Port: ");
          Serial.print(p);
          Serial.print(" added to group: ");
          Serial.println(v);
        }
        break;
      
      case 11:  // remove from a group
        v = GetPortGroupSensor(GROUP);
        if (v < 0) continue;
        if (! CF_GR_Ports(v, p, false))
          Serial.println("Error during removing");
        else {
          Serial.print("Port: ");
          Serial.print(p);
          Serial.print(" removed from group: ");
          Serial.println(v);
        }
        break;
        
      case 12:    // enable
      
        if (! PortValidEnable(p)){
          Serial.println("Could not enable port. Check GPIO !");
          continue;
        }
        Serial.print("Port: ");
        Serial.print(p);
        Serial.println(" is enabled.");
        break;
        
      case 13:
        CF_PT_Enable(false, p);
        Serial.print("Port: ");
        Serial.print(p);
        Serial.println(" is disabled.");
        break;
      
      case 14:        // Update port name
        UpdateName(p, PORTS);
        break;
      
      case 15:       // set working days
        HandleWorkDays(PORTS, p);
        break;   

      case 16:
        DispPort = !DispPort;
        break;

      case 17: //Clear time setting
        Serial.print("All time related parameter will be reset for port ");
        Serial.println(p);
        if (!AreYouSure()) break;
        CF_DoMoAct.ports[p].On_sunset = false;
        CF_DoMoAct.ports[p].On_minute = CF_NOTASSIGNED;
        CF_DoMoAct.ports[p].On_hour = CF_NOTASSIGNED;
    
        CF_DoMoAct.ports[p].Off_sunrise = false;    
        CF_DoMoAct.ports[p].Off_minute = CF_NOTASSIGNED;
        CF_DoMoAct.ports[p].Off_hour = CF_NOTASSIGNED;
        
        // update configuration
        CF_WriteConf();
        
        // disable port if not part of a group or sensor
        if (!CheckForGroupOrSensor(p)) CF_PT_Enable(false, p);
        break;
        
      case 18:    // health information
        GPShealth(p, PORTS);
        break; 
        
      default:
        break;
    }
    if (DispPort) DispPortInfo(p,false);
    Serial.println();
  }
}
 
void SysSetupGroup()
{
  while(1) {
    Serial.print(header);
    Serial.print("Group configuring menu");
    Serial.println(header);
    Serial.println("\n1\tDisplay configuration of a group");  
    Serial.println("2\tToggle switch OnSunset");
    Serial.println("3\tSet OnMinutes");
    Serial.println("4\tSet OnHour");
    Serial.println("5\tToggle switch OffSunrise");
    Serial.println("6\tSet OffMinutes");
    Serial.println("7\tSet OffHour");
    Serial.println("8\tAdd a port to a group");
    Serial.println("9\tRemove a port from a group");
    Serial.println("10\tEnable a group");
    Serial.println("11\tDisable a group");
    Serial.println("12\tUpdate group name");
    Serial.println("13\tSet Working days");
    Serial.println("14\tToggle display updated configuration");
    Serial.println("15\tClear time settings");
    Serial.println("16\tDisplay settings");
    Serial.println("17\tDone");
  
    v = GetInput(17); 
    if (v < 0 || v == 17) break;
    
    // get group
    if (v == 1)  AllowAll = true;
    g = GetPortGroupSensor(GROUP);
    if (g < 0) break;
  
    switch(v) {
      case 1:
        if (AllowAll && g == STRENTER) {
          for (j = 0; j < CF_NUMGROUPS ; j++)  DispGroupInfo(j,false);
        } else  DispGroupInfo(g,false);
        AllowAll = false;
        break;
        
      case 2: //Enable/disable OnSunset
        s = !CF_DoMoAct.groups[g].On_sunset;
        CF_GR_Onsunset(s, g);
        Serial.print("Sunset is set ");
        if (s) Serial.println("ON");
        else {
          Serial.println("OFF");

          if (CF_DoMoAct.groups[g].Enable){
            if (CF_DoMoAct.groups[g].On_minute == CF_NOTASSIGNED || CF_DoMoAct.groups[g].On_hour == CF_NOTASSIGNED){
              Serial.print("WARNING: No time to switch On is set !! ");
            }
          }
        }  
        break;
        
      case 3: //Set OnMinutes
        Serial.print("Provide OnMinutes number. Max.");
        Serial.print(59);
        v = GetInput(59);  
        if (v < 0) continue;
        CF_GR_OnMin(v, g);
        break;
        
      case 4: //Set OnHour
        Serial.print("Provide OnHour number. Max.");
        Serial.print(23);
        v = GetInput(23);  
        if (v < 0) continue;
        CF_GR_OnHour(v, g);
        break;
        
      case 5: //Switch OffSunrise
        s = !CF_DoMoAct.groups[g].Off_sunrise;
        CF_GR_Offsunrise(s, g);
        Serial.print("Sunrise is set ");
        if (s) Serial.println("ON");
        else {
          Serial.println("OFF");

          if (CF_DoMoAct.groups[g].Enable){
            if (CF_DoMoAct.groups[g].Off_minute != CF_NOTASSIGNED || CF_DoMoAct.groups[g].Off_hour != CF_NOTASSIGNED){
              Serial.print("WARNING : No time to switch off is set !!");
            }
          }
        }
        break;
      
      case 6: //Set OffMinutes
        Serial.print("Provide OffMinutes number. Max.");
        Serial.print(59);
        v = GetInput(59);  
        if (v < 0) continue;
        CF_GR_OffMin(v, g);
        break;
        
      case 7: //Set Offhour
        Serial.print("Provide OffHour number. Max.");
        Serial.print(23);
        v = GetInput(23);  
        if (v < 0) continue;
        CF_GR_OffHour(v, g);
        break;
      
      case 8: //Add a port to a group
        v = GetPortGroupSensor(PORTS);
        if (v < 0) continue;
        if (! CF_GR_Ports(g, v, true))
          Serial.println("Error during adding");
        else {
          Serial.print("Port: ");
          Serial.print(v);
          Serial.print(" added to group: ");
          Serial.println(g);
        }
        break;

      case 9: //remove a port from a group
        v = GetPortGroupSensor(PORTS);
        if (v < 0) continue;
        if (! CF_GR_Ports(g, v, false))
          Serial.println("Error during removing");
        else {
          Serial.print("Port: ");
          Serial.print(v);
          Serial.print(" removed from group: ");
          Serial.println(g);    
        }
        break;
        
      case 10: //Enable a Group
        CF_GR_Enable(true, g);
        Serial.print("Group: ");
        Serial.print(g);
        Serial.println(" enabled");
        break;

      case 11: //Disable a Group
        CF_GR_Enable(false, g);
        Serial.print("Group: ");
        Serial.print(g);
        Serial.println(" disabled");
        break;
      
      case 12: //Update group name
        UpdateName(g, GROUP);
        break;
      
      case 13: //set working hours
        HandleWorkDays(GROUP, g);
        break;
        
      case 14: //toggle display update
        DispGroup = !DispGroup;
        break;

      case 15:
        Serial.print("All time related parameter will be reset for group ");
        Serial.println(g);
        if (!AreYouSure()) break;
        CF_DoMoAct.groups[g].On_sunset = false;
        CF_DoMoAct.groups[g].On_minute = CF_NOTASSIGNED;
        CF_DoMoAct.groups[g].On_hour = CF_NOTASSIGNED;
    
        CF_DoMoAct.groups[g].Off_sunrise = false;    
        CF_DoMoAct.groups[g].Off_minute = CF_NOTASSIGNED;
        CF_DoMoAct.groups[g].Off_hour = CF_NOTASSIGNED;
                 
        // update configuration
        CF_WriteConf();
        break;
        
      case 16:    // health information
        GPShealth(g, GROUP);
        break; 
        
      default:
        break;
    }
    
    if (DispGroup) DispGroupInfo(g, false);
    Serial.println();
  }
}

void SysSetupSensor()
{
  while(1) {
    Serial.print(header);
    Serial.print("Sensor channel configuring menu");
    Serial.println(header);
    Serial.println("\n1\tDisplay configuration of a sensor channel");  
    Serial.println("2\tAdd a port to a sensor channel");
    Serial.println("3\tRemove a port from a sensor channel");
    Serial.println("4\tEnable a sensor channel");
    Serial.println("5\tDisable a sensor channel");
    Serial.println("6\tUpdate sensor channel name");
    Serial.println("7\tSet Working days");
    Serial.println("8\tToggle display updated configuration");
    Serial.println("9\tDisplay settings");
    Serial.println("10\tDone");
  
    v = GetInput(10); 
    if (v < 0 || v == 10) break;
    
    // get sensor
    if (v ==1 ) AllowAll = true;
    g = GetPortGroupSensor(SENSOR);
    if (g < 0) break;

    switch(v) {
      case 1:
        if (AllowAll && g == STRENTER) {
          for (j =0; j < CF_NUMSENSORS ; j++)  DispSensorInfo(j,false);
        } else  DispSensorInfo(g,false);
        AllowAll = false;
        break;
        
      case 2:
        v = GetPortGroupSensor(PORTS);
        if (v < 0) continue;
        if (! CF_SN_Ports(g, v, true))
          Serial.println("Error during adding");
        else {
          Serial.print("Port: ");
          Serial.print(v);
          Serial.print(" added to sensor: ");
          Serial.println(g);
        }
        break;

      case 3: // remove port
        v = GetPortGroupSensor(PORTS);
        if (v < 0) continue;
        if (! CF_SN_Ports(g, v, false))
          Serial.println("Error during removing");
        else {
          Serial.print("Port: ");
          Serial.print(v);
          Serial.print(" removed to sensor: ");
          Serial.println(g);
        }
        break;

      case 4: // enable sensor
        CF_SN_Enable(true, g);
        break;

      case 5: // disable sensor
        CF_SN_Enable(false, g);
        break;
        
      case 6: // update name
        UpdateName(g, SENSOR);
        break;

      case 7: // working day
        HandleWorkDays(SENSOR, g);
        break;

      case 8: // show of info
        DispSensor = !DispSensor;
        break;
        
      case 9:    // health information
        GPShealth(g, SENSOR);
        break; 
        
      default:
        break;
    }
    
    if(DispSensor) DispSensorInfo(g, false);
    Serial.println();
  }
}

/**
 * Initialize configuration to starting values
 */
void SysSetupInit()
{
  Serial.println("\nWARNING WARNING WARNING WARNING WARNING WARNING ");
  Serial.println("Current configuration will be overwritten !!!");
  Serial.println("Are you sure you want to set to initial setup ??");
  
  if (! AreYouSure()) {
    Serial.println("Action cancelled");
    return;
  }
  
  Serial.println("About the configuration backup location");
  Serial.println("1\tYES. Overwrite the configuration backup location.");
  Serial.println("2\tNO. Do NOT overwrite the configuration backup location.");
  Serial.println("3\tOH NO I have changed my mind. Please cancel !");
  v = GetInput(3); 
  if (v < 1 || v == 3) {
    Serial.println("Action cancelled");
    return;
  }
  // display and overwrite backup
  if (v == 1) CF_Init(true, true);
  //          display only
  else CF_Init(true, false);
} 

//////////////////////////////////////////////////////////
//  Manual switch
/////////////////////////////////////////////////////////

void ManualSwitch()
{
  bool disp = CF_DoMoAct.DisplaySwitch;
  CF_DoMoAct.DisplaySwitch = true;       // enable showing switch
  
  while(1) {
    Serial.print(header);
    Serial.print("Manual switch menu");
    Serial.println(header);
    Serial.println("\n1\tToggle / Switch a port.");
    Serial.println("2\tToggle / Switch a group.");
    Serial.println("3\tToggle / Switch a sensor channel.");
    Serial.println("4\tDone");
    v = GetInput(4);
    if (v < 0 || v == 4) {
      CF_DoMoAct.DisplaySwitch = disp; // restore original setting
      return;
    }
    s = false;
    switch(v) {
      case 1:
        // print enabled port names
        for (j = 0; j < CF_NUMPORTS;j++) {
          if (CF_DoMoAct.ports[j].Enable) {
            Serial.print(j);
            Serial.print("\t");
            Serial.print(CF_DoMoAct.ports[j].Name);
            Serial.print("\tcurrently ");
            if (PortStatus[j]) Serial.println("On");
            else Serial.println("Off");
            s = true;
          }
        }
        
        if (!s) {
          Serial.println("NO port enabled");
          continue;
        }
        
        // get port
        p = GetPortGroupSensor(PORTS);
        if (p < 0) return;
        
        j = SetPort(p,!PortStatus[p]);
        if (j < 0) {
          Serial.println("Port not enabled");
        }
        break;
      
      case 2:
        // print enabled group names
        for (j = 0; j < CF_NUMGROUPS;j++) {
          
          if (CF_DoMoAct.groups[j].Enable) {
            Serial.print(j);
            Serial.print("\t");
            Serial.print(CF_DoMoAct.groups[j].Name);
            Serial.print("\tcurrently ");
            if (GroupStatus[j]) Serial.println("On");
            else Serial.println("Off");
            s = true;
          }
        }
        
        if (!s) {
          Serial.println("NO Group enabled");
          continue;
        }
        
        // get group number
        g = GetPortGroupSensor(GROUP);
        if (g < 0) return;

        Serial.print("Group ");
        Serial.print(g);
             
        if (CF_DoMoAct.groups[g].Enable) {

          // switch status
          s = ! PortStatus[g];
          PortStatus[g] = s;

          Serial.print(" switched ");
          if (s) Serial.println("ON");
          else Serial.println("OFF");
          
          SwitchPorts(g, s);
        }
        else {          
          Serial.println(" not enabled");
        }
        break;
        
      case 3:
        // print enabled group names
        for (j = 0; j < CF_NUMSENSORS;j++) {
          
          if (CF_DoMoAct.sensors[j].Enable) {
            Serial.print(j);
            Serial.print("\t");
            Serial.print(CF_DoMoAct.sensors[j].Name);
            Serial.print("\tcurrently ");
            if (SensorStatus[j]) Serial.println("On");
            else Serial.println("Off");
            s = true;
          }
        }
        
        if (!s) {
          Serial.println("NO Sensor enabled");
          continue;
        }
        
        // get group number
        g = GetPortGroupSensor(SENSOR);
        if (g < 0) return;

        Serial.print("Sensor ");
        Serial.print(g);
             
        if (CF_DoMoAct.sensors[g].Enable) {

          // switch status
          s = ! SensorStatus[g];
          SensorStatus[g] = s;

          Serial.print(" switched ");
          if (s) Serial.println("ON");
          else Serial.println("OFF");
          
          SwitchSensor(g);
        }
        else {          
          Serial.println(" not enabled");
        }
        break;
        
      default:
        break;
    }
    Serial.println();
  }
}

//////////////////////////////////////////////////////////
//  MAINTENANCE
/////////////////////////////////////////////////////////
void SysMaintenance()
{
  while(1){
    Serial.print(header);
    Serial.print("Maintenance menu");
    Serial.println(header);
    Serial.println("\n1\tDisplay all configurations");
    Serial.println("2\tDisplay current time");
    Serial.println("3\tUpdate time");
    Serial.println("4\tDisplay sunset/sunrise times");
    Serial.println("5\tToggle debug message for time functions");
    Serial.println("6\tToggle debug message for Webfunctions");
    Serial.println("7\tMake a backup to PC");
    Serial.println("8\tRestore a backup from PC");
    Serial.println("9\tMake a backup in SPIFF");
    Serial.println("10\tRestore a backup from SPIFF");
    Serial.println("11\tSystem health check");
    Serial.println("12\tDisplay log of recent switches");
    Serial.println("13\tClear the recent log");
    Serial.println("14\tRestart DoMoAct");
    Serial.println("15\tToggle login requirement");
    Serial.println("16\tConfigure Dashboard");
    Serial.println("17\tChange Web background");
    Serial.println("18\tDone");
    v = GetInput(18);
    if (v < 0 || v == 18 ) return;
    
    switch(v) {
      case 1:
        DispObjectInfo();
        break;
        
      case 2: // get current time
        Serial.print(GetFromRTC(R_HOUR));
        Serial.print(":");
        Serial.print(GetFromRTC(R_MINUTE));
        Serial.print(":");
        Serial.println(GetFromRTC(R_SECOND));
        break;
        
      case 3: // update current time
        UpdateTime(); 
        break;

      case 4: // display sunrise/sunset
        Serial.print("Sunrise: ");
        Serial.print(twoDigits(sunrise / 60));
        Serial.print(":");
        Serial.print(twoDigits(sunrise % 60));
         
        Serial.print("\tSunset: ");
        Serial.print(twoDigits(sunset / 60));
        Serial.print(":");
        Serial.println(twoDigits(sunset % 60));
        break;

      case 5: // toggle time debug messages
        TimeDebug = ! TimeDebug;
        if (TimeDebug) Serial.println("TimeDebug enabled");
        else Serial.println("TimeDebug disabled");
        break;

      case 6: // toggle web debug messages
        WebDebug = ! WebDebug;
        if (TimeDebug) Serial.println("WebDebug enabled");
        else Serial.println("WebDebug disabled");
        break;
      
      case 7: // backup to PC
        CF_Export();
        break;
        
      case 8: // restore from PC
        CF_Import();
        break;
      
      case 9: // Back up to configuration
        Serial.println("The current parameters will overwrite to BACKUP location");
        Serial.println("Are you sure ?");
        if (AreYouSure()) CF_WriteBackUpConf(true);
        break;
        
      case 10: // restore backup from configuration
        Serial.println("The CURRENT parameters will be REPLACED from backup location");
        Serial.println("Are you sure ?");
        if (AreYouSure()) CF_ReadBackUpConf(true, false);
        break;
      
      case 11: //Health check
        SysMainHealthCheck();
        break;

      case 12: // Display log the recent switches
        CBuf_disp_entries();
        break;
        
      case 13: // clear log switches
        CBuf_clear();
        break;
      
      case 14:   
        Serial.println("You are about to restart DoMoAct");
        Serial.println("Are you sure ?");
        if (AreYouSure()){
          Serial.println("Restart NOW");
          delay(1000);
          ESP.restart();
        }
        break;
        
      case 15: // Toggle Login requirement
        AskMenuLogin = !AskMenuLogin;
        if (AskMenuLogin) Serial.println("Menu Login required");
        else Serial.println("Menu Login NOT required");
        break;
        
      case 16:  // configure Dashboard
        ConfigDashBoard();  
        break;

      case 17:  // configure web background
        ConfigBackground();  
        break;

      default:
        break;
    }
    Serial.println();
  }
}

/**
 * get input from keyboard
 * @param maxop: the maximum number to expect
 * 
 * return : 
 *  -1 NO entry was given
 *  else valid entry.
 */
int GetInput(int maxop)
{
  // flush pending input
  while(Serial.available()) Serial.read();
  
  Serial.print(" Only <enter> is return. (or wait ");
  Serial.print(INPUTDELAY);
  Serial.println(" seconds)");
  while(1) {
    
    String Keyb = Serial.readStringUntil(0x0a);

    if (CheckDuringMenu) {
      secureServer->loop();
      CheckSensors();
      DoSwitchOnOff();
    }

    // allow entering a '*'
    if (AllowAll){
      if (Keyb[0] == '*') return STRENTER;
    }

    if (Keyb.length() < 2) return -1;
    int in = Keyb.toInt();
    
    if(in > maxop) {
      Serial.print(in);
      Serial.print(" Invalid option. Max ");
      Serial.println(maxop);
    }
    else
      return(in);
  }
}

/**
 * Get a valid port or Group
 * @param port: true is get port, false is get group
 * 
 * return: valid port or group number
 */
int8_t GetPortGroupSensor(byte v)
{
  int8_t h;
  if(AllowAll) Serial.print("Provide * for all, else ");
  else Serial.print("Provide ");  
  
  if (v == PORTS) {
    Serial.print("a valid port number. Max. ");
    Serial.print(CF_NUMPORTS -1); // starts with port 0
    h = GetInput(CF_NUMPORTS -1);
  }
  else if (v == GROUP) {
    Serial.print("a valid Group number. Max. ");
    Serial.print(CF_NUMGROUPS -1); // starts with group 0
    h = GetInput(CF_NUMGROUPS -1);
  }
  else {
    Serial.print("a valid Sensor number. Max. ");
    Serial.print(CF_NUMSENSORS -1); // starts with sensor 0
    h = GetInput(CF_NUMSENSORS -1);
  }  

  // if no input
  if (h < 0) return h;

  if (v == PORTS) Serial.print("Port: ");
  else if (v == GROUP) Serial.print("Group: ");
  else Serial.print("Sensor: ");
  
  if (h == STRENTER) Serial.println("*");
  else Serial.println(h);
  
  return h;
}

/**
 * display the workdays
 * @param w : workdays value from structure
 * @param n : true = add number
 */
void DisplayWorkDays(int w, bool n)
{
    int d = 1;
    
    if (n) Serial.print(d++);
    Serial.print("\tSunday\t\t");
    if (((w >> CF_SUNDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tMonday\t\t");
    if (((w >> CF_MONDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tTuesday\t\t");
    if (((w >> CF_TUESDAY) & 0x1) == 0x1)Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tWednesday\t");
    if (((w >> CF_WEDNESDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tThursday\t");
    if (((w >> CF_THURSDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tFriday\t\t");
    if (((w >> CF_FRIDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (n) Serial.print(d++);
    Serial.print("\tSaturday\t");
    if (((w >> CF_SATURDAY) & 0x1) == 0x1) Serial.println("Enabled");
    else Serial.println("Disabled");
}

/**
 * Set/reset workdays
 * 
 * @param dev : PORTS, GROUP or SENSOR
 * @param n : number to port, group or sensor
 */
void HandleWorkDays(byte dev, int8_t n)
{
  while(1) {
    
    // get current values
    if (dev == PORTS) v = CF_DoMoAct.ports[n].Day;
    else if (dev == GROUP) v = CF_DoMoAct.groups[n].Day;
    else v = CF_DoMoAct.sensors[n].Day;

    Serial.print(header);
    Serial.print("Set Working Day");
    Serial.println(header);
    // display current weekdays with number (true)
    DisplayWorkDays(v, true);
    
    Serial.println("8\tReady");
    Serial.print("\nEnter number of day to toggle");
    
    j = GetInput(8);
    if (j < 0 || j == 8) return;
    
    uint8_t d = CF_NOTASSIGNED;
    switch(j) {
      case 0 :
        break;
      case 1 :
        d = CF_SUNDAY;
        break;
      case 2 :
        d = CF_MONDAY;
        break;
      case 3 :
        d = CF_TUESDAY;
        break;
      case 4 :
        d = CF_WEDNESDAY;
        break;
      case 5 :
        d = CF_THURSDAY;
        break;
      case 6 :
        d = CF_FRIDAY;
        break;
      case 7 :
        d = CF_SATURDAY;
        break;
      default:
        Serial.print("Invalid input ");
        Serial.println(j);
        break;
    }

    s = true;
    
    if( d != CF_NOTASSIGNED){
      if (((v >> d) & 0x1) == 0x1) s = false;
      
      if (dev == PORTS) CF_PT_SetDay(s, d, n);
      else if (dev == GROUP) CF_GR_SetDay(s, d, n);
      else CF_SN_SetDay(s, d, n);
    }
  }
}


/** 
 *  Get confirmation on critical action
 *  
 *  return : True if confirmed
 */
bool AreYouSure()
{
  Serial.print("To validate input: 5678");

  v = GetInput(6000);  
  
  if (v < 0 || v != 5678) {
    Serial.println("Action cancelled");
    return false;
  }
  return true;
}

/**
 * Check that port is part of a group or sensor
 * @param p : valid port number
 * 
 * Return true = part of group or sensor.
 */
bool CheckForGroupOrSensor(int p)
{
  // if part of group
  for(j = 0; j < CF_NUMGROUPS; j++){
    if (CF_DoMoAct.ports[p].Group[j] != CF_NOTASSIGNED)
      return true;
  }
  
  // check that port is assigned to sensor
  for(j = 0; j < CF_NUMSENSORS; j++){
    
    for(byte i = 0; i < CF_NUMPORTS; i++){
      if (CF_DoMoAct.sensors[j].Ports[i] == p)
      return true;
    }
  }
  
  return false;
}

/**
 * check that a port conditions are valid to enable
 * 
 * If port conditions are valid : enable the port
 * else disable the port.
 * 
 * return:
 * true : port is enabled
 * false : part is disabled
 */
bool PortValidEnable(int p)
{
  bool valid = true;
  
  // check GPIO
  if (CF_DoMoAct.ports[p].Gpio == CF_NOTASSIGNED) {
    valid = false;
  }
  
  // enable or disable port
  CF_PT_Enable(valid, p);

  return(valid);
}

/**
 * update name
 * @param h : group, port or sensornumber
 * @param dev : PORTS, GROUP or SENSOR
 */
void UpdateName(int h, byte dev)
{
  Serial.print("Current name: ");
  if (dev == PORTS) Serial.print(CF_DoMoAct.ports[h].Name);
  else if (dev == GROUP) Serial.print(CF_DoMoAct.groups[h].Name);
  else Serial.print(CF_DoMoAct.sensors[h].Name);
  Serial.println(". Provide a new name. only <enter> so cancel");
  n = Serial.readStringUntil(0x0d);
  
  if (n.length() > CF_NAMELEN )
    Serial.println("Name is too long");
  else if (n.length() > 0) {
    if (dev == PORTS) CF_PT_Name(n, h);
    else if (dev == GROUP) CF_GR_Name(n, h);
    else CF_SN_Name(n, h);
    return;
  }
  
  Serial.println("cancelled");
}

/**
 * check different settings are correct
 * @param TrytoCorrect : true try to correct a detected error
 */
void SysMainHealthCheck()
{
  uint8_t group[CF_NUMGROUPS] = {0};

  for (p = 0 ; p < CF_NUMPORTS ; p++) {
    Serial.print("\nChecking port: ");
    Serial.println(p);
    
    if (! GPShealth(p,PORTS)) {
      Serial.println("POTENTIAL ACTION TO TAKE");
    }
  }

  // obtain the group count
  for (j = 0 ; j < CF_NUMGROUPS ; j++) {
      if (CF_DoMoAct.ports[p].Group[j] != CF_NOTASSIGNED) group[CF_DoMoAct.ports[p].Group[j]]++;
  }
  
  // check group counts are correct
  for (j = 0 ; j < CF_NUMGROUPS ; j++) {
    Serial.print("\nChecking group: ");
    Serial.println(j);

    if (! GPShealth(j,GROUP)) {
      Serial.println("WARNING : POTENTIAL ACTION TO TAKE");
    }
    
    if (CF_DoMoAct.groups[j].count != group[j]) {
      Serial.print("WARNING: Mismatch on count for group ");
      Serial.print(j);
      Serial.print(" detected ");
      Serial.print(group[j]);
      Serial.print(" current count is ");
      Serial.print(CF_DoMoAct.groups[j].count);
      Serial.println(" (Corrected!)");  
      
      CF_DoMoAct.groups[j].count = group[j];
    }
    else if (group[j] == 0)  Serial.println("WARNING: NO Port assigned");
    else {
      Serial.print("INFO: ");
      Serial.print(group[j]);
      Serial.println(" ports assigned");
    }
  }

  for (p = 0 ; p < CF_NUMSENSORS ; p++){
    
    Serial.print("\nChecking Sensor: ");
    Serial.println(p);
    
    if (! GPShealth(p,SENSOR)) {
      Serial.println("WARNING: POTENTIAL ACTION TO TAKE");
    }
    
    int c = 0;
    
    for(j = 0; j < CF_NUMPORTS; j++){
      if (CF_DoMoAct.sensors[p].Ports[j] != CF_NOTASSIGNED) {
        Serial.print("INFO: Assigned Port ");
        Serial.println(CF_DoMoAct.sensors[p].Ports[j]);
        c++;
      }
    }
    
    if (c == 0) Serial.println("WARNING: NO Port assigned");
  }
 
  // check for correct backup in configuration
  if (! CF_ReadBackUpConf(false, true))
  {
    Serial.println("configuration backup does not look to be correct");
    CF_WriteBackUpConf(true);   // force an update
  }
 
  // check RTC is set
  if (GetFromRTC(R_DAY) == 0 && GetFromRTC(R_HOUR) == 0 && GetFromRTC(R_YEAR) == 0){
    Serial.println("The RTC time is suspected to be incorrect");
  }
}

/**
 * check health of port/group or sensor
 * 
 * return:
 * true : no concern
 * false : some concerns
 */
bool GPShealth(int p, int gps)
{
  bool valid = true;
  bool gs = true;
  int ts = 0;
  
  if (gps == PORTS) {

    if (! CF_DoMoAct.ports[p].Enable) {
      Serial.println("INFO: Not Enabled.");
      valid = false;
    }
    else
      Serial.println("INFO: Enabled.");
    
    // check GPIO
    if (CF_DoMoAct.ports[p].Gpio == CF_NOTASSIGNED) {
      Serial.println("WARNING: NO GPIO assigned.");
      valid = false;
    }
    else
      Serial.print("INFO: GPIO ");
      Serial.print(CF_DoMoAct.ports[p].Gpio);
      Serial.println("assigned.");
    
    // if not part of group or sensor
    if (! CheckForGroupOrSensor(p)) {
      Serial.println("INFO: NOT Assigned to group or sensor.");
      gs = false;
    }
    else
      Serial.println("INFO: Part of Group or sensor.");
      
    if (CF_DoMoAct.ports[p].Day != 0x7f) {
      Serial.println("INFO: NOT all days are enabled.");
    }
    else
      Serial.println("INFO: All days are enabled.");
    
    // no fixed On time was set
    if (CF_DoMoAct.ports[p].On_minute == CF_NOTASSIGNED || CF_DoMoAct.ports[p].On_hour == CF_NOTASSIGNED) {
      Serial.println("INFO: NO OnMinute or OnHour set.");
    }
    else{
      Serial.println("INFO: OnMinute and OnHour are set.");
      ts = 1;
    }
    
    // no sunset was set
    if (! CF_DoMoAct.ports[p].On_sunset) {
      Serial.println("INFO: NO On_Sunset.");
    } 
    else {
      Serial.println("INFO: set for On_Sunset.");
      ts += 2;
    }
    
    if (ts == 0) {    // if NO timeSwitch set
      if (! gs) {
        Serial.println("WARNING: this port will NOT switch ON."); 
        valid = false;
      }
    }
    else if (ts == 1) Serial.println("INFO: this port switch ON based on time setting."); 
    if (ts == 2 || ts == 3) Serial.println("INFO: this port switch ON based on sunset."); 

    if (gs) Serial.println("INFO: This port switch ON based on Group or sensor.");   

    ts = 0;
    
    // no fixed OFF time was set
    if (CF_DoMoAct.ports[p].Off_minute == CF_NOTASSIGNED || CF_DoMoAct.ports[p].Off_hour == CF_NOTASSIGNED){
      Serial.println("INFO: NO OffMinute or OffHour set.");
    }
    else {
      Serial.println("INFO: OFFMinute and OFFHour are set.");
      ts = 1;
    }
    
    // no sunrise was set
    if (! CF_DoMoAct.ports[p].Off_sunrise){ 
      Serial.println("INFO: NO On_Sunrise.");
    }
    else {
      Serial.println("INFO: set for On_Sunrise.");
      ts += 2;
    }
        
    if (ts == 0) {    // if NO timeSwitch set
      if (! gs) {
        Serial.println("WARNING: this port will NOT switch OFF."); 
        valid = false;
      }
    }
    else if (ts == 1) Serial.println("INFO: this port switch OFF based on time setting."); 
    if (ts == 2 || ts == 3) Serial.println("INFO: this port switch OFF based on sunrise."); 

    if (gs) Serial.println("INFO: This port switch OFF based on Group or sensor.");   
  
  }

  if (gps == GROUP){

    if (! CF_DoMoAct.groups[p].Enable) {
      Serial.println("INFO: group is disabled.");
      valid = false;
    }
    else
      Serial.println("INFO: Group is Enabled.");
      
    if (CF_DoMoAct.groups[p].Day != 0x7f) {
      Serial.println("INFO: not all days are enabled.");
    }
    else
      Serial.println("INFO: All days are enabled.");
    
    // no fixed On time was set
    if (CF_DoMoAct.groups[p].On_minute == CF_NOTASSIGNED || CF_DoMoAct.groups[p].On_hour == CF_NOTASSIGNED) {
      Serial.println("INFO: NO OnMinute or OnHour set.");
    }
    else {
      Serial.println("INFO: OnMinute and OnHour are set.");
      ts = 1;
    }
    
    // no sunset was set
    if (! CF_DoMoAct.groups[p].On_sunset) {
      Serial.println("INFO: NO On_Sunset");
    } 
    else {
      Serial.println("INFO:set for On_Sunset");
      ts += 2;
    }
    
    if (ts == 0) {    // if NO timeSwitch set
        Serial.println("WARNING: this group will NOT switch ON."); 
        valid = false;
    }
    else if (ts == 1) Serial.println("INFO: this group switch ON based on time setting."); 
    if (ts == 2 || ts == 3) Serial.println("INFO: this group switch ON based on sunset."); 

    ts = 0;
    
    // no fixed OFF time was set
    if (CF_DoMoAct.groups[p].Off_minute == CF_NOTASSIGNED || CF_DoMoAct.groups[p].Off_hour == CF_NOTASSIGNED){
      Serial.println("INFO: NO OffMinute or OffHour set");
    }
    else {
      Serial.println("INFO: OFFMinute and OFFHour are set.");
      ts = 1;
    }
    
    // no sunrise was set
    if (! CF_DoMoAct.groups[p].Off_sunrise){ 
      Serial.println("INFO: NO On_Sunrise");
    }
    else {
      Serial.println("INFO:set for On_Sunrise");
      ts += 2;
    }  
        
    if (ts == 0) {    // if NO timeSwitch set
        Serial.println("WARNING: this group will NOT switch OFF."); 
        valid = false;
    }
    else if (ts == 1) Serial.println("INFO: this group switch OFF based on time setting."); 
    if (ts == 2 || ts == 3) Serial.println("INFO: this group switch OFF based on sunrise."); 
  }

  if (gps == SENSOR){
    
    if (! CF_DoMoAct.groups[p].Enable) {
      Serial.println("INFO: sensor is disabled");
      valid = false;
    }
    else
      Serial.println("INFO: sensor is Enabled.");

    if (CF_DoMoAct.groups[p].Day != 0x7f) {
      Serial.println("INFO: not all days are enabled");
    }
    else
      Serial.println("INFO: All days are enabled");
    
    
  }
  return(valid);
}
