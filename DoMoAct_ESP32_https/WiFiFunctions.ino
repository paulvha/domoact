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

  cert = new SSLCert(certData, certLength, pkData, pkLength);

  // We can now use the certificate to setup our server as usual.
  secureServer = new HTTPSServer(cert);

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
 * Initialize the server with callback and start server
 * callbacks : in tab WebSupportFunctions
 */
void StartServer()
{
  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot    = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * node404     = new ResourceNode("", "GET", &handle404);

  // Register the echo handler. You can use the same handler function for multiple
  // nodes. Note also that we now have three resource nodes for the / URL, so
  // the server uses only the method to distinguish between them.
  ResourceNode * nodeRespPut  = new ResourceNode("/DMACT", "PUT", &handleResp); 
  ResourceNode * nodeRespPost = new ResourceNode("/DMACT", "POST", &handleResp);
  ResourceNode * nodeRespGet = new ResourceNode("/DMACT", "GET", &handleResp);

  // Add the root node to the server
  secureServer->registerNode(nodeRoot);

  // Add the nodes for the response service
  secureServer->registerNode(nodeRespPut);
  secureServer->registerNode(nodeRespPost);
  secureServer->registerNode(nodeRespGet);
  
  // Add the 404 not found node to the server.
  secureServer->setDefaultNode(node404);

  Serial.println("Starting server...");
  secureServer->start();
  if (secureServer->isRunning()) {
    Serial.println("Server ready.");
  }
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
  Serial.print("For DoMoAct: open a browser to https://");
  Serial.println(ip);
  Serial.println("====================================================");
}
