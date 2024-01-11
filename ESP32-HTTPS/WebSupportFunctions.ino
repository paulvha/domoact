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

    The functions in this tab provide the support functions to sent and parse the responds from
    a connected client
*/

/**
 * entry point for webmenu
 * @param Menuact : 
 *  true: start with selecting menu
 *  false: responds expected, start with analysing responds.
 */
void StartWebMenu(bool Menuact){
  bool SetMenu = Menuact;    
  // if more time has passed / reset to main menu
  if (CurTime - LastMenuWebSelectTime > WEBMENURESET || MenuWebSel == CF_NOTASSIGNED)
  {
    if (AskLogin) MenuWebSel = WEB_LOGIN_MENU;
    else MenuWebSel = WEB_MAIN_MENU;
    FirstMenuSet = true;
    SetMenu = true;
  }
  
  LastMenuWebSelectTime = CurTime;
  
  if (SetMenu) WebMenuSelect(); // set menu
  else WebParseSelect();        // handle response
}

/**
 * determine which web page to sent next
 */
void WebMenuSelect()
{
  if (WebDebug){
    Serial.print("Sending WebMenuSel >> ");
    Serial.println(MenuWebSel); 
  }
  
  switch(MenuWebSel) {
   
    case WEB_MAIN_MENU :      // mainmenu
      FirstMenuSet = false;
      MainWebMenu();
      break;
      
    case WEB_SWITCH_MENU :    // manual switch
      SwitchWebMenu();
      break;
    
    case WEB_MNTS_MENU :      // Maintenance menu
      MaintenanceWebMenu();
      break;
    
    case WEB_CONF_MENU :      // system configuration menu
      ConfigWebMenu();
      break;

    case WEB_LOGIN_MENU :     // system login menu
      FirstMenuSet = false;
      LoginWebMenu();
      break;

    case WEB_DASH_MENU :     // dashboard
      DashWebMenu();
      break;
    
    case 22 :                // port configuration
      MenuPortConfig();
      break;

    case 23 :                // group configuration
      MenuGroupConfig();
      break;
   
    case 24 :                // sensor configuration
      MenuSensorConfig();
      break;
    
    case 11 :   // toggle port  
    case 221:   // display port
    case 222:   // enable port
    case 223:   // disable port
    case 224:   // add to group
    case 225:   // remove port from group
    case 2341:  // get port to add
    case 2351:  // get port to remove
    case 2441:  // add to sensor
    case 2451:  // remove port from sensor
      AskPortGroupSensor(PORTS);
      break;
      
    case 12 :   // toggle group
    case 231:   // display group
    case 232:   // enable group
    case 233:   // disable group
    case 234:   // add to group
    case 235:   // remove port from group
    case 2241:  // get group to add
    case 2251:  // get group to remove
      AskPortGroupSensor(GROUP);
      break;
      
    case 13 :  // toggle sensor
    case 241:  // display sensor
    case 242:  // enable sensor
    case 243:  // disable sensor
    case 244:  // add to sensor
    case 245:  // remove port from sensor
      AskPortGroupSensor(SENSOR);
      break;

    case 2411:  // display Sensor
    case 2311:  // display group
    case 2211:  // display port
      WebDisplayConf(MenuWebSel);
      break;
      
    case 321:   // backup to SPIFF
    case 331:   // restore SPIFF
    case 341:   // restart DoMoAct
      WebDoubleAsk(MenuWebSel);
      break;
    case 35:                  // webbackground
      ChangeBackgroundMenu();
      break;
      
#if USERSENSORMENU == 1

    case WEB_USER_MENU:      // optional user sensor menu
      UserSensorMenu();
      break;
    case WEB_USER_SUB1:      // optional user sensor sub menu
      UserSensorSub1();
      break;
    case WEB_USER_SUB2:      // optional user sensor sub menu
      UserSensorSub2();
      break;    
    case WEB_USER_SUB3:      // optional user sensor sub menu
      UserSensorSub3();
      break; 
#endif
    
    default:
      if (WebDebug){
        Serial.print("ERROR : Menu ");
        Serial.print(MenuWebSel);
        Serial.println(" Not known.");
      }
      MenuWebSel = 999;
      // notice the fall through !!
    case 999:  // error
      MenuError();
      break;
  }
  
  // which parser to use on responds from page sent?
  ParseSelect = MenuWebSel;
}

/**
 * determines which parser to use to parse the responds from the client
 */
