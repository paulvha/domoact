/**
    Written by Paulvha December 2023
    Released under MIT licence.

    Parts taken from the original Alarm.ino sketch in the DallasTemperature library
*/
///////////////////////////////////////////////////////
// To use this example you need to :
// Configure the sensor channel in the menu configuration structure (set port and enable)
// Select the right Sensor channel (set as 0 below)
// Select the right ONE_WIRE_BUS or pin. (set as pin 7 below)
// Install OneWire library             //Click here to get the library: http://librarymanager/All#OneWireHub
// Install DallesTemperature library.  //Click here to get the library: http://librarymanager/All#DallasTemperature
// connect a DS1820  (see https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/)
// Define SENSOREXAMPLE in the tab DoMoAct.h tab to 1
// This example also has a serial menu and web-menu defined. For that you need to define USERSENSORMENU in the tab DoMoAct.h as 1
///////////////////////////////////////////////////////

#if SENSOREXAMPLE == 1     // Change SENSOREXAMPLE in the tab DoMoAct.h tab

#include <OneWire.h>
#include <DallasTemperature.h>

// which sensor-table entry to use. There max EE_NUMSENSORS (see tab DoMoAct.h)
#define SensorChannel 0                               //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// alarm levels in celcius. For test I kept the levels close.
float HighAlarm = 23;
float LowAlarm  = 21;

// Data wire is plugged into port 7 on the Arduino
#define ONE_WIRE_BUS 7

// indicate that device was found
bool DeviceFound = false;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// to hold device addresses
DeviceAddress DS_Thermometer;

bool InitDallas()
{
  Serial.println("Dallas Temperature Demo");

  // Start up the library
  sensors.begin();

  if(sensors.getDeviceCount() == 0) {
    Serial.println("NO devices found");
    return false;
  }
  
  DeviceFound = true;
  
  // Display device count on the bus
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // search for devices on the bus and assign based on an index.
  if (!sensors.getAddress(DS_Thermometer, 0)) {
    Serial.println("Unable to find address for Device 0"); 
    return false;
  }
  
#ifdef USERSENSORMENU == 1
  if (EE_DoMoAct.sensors[SensorChannel].UserFloat1 != EE_NOTASSIGNED) //<<<<<<<<<EEPROM usage
    HighAlarm = EE_DoMoAct.sensors[SensorChannel].UserFloat1;

  if (EE_DoMoAct.sensors[SensorChannel].UserFloat2 != EE_NOTASSIGNED) //<<<<<<<<<EEPROM usage
    LowAlarm = EE_DoMoAct.sensors[SensorChannel].UserFloat2;
#endif

  printAlarms();
  
  return true;
}

void printAlarms()
{
  char temp;
  temp = HighAlarm;
  Serial.print("High Alarm: ");
  Serial.print(temp, DEC);
  Serial.print("C/");
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.print("F | Low Alarm: ");
  temp = LowAlarm;
  Serial.print(temp, DEC);
  Serial.print("C/");
  Serial.print(DallasTemperature::toFahrenheit(temp));
  Serial.println("F");
}

/**
 * regular call this routine to check for temperature limits
 * and trigger the SensorChannel if needed.
 */
