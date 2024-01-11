/**
    Written by Paulvha December 2023
    Released under MIT licence.

    Different system variables which are used on different places.
*/

////////////////  USER MUST WIFI parameters //////////////////////////////////
#define SECRET_SSID "*" //network name
#define SECRET_PASS "*" //network password

/////////////// USER MUST location parameters ////////////////////////////////
// To set the right time and sunrise/sunset update these parameters
// You have to change the time zone offset to your local one.
float const LATITUDE  = 52.362083703528384;   // obtain the right values with Google maps
float const LONGITUDE = 4.891125827475844;
#define UTC_OFFSET      1                   // difference TZ to UTC (When NO DST)
#define DAYLIGHTSAVING  1                   // 1 = Western Europe / 2 = USA

////////////////  USER OPTIONAL DoMoAct Login //////////////////////////////////
// Name or Password can be set to a maximum MAX_CH_LENGTH
// Set MIN_PSWD_LENGTH a bit longer than the real password in order to not give away too much info

#define DoMoAct_NAME "*"        // DoMoAct name
#define DoMoAct_PASS "*"        // DoMOAct password
#define MIN_PSWD_LENGTH 5       // Only used as length of your password display webMenu (to confuse or help the user)
#define MAX_CH_LENGTH   10      // Max length of either name or password

////////////////  USER OPTIONAL Sensor example and menu ///////////////////////
#define SENSOREXAMPLE 0         // enable sensor example by setting to 1

#ifdef SENSOREXAMPLE == 1       // if sensor example is enabled
#define USERSENSORMENU  1       // if an optional (user defined) menu can also be enabled by setting to 1
#endif

////////////////  USER OPTIONAL enable matrix display ///////////////////////
#define ENABLEMATRIX 1         // enable LED Matrix display set to 1

////////////////////// PROGRAM parameters ////////////////////////////////////
#define INPUTDELAY     20       // wait max 20 seconds for serial-menu input
#define EE_NUMGROUPS   5        // max number of groups
#define EE_NUMPORTS    5        // max number of ports
#define EE_NUMSENSORS  5        // max number of sensors
#define EE_NUMDASHBOARD 10      // max number of dashboard items
#define EE_NAMELEN     20       // max namelength port / group / sensor
#define EE_NOTASSIGNED 0xff     // parameters is not assigned
#define EE_IMPORTDELAY 60       // wait max 60 x 500ms = 30 seconds for import
#define TIME_NOTASSIGNED 0xffff // Time is not assigned
#define WEBMENURESET   2        // if more than x minutes since last client connect, reset to start of web menu


// circular buffer
#define CB_MAXENTRIES 50        // max log of changes
#define CB_ENTRYSIZE  35        // max character length of each change

#define EE_VERSION  1.1

//////////////////// Port information
struct MyPort{
  uint8_t Gpio;         // GPIO to use
  bool    ActiveHigh;   // true : active is HIGH, false : active is low
  char    Name[EE_NAMELEN];// logical name (like backyard, frontdoor )
  uint8_t On_hour;      // Which hour ON
  uint8_t On_minute;    // Which minute ON
  uint8_t Off_hour;     // Which hour OFF
  uint8_t Off_minute;   // Which minute OFF
  bool    On_sunset;    // true: use on_minute (if set) to switch ON BEFORE before sunset, false: use (On_hour/ On_minute) or Group or sensor
  bool    Off_sunrise;  // true: use off_minute (if set) to switch OFF AFTER sunrise, false: use (Off_hour/ Off_minute) or Group or sensor
  uint8_t Day;          // which weekdays (bit 0 = sunday, 1 = ON, 0 = OFF)
  bool    Enable;       // true = enabled, false = disabled
  uint8_t Group[EE_NUMGROUPS];        // part of a group (0xff Not part of GROUP)

  // reserved for future extension / options
  uint8_t future1;
  uint8_t future2;
  bool    futureb1;
  bool    futureb2;
  float   futuref1;
  float   futuref2;
};
bool PortStatus[EE_NUMPORTS];

//////////////////// group information
struct MyGroups {
  char    Name[EE_NAMELEN];// logical name (like garden, house)
  uint8_t On_hour;      // Which hours ON
  uint8_t On_minute;    // Which minute ON
  uint8_t Off_hour;     // Which hours OFF
  uint8_t Off_minute;   // Which minute OFF
  bool    On_sunset;    // true: use on_minute (if set) to switch ON BEFORE before sunset, false: use On_hour/ On_minute
  bool    Off_sunrise;  // true: use off_minute (if set) to switch OFF AFTER sunrise, false: use Off_hour / Off_minute
  uint8_t Day;          // which weekdays (bit 0 = sunday, 1 = ON, 0 = OFF)
  bool    Enable;       // true = enabled, false = disabled
  uint8_t count;        // number of ports member of this group

  // reserved for future extension / options
  uint8_t future1;
  uint8_t future2;
  bool    futureb1;
  bool    futureb2;
  float   futuref1;
  float   futuref2;
};
bool GroupStatus[EE_NUMGROUPS];

//////////////////// sensor information
struct MySensors {
  char    Name[EE_NAMELEN];   // logical name (like temperature, waterlevel, humidity)
  uint8_t Day;                // which weekdays (bit 0 = sunday, 1 = ON, 0 = OFF)
  bool    Enable;             // true = enabled, false = disabled
  uint8_t Ports[EE_NUMPORTS]; // controlling ports (0xff No port)

  // reserved for sensor usage by the user
  uint8_t UserInt1;
  uint8_t UserInt2;
  bool    UserBool1;
  bool    UserBool2;
  float   UserFloat1;
  float   UserFloat2;
};

#define SENSOR_NOACT 0        // sensor channel actions
#define SENSOR_ON    1
#define SENSOR_OFF   2
#define SENSOR_FLIP  3

volatile byte ChangeSensorStatus[EE_NUMSENSORS];
bool SensorStatus[EE_NUMSENSORS];

// dashboard structure
struct MyDash {
  int PGS;    // port, group or sensor
  int num;    // which number of PGS
};

// structure stored in EEPROM
struct MyObject {
  float Version;                    // version of DoMoAct
  bool DisplaySwitch;               // enable display switching on Serial and led matrix (if enabled)
  int WebBackground;                // which WebBackground to use (Menu controlled)
  int DST;                          // daylight saving (automatic and menu controlled)
  MyDash DashBoard[EE_NUMDASHBOARD];// dashboard
  MyPort ports[EE_NUMPORTS];
  MyGroups groups[EE_NUMGROUPS];
  MySensors sensors[EE_NUMSENSORS];
};

// variables for GetFromRTC() to parse the RTC feedback
#define R_DAY    1            // day of month
#define R_MONTH  2
#define R_YEAR   3
#define R_HOUR   4
#define R_MINUTE 5
#define R_SECOND 6
#define R_WDAY   7            // day of week

// for enable/disable weekday
#define EE_SUNDAY    0
#define EE_MONDAY    1
#define EE_TUESDAY   2
#define EE_WEDNESDAY 3
#define EE_THURSDAY  4
#define EE_FRIDAY    5
#define EE_SATURDAY  6

// source or distination
#define PORTS   1
#define GROUP  2
#define SENSOR 3

// led matrix display
#define M_NO    0           // no scroll
#define M_LEFT  1           // scroll left
#define M_RIGHT 2           // scroll right
#define M_UP    3           // scroll up
#define M_DOWN  4           // scroll down
