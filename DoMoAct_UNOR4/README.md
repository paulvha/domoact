#DoMoAct UNO-R4-WiFi
===========================================================

## Overview
DoMoAct is a system to can switch a GPIO's high/low based on comparing the requested switch time against the current time. The current time is taken from the time. The on-board RTC is initialized from an NTP server at start-up and every night after.The switch time can be set as a hard / fixed time or based on the daily sunset / sunrise time on your location. It can also handle DaylightSaving for Western Europe or USA<br>

You can also add sensors to switch a GPIO. One could measure the temperature and if too high, switch on a blower, or measure the waterlevel and switch a pump if level is too high or too low. As an example there is a temperature sensor DS1820.<br>

DoMoAct has a serial-menu which is the primary source for configuration and a WEB-menu that is mend for the user. Next to configuration there are a number of parameters that can be set for customization before compile. E.g. It allows for  more ports, groups and sensors, you can set username and password protection for access to DoMoAct, use an LED matrix to show status and switch information, store sensor specific information in the EEPROM. etc.

It contains a buffer of the last 50 switches actions (this can be extended) that can recalled with the serial-menu. The Web-interface can be set to different background themes.

Detailed information and how to get started is in the included DoMoAct-manual.odt in extras-folder <br>

The processor board is an [UNO-R4 WiFi](https://store.arduino.cc/products/uno-r4-wifi) from Arduino. A board with relays can be connected to the GPIO's. There are many relays boards around.
See the hardware section in the DoMoAct-manual.odt for more important information.<br>

Next to this version there is also a version for ESP32-HTTP and ESP32-HTTPS.

## Documentation
Check the extras folder for the manual
* **[DoMoAct Manual](./extras/DomoAct-manual.odt)**
* **[Planning Spreadsheet](./extras/DoMoAct_form.xls)**

## Auteur
Paulvha@hotmail.com

## Product Versions
* V1.1 / January 2024 initial version


## License Information
* MIT: https://rem.mit-license.org

Distributed as-is; no warranty is given.

