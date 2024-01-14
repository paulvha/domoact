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
    
 Small Part has been taken from the Arduino Simple Webserver example
*/

/**
 * Try to connect to WIFI
 */
void ConnectToWifi()
{
  int status = WL_IDLE_STATUS;
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!. Freeze.");
    MessageMatrix("E2",M_NO);
    // don't continue
    while(1) {
      delay(3000);
      digitalWrite(led,LOW);  
      delay(3000);
      digitalWrite(led,HIGH);  
    }
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    
    // wait 5 seconds for try next connection:
    if (status != WL_CONNECTED) delay(5000);
  }
}

void StartServer()
{
  server.begin();   // start the web server on port 80
  delay(2000);      // wait 2 seconds
}

void EndServer()
{
  server.end();
}

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
  Serial.print("For DoMoAct: open a browser to http://");
  Serial.println(ip);
  Serial.println("====================================================");
}
