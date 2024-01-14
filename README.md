#DoMoAct versions
===========================================================

## Overview
DoMoAct is a system to can switch a GPIO's high/low based on comparing the requested switch time against the current time. The current time is taken from the time. Time is initialized from an NTP server at start-up and every night after.The switch time can be set as a hard / fixed time or based on the daily sunset / sunrise time on your location. DayLightSaving is handled as well. <br>

You can also add sensors to switch a GPIO. One could measure the temperature and if too high, switch on a blower, or measure the waterlevel and switch a pump if level is too high or too low. As an example there is a temperature sensor DS1820.<br>

DoMoAct has a serial-menu which is the primary source for configuration and a WEB-menu that is mend for the user. Next to configuration there are a number of parameters that can be set for customization before compile. E.g. It allows for  more ports, groups and sensors, you can set username and password protection for access to DoMoAct, use an LCD/ led Matrix to show status and switch information, store sensor specific information in the EEPROM or SPIFFS. etc.

It contains a buffer of the last 50 switches actions (this can be extended) that can recalled with the serial-menu. The Web-interface can be set to different background themes.

##Different versions
There are 3 versions which do functionally the same, the look&feel is the same,  but parts of the code are slidly different.

* Using [UNO-R4 Wifi](https://github.com/paulvha/domoact/tree/main/UNOR4). This is using an HTTP server and the on-board LED Matrix.
* Using [ESP32-HTTPS](https://github.com/paulvha/domoact/tree/main/ESP32-HTTPS). This is using an external [HTTPS-server](https://github.com/fhessel/esp32_https_server). It is reaction slower due to checks and structure of the HTTPS server.
* Using [ESP32-HTTP](https://github.com/paulvha/domoact/tree/main/ESP32-HTTP). Using the standard ESP32 HTTP-server implementation.

Please refer to the individual sub-folders for more information.

## Auteur
Paulvha@hotmail.com

## Product Versions
* V1.1 / January 2024 initial version

## License Information
* MIT: https://rem.mit-license.org

Distributed as-is; no warranty is given.