void WebParseSelect()
{
  if (WebDebug){
    Serial.print("ParseSelect >> ");
    Serial.println(ParseSelect);       
  }
  
  switch(ParseSelect) {
    
   case WEB_MAIN_MENU:    // main menu
   case WEB_SWITCH_MENU:  // manual switch
   case WEB_CONF_MENU:    // configuration
   case 22:               // port configuration
   case 23:               // group configuration
   case 24:               // sensor configuration
    ParseMenu(ParseSelect);
    break;
   
   case 999:              // error menu
    MenuWebSel = WEB_MAIN_MENU;
    break;
   
   case WEB_LOGIN_MENU:   // parse Login request
    LoginWebParse();
    break;
   
   case WEB_MNTS_MENU:    // maintenance menu
    MaintenanceParse();
    break;

   case WEB_DASH_MENU :   // dashboard
    DashWebParse();
    break;
   
   case 11:     // toggle port
   case 12:     // toggle group
   case 13:     // toggle sensor
    SwitchParse(ParseSelect);
    break;

   case 221:    // disp port
   case 222:    // enable port
   case 223:    // disable port
   case 224:    // add port to group
   case 225:    // remove port from group
   case 231:    // disp group
   case 232:    // enable group
   case 233:    // disable group
   case 234:    // add port to group
   case 235:    // remove port from group
   case 241:    // disp sensor
   case 242:    // enable sensor
   case 243:    // disable sensor
   case 244:    // add port to sensor
   case 245:    // remove port from sensor
    MenuConfParse(ParseSelect);
    break;
   
   case 2241:  // get group to add to port
   case 2251:  // get group to remove port from 
   case 2341:  // get port to add group
   case 2351:  // get port to remove from  group
   case 2441:  // get port to add to sensor
   case 2451:  // get port to remove from sensor
    MenuPortAddRemoveGroupSensor(ParseSelect);
    break;
   
   case 2411:  // display sensor
   case 2311:  // display group
   case 2211:  // display port
    ParseWebDisplayConf(ParseSelect);
    break;
   
   case 331:   // restore EEPROM
   case 321:   // backup to EEPROM
   case 341:   // restart DoMoAct
    ParseWebDoubleAsk(ParseSelect);
    break; 
   
   case 35:                   // webbackground
    ChangeBackgroundParse();
    break;

#if USERSENSORMENU == 1
   case WEB_USER_MENU:       // optional user sensor menu
    UserSensorParse();
    break;

   case WEB_USER_SUB1:      // optional user sensor sub menu1
    UserSensorParse1();
    break;
    
   case WEB_USER_SUB2:      // optional user sensor sub menu2
    UserSensorParse2();
    break;
    
   case WEB_USER_SUB3:      // optional user sensor sub menu3
    UserSensorParse3();
    break; 
#endif
       
   default:
    if (WebDebug){
      Serial.print("ERROR : Parse ");
      Serial.print(ParseSelect);
      Serial.println(" Not known");
    }  
    MenuWebSel = 999;
    break;
  }
  WebMenuSelect();
}

/**
 * add different times to HTML webpage
 */
void AddTrailer() 
{
  RES->print("\nVersion ");
  RES->print(CF_VERSION);
  RES->println("<br>");
  RES->print(twoDigits(CurTime / 60));
  RES->print(":");
  RES->print(twoDigits(CurTime % 60));
  RES->print("&nbsp;&nbsp;sunrise: ");
  RES->print(twoDigits(sunrise / 60));
  RES->print(":");
  RES->print(twoDigits(sunrise % 60));
  RES->print("&nbsp;&nbsp;sunset: ");
  RES->print(twoDigits(sunset / 60));
  RES->print(":");
  RES->print(twoDigits(sunset % 60));
}

/**
 * set common header for HTML page
 */
void html_begin_common() 
{
  RES->println();
  RES->println("<!DOCTYPE html>");
  RES->println("<!-- paulvha / December 2023 / Ver 1.10 -->");
  RES->println("<HTML lang=\"en-US\">");
  RES->println(" <HEAD>");
  RES->println("  <TITLE> DoMoAct </TITLE>");
  RES->println("  <meta charset=\"utf-8\">");
  RES->println("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=yes\">");
}
/**
 * set header for HTML webpage
 */
