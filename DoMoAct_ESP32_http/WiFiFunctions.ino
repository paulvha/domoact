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

/**
 * Try to connect to WIFI
 */
bool ConnectToWifi()
{

  WiFi.begin(ssid, pass);

  for (byte i = 0 ; i < MAX_RETRY; i++) {
    Serial.print("Waiting to Connect to ");
    Serial.println(ssid);

    if ( WiFi.status() != WL_CONNECTED) {
      delay(1000);
    }
    else {
      Serial.println("");
      Serial.println("WiFi connected.");
      return true;
    }
  }

  Serial.print("Failed to Connect to ");
  Serial.println(ssid);

  return false;
}

/**
 * Initialize the server with callbacks and start server
 * callbacks : in tab WebSupportFunctions
 */
void StartServer()
{
  server.on("/", handleRoot);             // first time around
  
  server.on("/DMACT", handleResp);        // respondse on menu's
  
  server.on("/favicon.ico", []() {        // some browsers ask for icon.
    server.send(200, "text/plain", "");
  });
  server.onNotFound(handle404);           // expect the un-expected
  
  server.begin();
  Serial.println("HTTP server started");
}

void EndServer()
{
  server.stop();
}

/**
 * display on Serial after start, init and connecting
 */
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.println("====================================================");
  Serial.println("THIS IS AN HTTP SERVER");
  Serial.print("For DoMoAct: open a browser to http://");
  Serial.println(ip);
  Serial.println("====================================================");
}
