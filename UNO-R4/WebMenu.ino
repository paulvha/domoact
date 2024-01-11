struct ParseValues{
  char lookup[10];              // look up value in line
  int  num;                     // number to return in case of menu or store parsed value
  char val[MAX_CH_LENGTH];      // store parsed a char value
};

// general look for number or string
ParseValues MenuPGS_Items[] {
  {"Select", 0, ""},
  {"Done", 0, ""}
};

#define LINE_ID  "GET /DMACT?"  //Parse only line with this ID

#define PMENU 1                 // parseline options
#define PNUM  2
#define PVAL  3

#define WEB_MAIN_MENU   0       // TOP WEB menu reference
#define WEB_SWITCH_MENU 1
#define WEB_CONF_MENU   2
#define WEB_MNTS_MENU   3
#define WEB_DASH_MENU   5
#define WEB_LOGIN_MENU  9

#ifdef USERSENSORMENU == 1
// for optional user sensor web menu example
ParseValues UserMenuItems[] {
  {"Q1",1,""},        // for each question on a page make a seperate entry. make as many as you want
  {"Q2",2,""},        // the second entry (2 in this case) will be the parse answer
  {"Q3",3,""},   
  {"Q3",4,""},
  {"done",9,""}       // last entry. Selecting done will result in return   
};

#define WEB_USER_MENU   4       // top menu
#define WEB_USER_SUB1   41      // next submenu...
#define WEB_USER_SUB2   42
#define WEB_USER_SUB3   43      // add more if needed ( 44 ..)
#endif

// headers to reduce memory usage
char linehead[] = "  <input type=\"radio\" id=\"menu\" name=\"menu\" ";

int PortGroup = 0xfff;          // Hold either group, sensor or port in case of adding / remove port
String currentLine;             // holds the received respondse
String Value = "";              // to parse a number or value
int MenuWebSel = EE_NOTASSIGNED;// Which is the next menu to display 
int ParseSelect = 0;            // which parser to use for the responds from the client
int LastMenuWebSelectTime = 0;  // last time a menu selecting was done
bool FirstMenuSet = false;      // reset to MainWebMenu after WEBMENURESET minutes in activity
bool InValidLogin= false;       // false : last login attempt was valid

/////////////////////////////login web menu ///////////////////////

/**
 * get a username and password if BOTH are defined in DoMoAct.h
 */
void LoginWebMenu()
{
  html_begin();
  
  if (InValidLogin) {
    client.println("  </H2> <H4 style=\"background-color:Tomato;\"> Invalid login. Retry </H4> <H2>");
    InValidLogin = false;
  }
  
  client.println("<div>");
  client.println("  <label for=\"username\">Username:</label>");
  client.println("  <input type=\"text\" id=\"username\" name=\"username\" />");
  client.println("</div>");
  
  client.println("<div>");
  client.print("  <label for=\"pass\">Password (");
  client.print(MIN_PSWD_LENGTH);
  client.println(" characters minimum):</label>");
  client.print("  <input type=\"password\" id=\"pass\" name=\"password\" minlength=\"");
  client.print(MIN_PSWD_LENGTH);
  client.println("\" required />");
  client.println("</div>");
  client.println("</H2>");
  
  client.println("<input type=\"submit\" class=\"button\" value=\"Sign in\" >");
  
  html_end();
}

ParseValues LoginMenuItems[] {
  {"username",0,""},
  {"password",0, ""}
};

/**
 * Parse the login information like :
 * GET /DMACT?username=Domoact&password=test1 HTTP/1.1
 *
 */
void LoginWebParse()
{
  InValidLogin = false;
  
  int num = ParseString(WEB_LOGIN_MENU);
  
  if (num == -1) return;  // invalid line-id
  
  if ( ParseString(WEB_LOGIN_MENU) != 2 ) InValidLogin = true; 
  else {
    // check for username match
    if (strcmp(Acc_Name, LoginMenuItems[0].val) != 0)  InValidLogin = true; 

    // check password (todo : should be encrypted)
    if (strcmp(Acc_Pswd, LoginMenuItems[1].val) != 0)  InValidLogin = true; 
  }

  // false means we are good else retry with warning
  if (! InValidLogin) MenuWebSel = WEB_MAIN_MENU;
}

/////////////////////////////  Main Web Menu /////////////////////

void MainWebMenu()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Main menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"Tswitch\"/>&nbsp;&nbsp;&nbsp;&nbsp;Manual switch <br> <br>");
  client.print(linehead);
  client.println("value=\"Tconf\"/>&nbsp;&nbsp;&nbsp;&nbsp;Configuration menu <br> <br>");
  client.print(linehead);
  client.println("value=\"Tmnt\"/>&nbsp;&nbsp;&nbsp;&nbsp;Maintenance menu <br/> <br>");
  client.print(linehead);
  client.println("value=\"Tdash\"/>&nbsp;&nbsp;&nbsp;&nbsp;Dashboard<br/> <br>");
#ifdef USERSENSORMENU == 1
  client.print(linehead);
  client.println("value=\"Tusr\"/>&nbsp;&nbsp;&nbsp;&nbsp;User sensor menu <br/> <br>");
#endif
  client.println("  </H2> <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues MainMenuItems[] {
  {"Tswitch", WEB_SWITCH_MENU},
  {"Tconf",WEB_CONF_MENU},
  {"Tmnt", WEB_MNTS_MENU},
  {"Tusr", WEB_USER_MENU},
  {"Tdash",WEB_DASH_MENU}
};

/////////////////////////////  manual switch options /////////////////////

