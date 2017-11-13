//#######################################################################################################
//#################################### Plugin 188: LIS3DH MEMS/Accelerometer ############################
//#######################################################################################################

// ESPEasy Plugin to detect motion from the accelerometer LIS3DH
// written by Mariusz Kociubinski (kociubin@gmail.com)


#ifdef PLUGIN_BUILD_DEV

#define PLUGIN_188
#define PLUGIN_ID_188         188
#define PLUGIN_NAME_188       "MEMS - LIS3DH [DEV]"
#define PLUGIN_VALUENAME1_188 "click_type_fired"

#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

#ifndef CONFIG_LONG
#define CONFIG_LONG(n) (Settings.TaskDevicePluginConfigLong[event->TaskIndex][n])
#endif


//Plugin Settings

// clicksetting:
//    0 = turn off click detection & interrupt
//    1 = single click only interrupt output
//    2 = double click only interrupt output, detect single click

// sensitivity
//    Adjust this number for the sensitivity of the 'click' force
//    Higher numbers are less sensitive
//    this strongly depend on the range! for 16G, try 5-10
//    for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80

// Other parameters for click detection
// More info:  http://www.adafruit.com/datasheets/LIS3DHappnote.pdf

unsigned long Plugin_188_clicksetting;     //default = 2;
unsigned long Plugin_188_clickthreshold;   //default = 40;
unsigned long Plugin_188_timelimit;        //default = 10;
unsigned long Plugin_188_timelatency;      //default = 20;
unsigned long Plugin_188_timewindow;       //default = 255;

boolean Plugin_188_initialized  = false;

Adafruit_LIS3DH* PLUGIN_188_pds = NULL;


boolean Plugin_188(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
      case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_188;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].VType = SENSOR_TYPE_SWITCH;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].TimerOptional = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

      case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_188);
        break;
      }

      case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_188));
        break;
      }

      case PLUGIN_WEBFORM_LOAD:
      {

        Plugin_188_clicksetting = CONFIG_LONG(0);
        if (Plugin_188_clicksetting < 1 || Plugin_188_clicksetting > 2) Plugin_188_clicksetting = 1;
        addFormNumericBox(string, F("Click Detection 1=single 2=double click"), F("plugin_188_clicksetting"), Plugin_188_clicksetting, 1, 2);

        Plugin_188_clickthreshold = CONFIG_LONG(1);
        if (Plugin_188_clickthreshold < 1 || Plugin_188_clickthreshold > 100) Plugin_188_clickthreshold = 40;
        addFormNumericBox(string, F("Click threshold"), F("plugin_188_clickthreshold"), Plugin_188_clickthreshold, 40, 120);

        Plugin_188_timelimit = CONFIG_LONG(2);
        if (Plugin_188_timelimit < 1 || Plugin_188_timelimit > 100) Plugin_188_timelimit = 10;
        addFormNumericBox(string, F("Click time limit"), F("plugin_188_timelimit"), Plugin_188_timelimit, 1, 100);

        Plugin_188_timelatency = CONFIG_LONG(3);
        if (Plugin_188_timelatency < 1 || Plugin_188_timelatency > 100) Plugin_188_timelatency = 20;
        addFormNumericBox(string, F("Click time latency"), F("plugin_188_timelatency"), Plugin_188_timelatency, 1, 100);

        Plugin_188_timewindow = CONFIG_LONG(4);
        if (Plugin_188_timewindow < 1 || Plugin_188_timewindow > 500) Plugin_188_timewindow = 255;
        addFormNumericBox(string, F("Click time window"), F("plugin_188_timewindow"), Plugin_188_timewindow, 1, 500);

        success = true;
        break;
      }

      case PLUGIN_WEBFORM_SAVE:
      {
        CONFIG_LONG(0) = getFormItemInt(F("plugin_188_clicksetting"));
        CONFIG_LONG(1) = getFormItemInt(F("plugin_188_clickthreshold"));
        CONFIG_LONG(2) = getFormItemInt(F("plugin_188_timelimit"));
        CONFIG_LONG(3) = getFormItemInt(F("plugin_188_timelatency"));
        CONFIG_LONG(4) = getFormItemInt(F("plugin_188_timewindow"));

        success = true;
        break;
      }

      case PLUGIN_INIT:
      {

        if (PLUGIN_188_pds)
          delete PLUGIN_188_pds;

        PLUGIN_188_pds = new Adafruit_LIS3DH();

        addLog(LOG_LEVEL_INFO,  F("LIS3DH : Begin init"));

        if ( PLUGIN_188_pds->begin(0x18))
        {
            PLUGIN_188_pds->setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

            String log = "";
            log += "Sending the following configs to LIS3DH:";
            log += "    Clicksetting: ";  log+= CONFIG_LONG(0);
            log += "    Clickthreshold: ";  log+= CONFIG_LONG(1);
            log += "    Timelimit: ";  log+= CONFIG_LONG(2);
            log += "    TimeLatency: ";  log+= CONFIG_LONG(3);
            log += "    Timewindow: ";  log+= CONFIG_LONG(4);
            addLog(LOG_LEVEL_INFO, log);

            PLUGIN_188_pds->setClick(
              CONFIG_LONG(0),CONFIG_LONG(1),CONFIG_LONG(2),CONFIG_LONG(3),CONFIG_LONG(4));

          addLog(LOG_LEVEL_INFO, F("Init Successful"));
          Plugin_188_initialized = true;
        }
        else
        {
          addLog(LOG_LEVEL_ERROR, F("Error during LIS3DH init!"));
          Plugin_188_initialized = false;
          //success = false;
          break;
        }

        success = true;
        break;
      }

      case PLUGIN_FIFTY_PER_SECOND:
      {
        if (!PLUGIN_188_pds || !Plugin_188_initialized)
          break;

        uint8_t click_value = 0.0;  //will be set to 1 for single and 2 for double click
        uint8_t click = PLUGIN_188_pds->getClick();
        if (click == 0)
          break;
        if (! (click & 0x30))
          break;

        Serial.print("Click detected (0x"); Serial.print(click, HEX); Serial.print("): ");
        if (click & 0x10) {
          Serial.print(" single click\n");
          click_value = 1.0;
        }
        if (click & 0x20) {
          Serial.print(" double click\n");
          click_value = 2.0;
        }

        UserVar[event->BaseVarIndex] = (float)click_value;
        event->sensorType = SENSOR_TYPE_SWITCH;

        sendData(event);

        success = true;
        break;
      }

      case PLUGIN_READ:
      {
        if (!PLUGIN_188_pds)
          break;

        success = true;
        break;
      }

  }
  return success;
}

#endif