void html_begin() 
{
  html_begin_common();

  if (CF_DoMoAct.WebBackground == 1) { // starfield
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/starfield.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("  <div id=\"container\"></div>");
    RES->println("  <FORM ACTION = \"DMACT\" method=\"POST\"> "); //<H2>");
    RES->println("   <div id=\"front\"> <H2>");
    RES->println("    </H2> <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\" href=\"#\" onclick=\"randomise()\"> ====== DoMoAct ====== </H2><H2><br>");
  }
 
  else if(CF_DoMoAct.WebBackground == 2) { // scrolling
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/scrolling.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("   <FORM class=front ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"wrapper\">");
    RES->println("    </H2> <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><H2><br>");
  }
  
  else if(CF_DoMoAct.WebBackground == 3) { // sliding
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/sliding.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("   <div class=\"bg\"></div>");
    RES->println("   <div class=\"bg bg2\"></div>");
    RES->println("   <div class=\"bg bg3\"></div>");
    RES->println("   <FORM class=front ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"content\">");
    RES->println("    </H2><H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><H2><br>");
  }
  
  else if(CF_DoMoAct.WebBackground == 4) { // gradient
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/gradient.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("  <div class=\"d-flex flex-column justify-content-center w-100 h-100\">");
    RES->println("   <div class=\"d-flex flex-column justify-content-center align-items-left\">");
    RES->println("    <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><br>");
    RES->println("    <FORM ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"front\"> <H2>");

  }
  else if(CF_DoMoAct.WebBackground == 5) { // gradient waves
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/gradientwaves.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("  <div class=\"wave\"></div>");
    RES->println("  <div class=\"wave\"></div>");
    RES->println("  <div class=\"wave\"></div>");

    RES->println("    <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><br>");
    RES->println("    <FORM ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"front\"> <H2>");
  }
  else if(CF_DoMoAct.WebBackground == 6) { // fireworks
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/fireworks.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <div class=\"firework\"></div>");
    RES->println("    <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><br>");
    RES->println("    <FORM ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"front\"> <H2>");
  }
    else if(CF_DoMoAct.WebBackground == 7) { // stars
    RES->println("  <link rel=\"stylesheet\" href=\"https://paulvha.github.io/domoact/stars.css\">");
    RES->println(" </HEAD>");
    RES->println();
    RES->println(" <BODY>");
    RES->println("  <div class=\"galaxy\">");
    RES->println("    <div class=\"stars1\"></div>");
    RES->println("    <div class=\"stars2\"></div>");
    RES->println("    <div class=\"stars3\"></div>");
    //RES->println("  </div>");
    RES->println("    <H2 style=\"text-align:center; font-style:italic; BackGround-color:DarkGoldenRod;\"> ====== DoMoAct ====== </H2><br>");
    RES->println("    <FORM ACTION = \"DMACT\" method=\"POST\"> "); 
    RES->println("    <div class=\"front\"> <H2>");
  }
}

/**
 * close out HTML page
 */
void html_end() 
{
  if(CF_DoMoAct.WebBackground == 1) { // starfield
    RES->println(" </div>");
    RES->println(" <!-- original source :https://github.com/dwmkerr/starfield -->");
    RES->println(" <script src=\"https://paulvha.github.io/domoact/starfield.js\"></script>");
   
    RES->println(" <script>");
    RES->println("   var container = document.getElementById('container');");
    RES->println("   var starfield = new Starfield();");
    RES->println("   starfield.stars = 500;      //  The number of stars.");
    RES->println("   starfield.minVelocity = 5;  //  The minimum star velocity in pixels per second.");
    RES->println("   starfield.maxVelocity = 15; //  The maximum star velocity in pixels per second. "); 
    RES->println("   starfield.initialise(container);");
    RES->println("   starfield.start();");
  
    RES->println("   function randomise() {");
    RES->println("     starfield.stop();");
    RES->println("     starfield.stars = Math.random()*1000 + 50;");
    RES->println("     starfield.minVelocity = Math.random()*30+5;");
    RES->println("     starfield.maxVelocity = Math.random()*50 + starfield.minVelocity;");  
    RES->println("     starfield.start();");
    RES->println("   }");
    RES->println(" </script>");
    RES->println(" </FORM>");
    RES->print("<p style=\"BackGround-color:GREY; color:GhostWhite; text-align:center;\">");
    AddTrailer();
    RES->println("</p>");
    RES->println(" </BODY>");
    RES->println("</HTML>");
    RES->println();
  }
  else if(CF_DoMoAct.WebBackground == 2 || CF_DoMoAct.WebBackground == 3 || CF_DoMoAct.WebBackground == 5 || CF_DoMoAct.WebBackground == 6 ){ // scrolling & sliding & waves & fireworks 
    RES->println(" <br> <br> <h1>");
    AddTrailer();
    RES->println("</h1>");
    
    RES->println("    </div>");
    RES->println("   </FORM>");
    RES->println(" </BODY>");
    RES->println("</HTML>");
    RES->println();
  }
  else if(CF_DoMoAct.WebBackground == 4) { // gradient
    
    RES->println("     </div>");
    RES->println("    </FORM>");
     RES->println(" <br> <br> <h1>");
     AddTrailer();
     RES->println("</h1>");
    RES->println("   </div>");
    RES->println("  </div>");
    RES->println(" </BODY>");
    RES->println("</HTML>");
    RES->println();
  }
  else if( CF_DoMoAct.WebBackground == 7 ){ // stars 
    RES->println("    </div>");
    RES->println("   </FORM>");
        RES->println(" <br> <br> <h1>");
    AddTrailer();
    RES->println("</h1>");
    RES->println("  </div>");
    RES->println(" </BODY>");
    RES->println("</HTML>");
    RES->println();
  }
}