void SwitchWebMenu(){

  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Manual Switch menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"MSport\"/>&nbsp;&nbsp;&nbsp;&nbsp;Toggle / Switch a port<br> <br>");
  client.print(linehead);
  client.println("value=\"MSgroup\"/>&nbsp;&nbsp;&nbsp;&nbsp;Toggle / Switch a group<br> <br>");
  client.print(linehead);
  client.println("value=\"MSsensor\"/>&nbsp;&nbsp;&nbsp;&nbsp;Toggle / Switch a sensor<br> <br>");
  client.print(linehead);
  client.println("value=\"MSdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

// will forward to void AskPortGroupSensor(int ask)
// to show and ask
ParseValues SwitchWebMenuItems[] {
  {"MSport", 11},
  {"MSgroup",12},
  {"MSsensor",13},
  {"MSdone", WEB_MAIN_MENU}
};

/**
 * asuming there are 'ONLY' 15 ports, groups or Sensors
 * If more.. than adjust
 */
ParseValues PGSSelectItems[] {
  {"A0", 0},
  {"A1", 1},
  {"A2", 2},
  {"A3", 3},
  {"A4", 4},
  {"A5", 5},
  {"A6", 6},
  {"A7", 7},
  {"A8", 8},
  {"A9", 9},
  {"A10", 10},
  {"A11", 11},
  {"A12", 12},
  {"A13", 13},
  {"A12", 14},
  {"A13", 15},
  {"Done", 50}    // main menu
};

/**
 * parse responds on toggle group, port or sensor
 */
void SwitchParse(int s_act)
{
  int se = ParseMenu(11);

  if ( se < 0) return;
  
  // Next is manual switch
  MenuWebSel = WEB_SWITCH_MENU;
  
  if (se == 50) {
    if (WebDebug) Serial.println("Cancelled");
  }
  else if (s_act == 11){
    if (WebDebug) {
      Serial.print("Setting now port : ");
      Serial.println(se);
    }
    j = SetPort(se,! PortStatus[se]);
    
    if (j < 0) {
      if (WebDebug) Serial.println("Port not enabled");
    }
  }
  else if (s_act == 12){
    if (WebDebug){
      Serial.print("setting now group : ");
      Serial.println(se);
    }
    // switch status
    j = SwitchPorts(se, !GroupStatus[se]);
    
    if (j < 0) {
      if (WebDebug) Serial.println("Port not enabled");
    }
  }
  else if (s_act == 13){
    if (WebDebug){
      Serial.print("setting now Sensor : ");
      Serial.println(se);
    }

    // flip status
    SensorStatus[se] = !SensorStatus[se];
    
    // switch status
    j = SwitchSensor(se);
    
    if (j < 0) {
      if (WebDebug) Serial.println("Sensor not enabled");
    }
  }
}

/**
 * Show dashboard
 */
void DashWebMenu()
{
  int pgs, num;
  int pointer = 0;
  bool DashEmpty = true;
  
  html_begin();
  client.println("</H2><H2 style=\"color:yellow;\"> <label for=\"menu\">Dashboard</label> </H2> <H2>");
  
  for (j = 0; j < EE_NUMDASHBOARD; j++) {
  
    if (EE_DoMoAct.DashBoard[j].PGS == EE_NOTASSIGNED) continue;  // not used entry
    
    pgs = EE_DoMoAct.DashBoard[j].PGS;
    num = EE_DoMoAct.DashBoard[j].num;
    DashEmpty = false;
    
    rem[pointer++] = j;           // remember location

    client.print(linehead);
    client.print("value=\"A");
    client.print(j);
    client.print("\"/>");

    if (pgs == PORTS) {
        client.print("P-");
        client.print(num);
        client.print("&nbsp;");
        client.print(EE_DoMoAct.ports[num].Name);
        if (EE_DoMoAct.ports[num].Enable) {
          client.print("&nbsp;enabled ");
          if(PortStatus[num]) client.println("ON");
          else client.println("OFF");
        }
        else client.println("&nbspdisabled");
    }
    else if (pgs == GROUP) {
        client.print("G-");
        client.print(num);
        client.print("&nbsp;");
        client.print(EE_DoMoAct.groups[num].Name);
        if (EE_DoMoAct.groups[num].Enable) {
          client.print("&nbsp;enabled ");
          if(GroupStatus[num]) client.println("ON");
          else client.println("OFF");
        }
        else client.println("\tdisabled");
    }
    else if (pgs == SENSOR) {
        client.print("S-");
        client.print(num);
        client.print("&nbsp;");
        client.print(EE_DoMoAct.sensors[num].Name);
        if (EE_DoMoAct.sensors[num].Enable) {
          client.print("&nbsp;enabled ");
          if(SensorStatus[num]) client.println("ON");
          else client.println("OFF");
        }
        else client.println("&nbspdisabled");
    }
    client.print("<br>");
  }
  
  if (DashEmpty) {
    client.println("Empty : nothing defined. <BR>");
  }
  
  client.println("<br>");
  client.print(linehead);
  client.println("value=\"Done\"/>&nbsp;Cancel <BR/> <br> </H2><BR>");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

/** 
 *  parse dashboard feedback
 */
void DashWebParse()
{
  int se = ParseMenu(WEB_DASH_MENU);
  if (se < 0)  return;   // wrong line    
  
  if(se == 50) {        // cancel
    MenuWebSel = WEB_MAIN_MENU;
    return; 
  }
  
  int pgs = EE_DoMoAct.DashBoard[rem[se]].PGS;
  int num = EE_DoMoAct.DashBoard[rem[se]].num;

  if (pgs == PORTS)  j = SetPort(num,!PortStatus[num]);
  else if (pgs == GROUP) j = SwitchPorts(num,!GroupStatus[num]);
  else if (pgs == SENSOR) {
    SensorStatus[num] = ! SensorStatus[num];
    SwitchSensor(num);
  }
  
  MenuWebSel = WEB_DASH_MENU;
}

/////////////////////////////  configure options /////////////////////

void ConfigWebMenu()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Configuration menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"CFport\"/>&nbsp;&nbsp;&nbsp;&nbsp;Port configuration <br> <br>");
  client.print(linehead);
  client.println("value=\"CFgroup\"/>&nbsp;&nbsp;&nbsp;&nbsp;Group configuration <br> <br>");
  client.print(linehead);
  client.println("value=\"CFsensor\"/>&nbsp;&nbsp;&nbsp;&nbsp;Sensor channel configuration <br> <br>");
  client.print(linehead);
  client.println("value=\"CFdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues ConfigWebMenuItems[] {
  // removed option 21, place holder for now
  {"CFport",  22},
  {"CFgroup", 23},
  {"CFsensor",24},
  {"CFdone", WEB_MAIN_MENU}
};

// port configuration menu
void MenuPortConfig()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Port Configuration menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"PFdisp\"/>&nbsp;&nbsp;&nbsp;&nbsp;Display configuration of a port <br> <br>");
  client.print(linehead);
  client.println("value=\"PFenable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Enable port <br> <br>");
  client.print(linehead);
  client.println("value=\"PFdisable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Disable port <br> <br>");
  client.print(linehead);
  client.println("value=\"PFadd\"/>&nbsp;&nbsp;&nbsp;&nbsp;Add a port to a group<br> <br>");
  client.print(linehead);
  client.println("value=\"PFremove\"/>&nbsp;&nbsp;&nbsp;&nbsp;Remove a port from a group <br> <br>");
  client.print(linehead);
  client.println("value=\"PFdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues PFMenuItems[] {
  {"PFdisp",    221},
  {"PFenable",  222},
  {"PFdisable", 223},
  {"PFadd",     224},
  {"PFremove",  225},
  {"PFdone", WEB_CONF_MENU}
};

