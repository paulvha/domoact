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

  Simple ring or circular buffer. 
  
  The routines in this tab will store CB_MAXENTRIES (defined in DoMoAct.h) of the last switch activities
  Either group, ports or sensors. Can be displayed within the serial menu maintenance and Led Matrix
*/

////////////// common used variables //////////////////////

uint8_t head = 0;
uint8_t tail = 0;
String m;
String sled;           // display on Led or serial
char CBuffer[CB_MAXENTRIES][CB_ENTRYSIZE];

/**
 * clear the buffer 
 */
void CBuf_clear()
{
  head = 0;
  tail = 0;
}

/** 
 *  Update circular buffer with switch action
 *  Show on serial as well if DisplaySwitch was set in menu
 *  
 *  @param dev : PORT, GROUP or SENSOR
 *  @param num : number
 *  @param act : true = ON , false = OFF
 *  
 *  use of sled string reduce delay in shifting left on Led
 */
void CB_UpdateBuffer(byte dev, int num, bool c_act )
{
  RTCTime currentTime;
  RTC.getTime(currentTime);
  m = String(currentTime);
  sled ="";
  
  // below could be used instead with only time would be required
  //m = twoDigits(GetFromRTC(R_HOUR)) + String(":") + twoDigits(GetFromRTC(R_MINUTE));

  if (dev == PORTS) {m += String(" port "); sled += String("P");}
  else if (dev == GROUP) {m += String(" group "); sled += String("G");}
  else {m += String(" sensor "); sled += String("S");}

  if (c_act) { m += String(num) + String(" On"); sled += String(num) + String(" ON");}
  else { m += String(num) + String(" Off"); sled +=String(num) + String(" Off");}

  // add 0x0
  uint8_t str_len = m.length() + 1; 

  // Prepare the character array (the buffer) 
  char char_array[str_len];

  // Copy it over 
  m.toCharArray(char_array, str_len);

  // add to buffer
  CBuf_add_entry(char_array, str_len); 

  // if showing switch on serial and Led Matrix is enabled in menu
  if (CF_DoMoAct.DisplaySwitch){
    Serial.println(m);
    
    //MessageMatrix(m.substring(m.indexOf(" ") + 1), M_LEFT);
    MessageMatrix(sled, M_LEFT);
  }
}

/**
 * Add a new entry to the circular buffer
 * 
 * @param p : message to add
 * @param l : length of message
 */
void CBuf_add_entry(char *p, uint8_t l)
{
  uint8_t j;
  static bool Uptail = false;

  // increase tail if head is more than CB_MAXENTRIES ahead
  if (Uptail) tail++;

  // did we reach end of table ?
  if (tail == CB_MAXENTRIES) {
    tail = 0;
    Uptail = false;  //head is NOT more than CB_MAXENTRIES ahead
  }
  
  // add message
  for (j = 0 ; j < l && j < CB_ENTRYSIZE; j++) {
     CBuffer[head][j] = p[j];
  }
  
  // make the rest 0x0
  while(j < CB_ENTRYSIZE) CBuffer[head][j++] = 0x0;

  head++;
  
  // reached end of table ? loop around !
  if (head == CB_MAXENTRIES) {
    head = 0;
    Uptail = true;   //head is more than CB_MAXENTRIES ahead
  }
}

/** 
 *  display entries in buffer to Serial
 */
void CBuf_disp_entries()
{
  uint8_t j, i,  s = tail;
  uint8_t cnt = head;
  if (tail != 0) cnt += CB_MAXENTRIES;

  for (i = tail, s = tail; i < cnt; i++) {

    for (j = 0 ; j < CB_ENTRYSIZE; j++) {
      if (CBuffer[s][j] != 0x0) Serial.print(CBuffer[s][j]);
      else break;
    }
    
    Serial.println();
    s++;
    if (s == CB_MAXENTRIES)  s = 0;
  }
}
