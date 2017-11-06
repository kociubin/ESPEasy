//#######################################################################################################
//#################################### Plugin 064: APDS9960 Gesture ##############################
//#######################################################################################################

// ESPEasy Plugin to scan a gesture, proximity and light chip APDS9960
// written by Jochen Krapf (jk@nerd2nerd.org)

// A new gesture is send immediately to controllers.
// Proximity and Light are scanned frequently by given 'Delay' setting.
// RGB is not scanned because there are only 4 vars per task.

// Known BUG: While performing a gesture the reader function blocks rest of ESPEasy processing!!! (Feel free to fix...)

// Note: The chip has a wide view-of-angle. If housing is in this angle the chip blocks!


//TODO
//  add configurations for INTERRUPTS and i2C GPIOs



#ifdef PLUGIN_BUILD_DEV

#define PLUGIN_064
#define PLUGIN_ID_064         64
#define PLUGIN_NAME_064       "Gesture - APDS9960 [DEV]"
#define PLUGIN_VALUENAME1_064 "Proximity"

#include <SparkFun_APDS9960.h>   //Lib is modified to work with ESP

#ifndef CONFIG
#define CONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][n])
#endif

SparkFun_APDS9960* PLUGIN_064_pds = NULL;


boolean Plugin_064(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_064;
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
        string = F(PLUGIN_NAME_064);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_064));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte addr = 0x39;   // CONFIG(0); chip has only 1 address

        int optionValues[1] = { 0x39 };
        addFormSelectorI2C(string, F("i2c_addr"), 1, optionValues, addr);  //Only for display I2C address

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //CONFIG(0) = getFormItemInt(F("i2c_addr"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (PLUGIN_064_pds)
          delete PLUGIN_064_pds;
        PLUGIN_064_pds = new SparkFun_APDS9960();

        String log = F("APDS : ");
        if ( PLUGIN_064_pds->init() )
        {
          log += F("Init");

          PLUGIN_064_pds->enablePower();

          if (! PLUGIN_064_pds->enableProximitySensor(false))
             log += F(" - Error during proximity sensor init!");

          // Adjust the Proximity sensor gain
          // if ( !PLUGIN_064_pds->setProximityGain(PGAIN_4X) ) {
          //    log += F(" - Something went wrong trying to set PGAIN");
          //  }

        }
        else
        {
          log += F("Error during APDS-9960 init!");
        }

        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        if (!PLUGIN_064_pds)
          break;

        uint8_t proximity_data = 0;
        PLUGIN_064_pds->readProximity(proximity_data);

        //Filter out unnecessary values
        if (proximity_data >= 254.0 || proximity_data < 120)
          break;

        //UserVar[event->BaseVarIndex + 1] = (float)proximity_data;
        //addLog(LOG_LEVEL_INFO, "Read some proximity data");
        //log += F(" (");
        //log += (float)proximity_data;
        //log += F(")");

        UserVar[event->BaseVarIndex] = (float)proximity_data;
        event->sensorType = SENSOR_TYPE_SWITCH;

        sendData(event);

        //addLog(LOG_LEVEL_INFO, log);


        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        if (!PLUGIN_064_pds)
          break;

        success = true;
        break;
      }

  }
  return success;
}

#endif