// Group configuration menu
void MenuGroupConfig()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Group Configuration menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"GFdisp\"/>&nbsp;&nbsp;&nbsp;&nbsp;Display configuration of a group <br> <br>");
  client.print(linehead);
  client.println("value=\"GFenable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Enable group <br> <br>");
  client.print(linehead);
  client.println("value=\"GFdisable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Disable group <br> <br>");
  client.print(linehead);
  client.println("value=\"GFadd\"/>&nbsp;&nbsp;&nbsp;&nbsp;Add a port to a group<br> <br>");
  client.print(linehead);
  client.println("value=\"GFremove\"/>&nbsp;&nbsp;&nbsp;&nbsp;Remove a port from a group <br> <br>");
  client.print(linehead);
  client.println("value=\"GFdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues GFMenuItems[] {
  {"GFdisp",    231},
  {"GFenable",  232},
  {"GFdisable", 233},
  {"GFadd",     234},
  {"GFremove",  235},
  {"GFdone", WEB_CONF_MENU}
};

// Sensor channel configuration menu
void MenuSensorConfig()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Sensor channel Configuration menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"SNdisp\"/>&nbsp;&nbsp;&nbsp;&nbsp;Display configuration of a sensor channel <br> <br>");
  client.print(linehead);
  client.println("value=\"SNenable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Enable sensor channel <br> <br>");
  client.print(linehead);
  client.println("value=\"SNdisable\"/>&nbsp;&nbsp;&nbsp;&nbsp;Disable sensor channel <br> <br>");
  client.print(linehead);
  client.println("value=\"SNadd\"/>&nbsp;&nbsp;&nbsp;&nbsp;Add a port to a sensor<br> <br>");
  client.print(linehead);
  client.println("value=\"SNremove\"/>&nbsp;&nbsp;&nbsp;&nbsp;Remove a port from a sensor<br> <br>");
  client.print(linehead);
  client.println("value=\"SNdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues SNMenuItems[] {
  {"SNdisp",    241},
  {"SNenable",  242},
  {"SNdisable", 243},
  {"SNadd",     244},
  {"SNremove",  245},
  {"SNdone", WEB_CONF_MENU}
};

/////////////////////////////  Maintenance options /////////////////////

void MaintenanceWebMenu() 
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Maintenance menu: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"MMtime\"/>&nbsp;&nbsp;&nbsp;&nbsp;Update current time <br> <br>");
  client.print(linehead);
  client.println("value=\"MMbackup\"/>&nbsp;&nbsp;&nbsp;&nbsp;Backup to EEPROM <br> <br>");
  client.print(linehead);
  client.println("value=\"MMrestore\"/>&nbsp;&nbsp;&nbsp;&nbsp;Restore from EEPROM <br> <br>");
  client.print(linehead);
  client.println("value=\"MMrestart\"/>&nbsp;&nbsp;&nbsp;&nbsp;Restart DoMoAct <br> <br>");
  client.print(linehead);
  client.println("value=\"MMback\"/>&nbsp;&nbsp;&nbsp;&nbsp;Change Web background <br> <br>");
  client.print(linehead);
  client.println("value=\"MMdone\"/>&nbsp;&nbsp;&nbsp;&nbsp;Done<br> <br> </H2>");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

ParseValues MM_MenuItems[] {
  {"MMtime",    31},
  {"MMbackup",  32},
  {"MMrestore", 33},
  {"MMrestart", 34},
  {"MMback", 35},
  {"MMdone", WEB_MAIN_MENU}
};

/**
 * There is NO further request/responds needed.
 * Hence we handle it here.
 */
void MaintenanceParse()
{
  int m_act = ParseMenu(WEB_MNTS_MENU);
  
  if (m_act < 0) return;
  
  // next menu
  MenuWebSel = WEB_MNTS_MENU;
  
  if (m_act == 31) {
    UpdateTime(); 
    if (WebDebug) Serial.println("Update time ");
  }
  else if(m_act == 32) {
    MenuWebSel = 321;
    if (WebDebug) Serial.println("backup current to EEPROM");
  }
  else if (m_act == 33) {
    MenuWebSel = 331;
    if (WebDebug) Serial.println("ask restore backup from EEPROM");
  }
  else if (m_act == 34) {
    MenuWebSel = 341;
    if (WebDebug) Serial.println("Ask for restart EEPROM");
  }
  else if (m_act == 35) {
    MenuWebSel = 35;
    if (WebDebug) Serial.println("Change background");
  }
  else if (m_act == WEB_MAIN_MENU) {      // done
    MenuWebSel = WEB_MAIN_MENU;
  }
}