void checkAlarm()
{
  static bool DispAlarm = true;                   // display HIGH or LOW alarm only ONCE when it happens

  if (!DeviceFound) return;
  
  sensors.requestTemperatures();
  float tempC = roundf(sensors.getTempC(DS_Thermometer));
  
#if 0  // enable for debug only
  Serial.print("Temp (rounded) C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));
#endif

  // will display the temperature in Celsius
  MessageMatrix(twoDigits(tempC),M_NO);

  if (tempC > HighAlarm) {
    if (DispAlarm) {
      Serial.println("High alarm");
      DispAlarm = false;
    }
    
    ChangeSensorStatus[SensorChannel] = SENSOR_ON;     //<<<<<<<<<<<< switch usage <<<<<<<<<<<<<<
  }

  else if (tempC < LowAlarm) {
    if (DispAlarm) {
      Serial.println("LOW alarm");
      DispAlarm = false;
    }
    
    ChangeSensorStatus[SensorChannel] = SENSOR_OFF;    //<<<<<<<<<<<< switch usage<<<<<<<<<<<<<
  }
  else
    DispAlarm = true;     // re-enable
  
}

#ifdef USERSENSORMENU == 1

/**
 * SPECIAL : SEE CHAPTER 9.4 IN THE DoMoAct manual
 * 
 * example of Sensor menu
 * Name is fixed !
 * 
 * it will store the data in available variables for each sensor
 * 
 * uint8_t UserInt1;
 * uint8_t UserInt2;
 * bool    UserBool1;
 * bool    UserBool2;
 * float   UserFloat1;    = HIGH alarm
 * float   UserFloat2;    = LOW AlARM
 * 
 */
void SensorMenu() //<<<<<<<<<<<MENU usage<<<<<<<<
{
  bool MustChangeLow = false;
  bool DisplayCel = true;
  float h, l, tempC;
  int c;
  String cf = "";
  
  Serial.print(header);
  Serial.print(" DS1820 Sensor menu ");
  Serial.println(header);

  Serial.print("\nDo you want to display in 1 = Celcius or 2 = Fahrenheit)");
  c = GetInput(2);
  if (c < 0) return;

  if (c == 2){
    DisplayCel = false;
    cf += " F";
  }
  else
    cf += " C";

  tempC = GetCurrentTemp();
  h = roundf(HighAlarm);
  l = roundf(LowAlarm);
  
  if (! DisplayCel) {
    h = (h * 9) / 5 + 32;
    l = (l * 9) / 5 + 32;
    tempC = (tempC * 9) / 5 + 32;
  }
  
  Serial.print("Current temperature is ");
  Serial.print(tempC);
  Serial.println(cf);
  
  Serial.print("Current High Alarm is ");
  Serial.print(h);
  Serial.println(cf);
  
  Serial.print("\nDo you want to change ? (1 = Yes, 2 = No) ");

  c = GetInput(2);
  if (c < 0) return;
  Serial.print("Selected ");
  Serial.println(c);
  
  if (c == 1) {
    if (DisplayCel) h = 85;
    else h = (85 * 9)/5 + 32;
    Serial.print("Provide the new High Alarm. Max. ");
    Serial.print(h);
    Serial.print(cf);

    h = GetInput(h);
    if (h < 0) return;
   
    if (h <= l) {
      Serial.print(h);
      Serial.print(cf);
      Serial.print(" High Alarm equal or lower to Low Alarm. ");
      Serial.println("You must adjust low Alarm! ");
      Serial.print("Do you set this HighAlarm ? (1 = yes, 2 = No)");
      
      c = GetInput(2);
      if (c != 1 || c < 0) {
        Serial.println("cancelled");
        return;
      }
      
      MustChangeLow = true;      
    }
  }

   // handle low alarm
  Serial.print("Current low Alarm is ");
  Serial.print(l);
  Serial.println(cf);
  
  if (! MustChangeLow) {
    Serial.print("\nDo you want to change ? (1 = yes, 2 = No)");
    c = GetInput(2);
    if (c != 1 || c < 0) {
      Serial.println("cancelled");
      return;
    }
  }
    
  Serial.print("\nProvide the new low Alarm. Min is 0, Max. ");
  Serial.print(h-1);
  l = GetInput(h-1);
  if (l < 0) return;

  Serial.print("High Alarm will be set to ");
  Serial.print(h);
  Serial.println(cf);
  
  Serial.print("Low Alarm will be set to ");
  Serial.print(l);
  Serial.println(cf);
  Serial.print("\nIs this correct ? (1 = yes, 2 = No)");

  c = GetInput(2);
  if (c != 1 || c < 0) {
    Serial.println("cancelled");
    return;
  }
  
  if (! DisplayCel){    // turn to celcius
    l = roundf(((l -32) * 5)/9);
    h = roundf(((h -32) * 5)/9);
  }

  SetAlarm(h,l);

  Serial.println("\nAlarm levels have been updated");
}

float GetCurrentTemp()
{
  sensors.requestTemperatures();
  return(sensors.getTempC(DS_Thermometer));
}

void SetAlarm(float high, float low)
{
  LowAlarm = low;
  HighAlarm = high;
  EE_DoMoAct.sensors[SensorChannel].UserFloat1 = HighAlarm; //<<<<<<<<<EEPROM usage
  EE_DoMoAct.sensors[SensorChannel].UserFloat2 = LowAlarm;  //<<<<<<<<<EEPROM usage
  
  // update EEPROM
  EE_WriteToEEprom();                                       //<<<<<<<<<EEPROM usage
}

#endif  //USERSENSORMENU

#endif // SENSOREXAMPLE