/////////////////////// callback from the HTTP server ///////////////////
void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  // Status code is 200 OK by default.
  MessageLCD("Client active",M_NO);
  RES = res;
  if (WebDebug) Serial.println("ROOT");
  StartWebMenu(true);
  MessageLCD("",M_NO);
}

/**
 * Entry point for responses
 * Gather the response whether GET or POST and call WebParseSelect()
 */
void handleResp(HTTPRequest * req, HTTPResponse * res) {
  char buffer[50];
  MessageLCD("Client active",M_NO);
  // set global pointer
  RES = res;
  
  String method = req->getMethod().c_str();
  
  if(strcmp(method.c_str(),"GET") == 0) {
    if (WebDebug) Serial.println("GET request");
    currentLine = req->getRequestString().c_str();
  }
  
  else if(strcmp(method.c_str(),"PUT") == 0) {
    if (WebDebug) Serial.println("PUT request");
    // get it from the body
    // **** not implemented ***
    // discard the rest
    req->discardRequestBody();
  }
  
  else if(strcmp(method.c_str(),"POST") == 0) {
    if (WebDebug) Serial.println("Post request");
    currentLine = req->getRequestString().c_str() + String("?");
    
    // HTTPReqeust::requestComplete can be used to check whether the
    // body has been parsed completely.
    while(!(req->requestComplete())) {
      // HTTPRequest::readBytes provides access to the request body.
      // It requires a buffer, the max buffer length and it will return
      // the amount of bytes that have been written to the buffer.
      size_t s = req->readBytes((byte*)buffer, 50);
      buffer[s] = 0x0;
      currentLine += String(buffer);
    }
  }
  
  if (WebDebug) Serial.println(currentLine);
  StartWebMenu(false);
  MessageLCD("",M_NO);
}

/**
 * invalid request
 */
void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  MessageLCD("Client E404",M_NO);
  
  req->discardRequestBody();
  
  // set global pointer
  RES = res;
  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  MenuWebSel=999;
  StartWebMenu(true);
  MessageLCD("",M_NO);
}

///////////////////////////////////////////////////////////////////////////////////////////
// PARSING FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////

/**
 * Parse a string from the feedback
 * @param Parsenum : parser to use
 * String result is stored in parser
 * 
 * return :
 * -1 error/ not found
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
 * @param Parsenum : parser to use
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
 * @param menu : Menu order number / parser to use
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
      
#if USERSENSORMENU == 1
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


/**
 * parse feedback line
 * 
 * @param p : point to the lookup table
 * @param pnum: number of entries in lookuptable
 * @param detect : either PMENU, PNUM or PVAL
 * 
 * 
 * Return :
 * -1 : invalid line (missing LINEID)
 * -2 : Lookup item not found (PMENU)
 * -3 : Lookup VALUE could not be found
 * 
 * else the
 * p[x].num[x] to be used for menu selection OR number
 * p[x].val if PVAL was requested and p[x].num == 1;
 * 
 */
int ParseLine(struct ParseValues *p , int pnum, int detect)
{
  int i, l, pos, ret;

  // check for line with DoMoAct feedback
  if (currentLine.indexOf(LINE_ID) == -1) return -1;

  // lookup menu selection
  if (detect == PMENU) {
    for(i = 0 ; i < pnum; i++){
      // found the lookup return the menu number
      if (currentLine.indexOf(p[i].lookup) != -1) return p[i].num;
    }
    
    // not found
    return -2;
  }

  // try to get a value GET /DMACT?Hour=18&Min=36 HTTP/1.1
  l = currentLine.length();
  i = 0;
  ret = 0;
  while (i < pnum)
  {
    p[i].num = -1;
    
    pos = currentLine.indexOf(p[i].lookup);

    // not found
    if (pos == -1) {
      i++;
      continue;
    }

    pos += strlen(p[i].lookup) + 1;

    if (pos >= l) return -3;

    Value = "";
  
    while(currentLine[pos] != ' ' && currentLine[pos] != '&' && pos < l) Value += currentLine[pos++];

    // store value
    if (detect == PNUM) p[i].num = Value.toInt(); 
    
    else if (detect == PVAL) {
      for (int j = 0 ; j < MAX_CH_LENGTH ,j < Value.length(); j++) p[i].val[j] = Value[j];
      p[i].num = 1;
    }

    ret++;  // count matches found
    i++;    // next entry of lookup table
  
  }
  return ret;
}