void ChangeBackgroundMenu()
{
  html_begin();
  client.println("  <label style=\"color:yellow;\" for=\"menu\">Change background: </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"A1\"/>&nbsp;&nbsp;Starfield<br> <br>");
  client.print(linehead);
  client.println("value=\"A2\"/>&nbsp;&nbsp;Scrolling sand<br> <br>");
  client.print(linehead);
  client.println("value=\"A3\"/>&nbsp;&nbsp;Sliding diagonal<br> <br>");
  client.print(linehead);
  client.println("value=\"A4\"/>&nbsp;&nbsp;Pure CSS3 Gradient<br> <br>");
  client.print(linehead);
  client.println("value=\"A5\"/>&nbsp;&nbsp;Gradient waves<br> <br>");
  client.print(linehead);
  client.println("value=\"A6\"/>&nbsp;&nbsp;Fireworks<br> <br>");
  client.print(linehead);
  client.println("value=\"A7\"/>&nbsp;&nbsp;Stars<br> <br>");
  client.print(linehead);
  client.println("value=\"Done\"/>&nbsp;&nbsp;Cancel <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

void ChangeBackgroundParse()
{
  int se = ParseMenu(11);
  
  if (se < 0) return;
  MenuWebSel = WEB_MNTS_MENU;
  
  if (se == 50) return;
  EE_Set_WebBackGround(se);
}

////////////////////////// ERROR  /////////////////////////////////
// (internal) Error found menu
void MenuError()
{
  html_begin();
  client.println("  <label style=\"color:RED;\" for=\"menu\">ERROR Internal error detected !! </label> <br> <br>");
  client.print(linehead);
  client.println("value=\"Done\"/>&nbsp;&nbsp;&nbsp;&nbsp;Select to return to MainMenu <br> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

////////////////////////// supporting functions //////////////////
/**
 * get the combination of Port and Group, or Port and sensor
 */
void MenuPortAddRemoveGroupSensor(int C_act)
{
  int se = ParseMenu(11);
  
  if (se < 0) {
    if (WebDebug){
      Serial.print("Action: ");
      Serial.print(C_act);
      Serial.println(" in error");
    }
    return;
  }
  
  // set Port, group or sensor configuration
  MenuWebSel = C_act / 100;
  
  if (se == 50){
    if (WebDebug){
      Serial.print("Action: ");
      Serial.print(C_act);
      Serial.println(" cancelled");
    }
    return;
  }
  
  if (C_act == 2241) {
    if (! EE_GR_Ports(se, PortGroup, true))
     if (WebDebug) Serial.println("Error during adding");
    else {
      if (WebDebug) {
        Serial.print("Add now port: ");
        Serial.print(PortGroup);
        Serial.print(" to group ");
        Serial.println(se);
      }
    }
  }
  else  if (C_act == 2251) {
    if (! EE_GR_Ports(se, PortGroup, false))
      if (WebDebug) Serial.println("Error during removing");
    else {
      if (WebDebug){
        Serial.print("Removed port: ");
        Serial.print(PortGroup);
        Serial.print(" from group ");
        Serial.println(se);
      }
    }
  }
  else if (C_act == 2341) {
    if (! EE_GR_Ports(PortGroup, se , true))
      if (WebDebug) Serial.println("Error during adding");
    else {
      if (WebDebug) {
        Serial.print("Added port: ");
        Serial.print(se);
        Serial.print(" to group ");
        Serial.println(PortGroup);
      }
    }
  }
  else  if (C_act == 2351) {
    if (! EE_GR_Ports(PortGroup, se , false))
      if (WebDebug) Serial.println("Error during removing");
    else {
      if (WebDebug) {
        Serial.print("Removed port: ");
        Serial.print(se);
        Serial.print(" from group ");
        Serial.println(PortGroup);
      }
    }
  }
  else if (C_act == 2441) {

    if (! EE_SN_Ports(PortGroup, se, true))
      if (WebDebug) Serial.println("Error during enable");
    else {
      if (WebDebug){
        Serial.print("Port: ");
        Serial.print(se);
        Serial.print(" added to sensor: ");
        Serial.println(PortGroup);
      }
    }
  }
  else  if (C_act == 2451) {
    if (! EE_SN_Ports(PortGroup, se, false))
      if (WebDebug) Serial.println("Error during removing");
    else {
      if (WebDebug){
        Serial.print("Port: ");
        Serial.print(se);
        Serial.print(" removed to sensor: ");
        Serial.println(PortGroup);
      }
    }
  }
}

/** 
 *  Parse responds from port, group or sensor configuration menu
 */
void MenuConfParse(int C_act)
{
  int se = ParseMenu(11);  // default
 
  if (se < 0) return;

  // set Port, group or sensor configuration
  MenuWebSel = C_act / 10;

  if (se == 50){
    if (WebDebug){
      Serial.print("Action: ");
      Serial.print(C_act);
      Serial.println(" cancelled");
    }
    return;
  }
 
  if (C_act == 221) { 
    if (WebDebug){
      Serial.print("Display now port  : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2211;
  }
  else if(C_act == 222) {
    if (! PortValidEnable(se)) {
      if (WebDebug) Serial.println("Could not enable port");
    }
    else {
      if (WebDebug){  
        Serial.print("Port: ");
        Serial.print(se);
        Serial.println(" is enabled");
      }
    }
  }
  else if (C_act == 223) {
    EE_PT_Enable(false, se);
    if (WebDebug) {
      Serial.print("Port: ");
      Serial.print(se);
      Serial.println(" is disabled");
    }
  }
  else if (C_act == 224) {
    if (WebDebug){
      Serial.print("Add now port : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2241; 
  }
  else if (C_act == 225) {
    if (WebDebug) {
      Serial.print("remove now port : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2251; 
  }
  else if (C_act == 231) {
    if (WebDebug){
      Serial.print("Display now group  : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2311;
  }
  else if(C_act == 232) {
    EE_GR_Enable(true, se);
    if (WebDebug){
      Serial.print("Group: ");
      Serial.print(se);
      Serial.println(" enabled");
    }
  }
  else if (C_act == 233) {
    EE_GR_Enable(false, se);
    if (WebDebug){
      Serial.print("Group: ");
      Serial.print(se);
      Serial.println(" disabled");
    }
  }
  else if (C_act == 234) {
    if (WebDebug){
      Serial.print("Add now group : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2341; 
  }
  else if (C_act == 235) {
    if (WebDebug){
      Serial.print("remove now group : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2351; 
  }
  else if (C_act == 241) {
    if (WebDebug){
      Serial.print("Display now sensor  : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2411;
  }
  else if(C_act == 242) {
    EE_SN_Enable(true, se);
    if (WebDebug){
      Serial.print("Enable now sensor  : ");
      Serial.println(se);
    }
  }
  else if (C_act == 243) {
    EE_SN_Enable(false, se);
    if (WebDebug){
      Serial.print("Disable now sensor : ");
      Serial.println(se);
    }
  }
  else if (C_act == 244) {
    if (WebDebug) {
      Serial.print("Add now port : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2441; 
  }
  else if (C_act == 245) {
    if (WebDebug) {
      Serial.print("remove now port : ");
      Serial.println(se);
    }
    PortGroup = se;
    MenuWebSel = 2451; 
  }
}

/**
 * display port / group or sensor info
 */

void WebDisplayConf(int dev)
{
  html_begin();
  int n = PortGroup;
  
  if (dev == 2211) {

    client.print("  </H2><H4><pre>Port: ");
    client.print(n);
    client.print("\t\tenabled: ");
    if (EE_DoMoAct.ports[n].Enable) client.print("yes");
    else client.print("no ");
    client.print("\tname: ");
    client.println(EE_DoMoAct.ports[n].Name);
    
    client.print("GPIO: ");
    
    if (EE_DoMoAct.ports[n].Gpio != EE_NOTASSIGNED){
      client.print(twoDigits(EE_DoMoAct.ports[n].Gpio));
    }
    else client.print("---");
    client.print("\tActive: ");
    if (EE_DoMoAct.ports[n].ActiveHigh) client.print("High");
    else client.print("Low ");
    client.print("\tStatus: ");
    if (PortStatus[n]) client.println("on");
    else client.println("off");
  
    client.print("  <br>sunset: ");
    if (EE_DoMoAct.ports[n].On_sunset) client.print("yes");
    else client.print("no ");

    client.print("\tOnHour: ");
    if (EE_DoMoAct.ports[n].On_hour != EE_NOTASSIGNED)  client.print(EE_DoMoAct.ports[n].On_hour);
    else client.print("---");
    
    client.print("\tOnMinute: ");
    if (EE_DoMoAct.ports[n].On_minute != EE_NOTASSIGNED)  client.print(EE_DoMoAct.ports[n].On_minute);
    else client.print("---");
    
    client.print("<br>sunrise: ");
    if (EE_DoMoAct.ports[n].Off_sunrise) client.print("yes");
    else client.print("no ");
    
    client.print("\tOffHour: ");
    if (EE_DoMoAct.ports[n].Off_hour != EE_NOTASSIGNED)  client.print(EE_DoMoAct.ports[n].Off_hour);
    else client.print("---");

    client.print("\tOffMinute: ");
    if (EE_DoMoAct.ports[n].Off_minute != EE_NOTASSIGNED)  client.println(EE_DoMoAct.ports[n].Off_minute);
    else client.println("---");
   
    s = true;
    
    for(j = 0; j < EE_NUMGROUPS ; j++) {
    
      if (EE_DoMoAct.ports[n].Group[j] != EE_NOTASSIGNED){
        if(s) {
          client.println("\n  Part of the following group(s)");
          s = false;
        }
        client.print("\tGroup: ");
        client.print(EE_DoMoAct.ports[n].Group[j]); // part of a group
        client.print("\tname: ");
        client.println(EE_DoMoAct.groups[EE_DoMoAct.ports[n].Group[j]].Name);
      }
    }
        
    DisplayWebWorkDays(EE_DoMoAct.ports[n].Day); // which days (bit 0 = sunday, 1 = ON, 0 = OFF)
  }
  else if (dev == 2311) {

    client.print("  </H2><H4><pre>Group: ");
    client.print(n);
    client.print("\tenabled: ");
    if (EE_DoMoAct.groups[n].Enable) client.print("yes");
    else client.print("no ");
    client.print("\tname: ");
    client.println(EE_DoMoAct.groups[n].Name);
    
    client.print("  membercount: ");
    client.print(EE_DoMoAct.groups[n].count);

    client.print("\tStatus: ");
    if (GroupStatus[n]) client.println("On");
    else client.println("off");
  
    client.print("  <br>sunset: ");
    if (EE_DoMoAct.groups[n].On_sunset) client.print("yes");
    else client.print("no ");

    client.print("\tOnHour: ");
    if (EE_DoMoAct.groups[n].On_hour != EE_NOTASSIGNED)  client.print(EE_DoMoAct.groups[n].On_hour);
    else client.print("---");
    
    client.print("\tOnMinute: ");
    if (EE_DoMoAct.groups[n].On_minute != EE_NOTASSIGNED)  client.print(EE_DoMoAct.groups[n].On_minute);
    else client.print("---");
    
    client.print("<br>sunrise: ");
    if (EE_DoMoAct.groups[n].Off_sunrise) client.print("yes");
    else client.print("no ");
    
    client.print("\tOffHour: ");
    if (EE_DoMoAct.groups[n].Off_hour != EE_NOTASSIGNED)  client.print(EE_DoMoAct.groups[n].Off_hour);
    else client.print("---");

    client.print("\tOffMinute: ");
    if (EE_DoMoAct.groups[n].Off_minute != EE_NOTASSIGNED)  client.print(EE_DoMoAct.groups[n].Off_minute);
    else client.print("---");
    
    DisplayWebWorkDays(EE_DoMoAct.groups[n].Day); // which days (bit 0 = sunday, 1 = ON, 0 = OFF)
  }
  else if (dev == 2411) {

    client.print("  </H2><H4><pre>Sensor channel: ");
    client.print(n);
    client.print("\tenabled: ");
    if (EE_DoMoAct.sensors[n].Enable) client.print("yes");
    else client.print("no ");
    client.print("\tStatus: ");
    if (SensorStatus[n]) client.println("On");
    else client.println("off");
    client.print("  name: ");
    client.print(EE_DoMoAct.sensors[n].Name);
    
    client.print("\tChangeSensorStatus: ");
    if (ChangeSensorStatus[n] == SENSOR_ON) client.println("switch ON");
    else if (ChangeSensorStatus[n] == SENSOR_OFF) client.println("switch OFF");
    else if (ChangeSensorStatus[n] == SENSOR_FLIP) client.println("switch FLIP");
    else if (ChangeSensorStatus[n] == SENSOR_NOACT) client.println("No action");

    s = true;
    for(j = 0; j < EE_NUMPORTS ; j++) {
      if (EE_DoMoAct.sensors[n].Ports[j] != EE_NOTASSIGNED){
        if (s) {
          client.println("  \nPorts assigned");
          s= false;
        }
        client.print("  \tPort: ");
        client.print(EE_DoMoAct.sensors[n].Ports[j]); // part of sensor
        client.print(" name: ");
        client.println(EE_DoMoAct.ports[EE_DoMoAct.sensors[n].Ports[j]].Name);
      }
    }
     
    DisplayWebWorkDays(EE_DoMoAct.groups[n].Day); // which days (bit 0 = sunday, 1 = ON, 0 = OFF)
  }

  client.println("  <br></pre></H4>");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Continue\">"); 
  html_end();
}

/**
 * Display workdays enabled / disabled
 * @param w: day variable from port, group or sensor
 */
void DisplayWebWorkDays(int w)
{  
  client.println("  \nWorkdays: ");
  client.print("  \tSunday\t\t");
  if (((w >> EE_SUNDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tMonday\t\t");
  if (((w >> EE_MONDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tTuesday\t\t");
  if (((w >> EE_TUESDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tWednesday\t");
  if (((w >> EE_WEDNESDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tThursday\t");
  if (((w >> EE_THURSDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tFriday\t\t");
  if (((w >> EE_FRIDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");

  client.print("  \tSaturday\t");
  if (((w >> EE_SATURDAY) & 0x1) == 0x1) client.println("enabled");
  else client.println("disabled");
  client.println();
}

/**
 * parse from display
 * Just change to the previous /parent website
 */
void ParseWebDisplayConf(int m){
  MenuWebSel = m / 100;
}

/**
 * Ask a double confirmation on a critical action
 * Wact: 
 * 321 make backup to EEPROM
 * 331 Restore EEPROM
 * 341 restart the system
 */
void WebDoubleAsk(int Wact)
{
  html_begin();
  client.println("  </H2><H2 style=\"color:red;\">");
  if (Wact == 341) client.println("  Reboot DoMoAct!<br><br>");
  else if (Wact == 331) client.println("  Restore from EEPROM!<br><br>");
  else if (Wact == 321) client.println("  Backup to EEPROM!<br><br>");
  client.println("  </H2><H2>");
  client.println("  Are you absolutely sure?<br><br>");
  client.print(linehead);
  client.println("value=\"Ayes\"/>&nbsp;&nbsp;&nbsp;&nbsp;YES <br> <br>");
  client.print(linehead);
  client.println("value=\"Ano\"/>&nbsp;&nbsp;&nbsp;&nbsp;NO <br> <br>");
  client.print(linehead);
  client.println("value=\"Acancel\"/>&nbsp;&nbsp;&nbsp;&nbsp;CANCEL <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"submit\">"); 
  html_end();
}
ParseValues DblChck_MenuItems[] {
  {"Ayes", 1},
  {"Ano",  2},
  {"Acancel", 3}
};

/**
 * parse response on double check
 * @param Wact : parse select.
 */
void ParseWebDoubleAsk(int Wact)
{
  int num = ParseMenu(Wact);
  if (num < 0) return;
   
  MenuWebSel = WEB_MNTS_MENU;

  if (num == 3) {
    if (WebDebug) Serial.println("Cancelled");
    return; 
  }

  else if (num == 2) {
    if (WebDebug) Serial.println("NO, cancelled");
    return;
  }

  else if (num == 1) {
    if (Wact == 321) {
      if (WebDebug) Serial.println("Backup to EEPROM");
      EE_WriteBackUpToEEprom(true);
    }   
    else if (Wact == 331) {
      if (WebDebug) Serial.println("Restore EEPROM");
      EE_ReadBackUpEeprom(true, false);
    }
    else if (Wact == 341) {
      if (WebDebug) Serial.println("Reset");
      delay(1000);
      NVIC_SystemReset();
    }
  }
}

/** 
 *  ask for port, group or sensor number
 *  
 *  @param ask : PORTS, GROUP or SENSOR
 *  
 */
void AskPortGroupSensor(int ask)
{
  html_begin();
  
  if (ask == PORTS) {
    client.println("  <label style=\"color:yellow; for=\"menu\">Port selection: </label> <br> <br>");
    for (j = 0; j < EE_NUMPORTS ; j++){

      client.print(linehead);
      client.print("value=\"A");
      client.print(j);
      client.print("\"/>&nbsp;&nbsp;&nbsp;&nbsp;");
      client.print(j);
      client.print("&nbsp;&nbsp;");
      client.print(EE_DoMoAct.ports[j].Name);
      if (EE_DoMoAct.ports[j].Enable) {
        client.print("&nbsp;&nbsp;enabled ");
        if(PortStatus[j]) client.println("&nbsp;&nbsp;ON");
        else client.println("&nbsp;&nbsp;OFF");
      }
      else client.println("&nbsp;&nbsp;disabled");

      client.print("<br>");
    }  
  }
  else if (ask == GROUP) {
    client.println("  <label style=\"color:yellow; for=\"menu\">Group selection: </label> <br> <br>");
    for (j = 0; j < EE_NUMGROUPS ; j++){

      client.print(linehead);
      client.print("value=\"A");
      client.print(j);
      client.print("\"/>&nbsp;&nbsp;&nbsp;&nbsp;");
      client.print(j);
      client.print("&nbsp;&nbsp;");
      client.print(EE_DoMoAct.groups[j].Name);
      if (EE_DoMoAct.groups[j].Enable) {
        client.print("&nbsp;&nbsp;enabled ");
        if(GroupStatus[j]) client.println("&nbsp;&nbsp;ON");
        else client.println("&nbsp;&nbsp;OFF");
      }
      else client.println("&nbsp;&nbsp;disabled");

      client.print("<br>");
    }  
  }
  else if (ask == SENSOR) {
    client.println("  <label style=\"color:yellow; for=\"menu\">Sensor channel selection: </label> <br> <br>");
    for (j = 0; j < EE_NUMSENSORS ; j++){

      client.print(linehead);
      client.print("value=\"A");
      client.print(j);
      client.print("\"/>&nbsp;&nbsp;&nbsp;&nbsp;");
      client.print(j);
      client.print("&nbsp;&nbsp;");
      client.print(EE_DoMoAct.sensors[j].Name);
      if (EE_DoMoAct.sensors[j].Enable) {
        client.print("&nbsp;&nbsp;enabled ");
        if(SensorStatus[j]) client.println("&nbsp;&nbsp;ON");
        else client.println("&nbsp;&nbsp;OFF");
      }
      else client.println("&nbsp;&nbsp;disabled");

      client.print("<br>");
    }  
  }  
  client.println("  <br>");
  client.print(linehead);
  client.println("value=\"Done\"/>&nbsp;&nbsp;&nbsp;&nbsp;cancel <br/> <br> </H2> <BR>");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end();
}

/**
 * Parse a string from the feedback
 */
int ParseString(int Parsenum)
{
  int num = -1;
  
  switch(Parsenum){
    
    case WEB_LOGIN_MENU:      

      num = ParseLine(&LoginMenuItems[0], sizeof(LoginMenuItems) /sizeof(struct ParseValues), PVAL);
      break;

    default :
      if (WebDebug){
        Serial.print(" Unkown parse value request ");
        Serial.println(Parsenum);
      }
      break;
  }
  
  return (num);
}

/**
 * Parse a number from the feedback
 */
int ParseNumber(int Parsenum)
{
  int num = -1;
  
  switch(Parsenum){
    
    case 0:      // default

      num = ParseLine(&MenuPGS_Items[0], sizeof(MenuPGS_Items) /sizeof(struct ParseValues), PNUM);
      break;
      
    default :
      if (WebDebug){
        Serial.print(" Unkown parse number request ");
        Serial.println(Parsenum);
      }
      break;
  }
  
  return (num);
}

/**
 * Parse a menu responds
 * 
 * @param menu : Menu order number
 * 
 * Will set MenuWebSel to the detected corresponding menu number
 * 
 * return 
 * -1 no match found
 * else corresponding match number
 */
int ParseMenu(int menu)
{
  int num;
  
  switch(menu) {
    case WEB_MAIN_MENU:
      num = ParseLine(&MainMenuItems[0], sizeof(MainMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
    
    case WEB_SWITCH_MENU:
      num = ParseLine(&SwitchWebMenuItems[0], sizeof(SwitchWebMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
 
    case WEB_CONF_MENU:
      num = ParseLine(&ConfigWebMenuItems[0], sizeof(ConfigWebMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
    
    case WEB_MNTS_MENU:
      num = ParseLine(&MM_MenuItems[0], sizeof(MM_MenuItems) /sizeof(struct ParseValues), PMENU);
      break;
      
#ifdef USERSENSORMENU == 1
    case WEB_USER_MENU:
      num = ParseLine(&UserMenuItems[0], sizeof(UserMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
#endif

    case 11 :     // port group or sensor ask
    case 12 :
    case 13 :
    case WEB_DASH_MENU:
      num = ParseLine(&PGSSelectItems[0], sizeof(PGSSelectItems) /sizeof(struct ParseValues), PMENU);
      break;
      
    case 22 :     // port configuration menu
      num = ParseLine(&PFMenuItems[0], sizeof(PFMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
    
    case 23 :     // group configuration menu
      num = ParseLine(&GFMenuItems[0], sizeof(GFMenuItems) /sizeof(struct ParseValues), PMENU);
      break;

    case 24 :     // sensor configuration menu
      num = ParseLine(&SNMenuItems[0], sizeof(SNMenuItems) /sizeof(struct ParseValues), PMENU);
      break;
      
    case 331:     // double check for critical action
    case 341:
    case 321:
      num = ParseLine(&DblChck_MenuItems[0], sizeof(DblChck_MenuItems) /sizeof(struct ParseValues), PMENU);
      break;

    case 999:    // Error menu
      num = ParseLine(&MenuPGS_Items[0], sizeof(MenuPGS_Items) /sizeof(struct ParseValues), PMENU);
      break;

    default:
      if (WebDebug) {
        Serial.print("ERROR: Nothing defined to parse for menu: ");
        Serial.println(menu);
      }  
      MenuWebSel = 999;
      return -1;
  }
    
  if (WebDebug) {
    Serial.print("ParseMenu >>>>>>>>>>>>>>>>>>>got number ");
    Serial.println(num);
  }
  if (num == -2) {
    if (WebDebug) Serial.println("Could not detect next menu");
  }
  else if (num != -1){
    MenuWebSel = num;
  }
  return num;
}


//////////////////////////// user sensor menu /////////////////////
#ifdef USERSENSORMENU == 1
bool WebTempCelsius = true;
float h, l, tempC;
String Webcf = "";

/**
 * Define the WebMenupages to sent to the user
 */
void UserSensorMenu()
{
  html_begin();
  client.println("  </H2> <H2 style=\"background-color:violet;\" > <label for=\"menu\">User Sensor menu: </label></H2> <H2> <br>");
  client.print(linehead);
  client.println("value=\"Q1\"/>&nbsp;&nbsp;&nbsp;&nbsp;Select Celsius<br> <br>");
  client.print(linehead);
  client.println("value=\"Q2\"/>&nbsp;&nbsp;&nbsp;&nbsp;Select Fahrenheit<br> <br>");
  client.print(linehead);
  client.println("value=\"done\"/>&nbsp;&nbsp;&nbsp;&nbsp;Cancel <br/> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end(); 
}

/**
 * Parse for Celcius or Fahrenheit
 */
void UserSensorParse()
{
  int num = ParseMenu(WEB_USER_MENU);
  if (num < 0) return;

  MenuWebSel = WEB_MAIN_MENU;
  if (num == 9) return;     // done
  
  if(num == 2){
    WebTempCelsius = false;
    Webcf = "&nbsp;F<br>";
  }
  else {
    WebTempCelsius = true;
    Webcf = "&nbsp;C<br>";
  }
  
  MenuWebSel = WEB_USER_SUB1;
}

/**
 * Ask for High alarm or cancel
 */
void UserSensorSub1()
{
  html_begin();

  client.println("  </H2> <H2 style=\"background-color:violet;\" > <label for=\"menu\">User Sensor: High Alarm </label> </H2> <H2><br> ");
  
  tempC = GetCurrentTemp();
  h = roundf(HighAlarm);
  l = roundf(LowAlarm);

  if (! WebTempCelsius) {
    tempC = (tempC * 9) / 5 + 32;
    h = (h * 9) / 5 + 32;
    l = (l * 9) / 5 + 32;
  }
  
  client.print("  Current temperature is ");
  client.print(tempC);
  client.println(Webcf);

  client.print("  <br>Current High Alarm is ");
  client.print(h);
  client.print(Webcf);

  client.println("  <br><label for=\"Ask\">Change to:</label>");
  client.print("  <input type=\"number\" id=\"Ask\" name=\"Select\" min=\"0\" max=\"");
  if (WebTempCelsius) client.print(85);
  else client.print((85 * 9)/5 + 32);
  client.print("\" value=\"0\"> &nbsp;&nbsp; <br> <br>");
  client.println("  <input type=\"radio\" id=\"Ask\" name=\"Done\" value=\"99\" /> &nbsp;&nbsp;No change High Alarm<br> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end(); 
}

/**
 * Parse for High alarm or cancel
 */
void UserSensorParse1()
{
  int num = ParseNumber(0);

  if (num < 0 ) return;     // not the right line, no action

  if (MenuPGS_Items[1].num != 99 ) {    // if NO change to High alarm
    if (num == 1) h = MenuPGS_Items[0].num;
  }

  MenuWebSel = WEB_USER_SUB2;
}

/**
 * Ask for low alarm or canncel
 */
void UserSensorSub2()
{
  html_begin();

  client.println("  </H2> <H2 style=\"background-color:violet;\" > <label for=\"menu\">User Sensor: Low Alarm</label></H2> <H2> <br>");

  client.print("  Current temperature is ");
  client.print(tempC);
  client.println(Webcf);

  client.print("  <br>Current Low Alarm is ");
  client.print(l); 
  client.print(Webcf);
  client.println("<br>");

  if (h <= l) {
    client.print("  </H2> <H3 style=\"color:Red; text-align:center;\">!! You MUST change LowAlarm due to the High Alarm setting at ");
    client.print(h);
    client.print(Webcf);
    client.println("</H3> <H2> <br>");
  }
  
  client.println("  <label for=\"Ask\">Change to:</label>");
  client.print("  <input type=\"number\" id=\"Ask\" name=\"Select\" min=\"0\" max=\"");
  client.print(h-1);
  client.print("\" value=\"0\"> &nbsp;&nbsp; <br> <br>");
  client.println("  <input type=\"radio\" id=\"Ask\" name=\"Done\" value=\"99\" /> &nbsp;&nbsp;Cancel AlarmSetting<br> <br> </H2> ");
  client.println("  <input type=\"submit\" class=\"button\" value=\"Submit\">"); 
  html_end(); 
}

/** Parse responds UserSensorSub2
 *  Parse Low Alarm level
 */
void UserSensorParse2()
{
  int num = ParseNumber(0);

  if (num < 0)  return;    // not the right line
  
  if (MenuPGS_Items[1].num == 99 ){   // cancel
    MenuWebSel = WEB_MAIN_MENU;  
    return;
  }
  
  MenuWebSel = WEB_USER_SUB3;
  
  if (num == 1) l = MenuPGS_Items[0].num;
}

/**
 * Ask confirmation question
 */
void UserSensorSub3()
{
  // show new Low and high alarm and get confirmation
  html_begin();

  client.println("  </H2> <H2 style=\"background-color:violet;<label for=\"menu\">User Sensor : confirm </label> </H2> <H2> <br> <br>");
  
  client.print("<br>High Alarm will be set to ");
  client.print(h);
  client.print(Webcf);
  client.println("<br>");

  client.print("Low Alarm will be set to ");
  client.print(l);
  client.print(Webcf);
  client.println("<br>");

  client.print(linehead);
  client.println("value=\"Q1\"/>&nbsp;&nbsp;&nbsp;&nbsp;Yes, proceed<br> <br>");
  client.print(linehead);
  client.println("value=\"Q2\"/>&nbsp;&nbsp;&nbsp;&nbsp;No, cancel<br> <br>");

  client.println("<input type=\"submit\" class=\"button\" value=\"Submit\">"); 

  html_end(); 
}

/**
 * parse confirmation question
 */
void UserSensorParse3()
{
  int num = ParseMenu(WEB_USER_MENU);
  if (num < 0) return;          // not the right line

  MenuWebSel = WEB_MAIN_MENU;
  
  if (num == 2) {
    if (WebDebug) Serial.println("Cancel");
    return;
  }
  
  if (num == 1) {
    
    if (! WebTempCelsius){    // turn to celcius
      l = roundf(((l - 32) * 5)/9);
      h = roundf(((h - 32) * 5)/9);
    }
    
    if (WebDebug){   
      Serial.print("alarm high");
      Serial.println(h);
      Serial.print("alarm low ");
      Serial.println(l);
    }
    
    SetAlarm(h,l);
  }
}
#endif
