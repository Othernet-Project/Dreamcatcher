#include "Preferences.h"
#include "customize.h"
#include "settings.h"
#include "wifi.h"
#include "lnb.h"

Preferences prefs;
uint8_t CPU_USAGE;

static void setDefaultsIRQ()
{
  defaultsPin = digitalRead(0);
  if (!defaultsPin)
  {
    Serial.printf("pin0 status: %d, time: %ld\n", defaultsPin, millis());
    resetStart = millis();
  } else {
    Serial.println("stop IO0");
  }
}

void loadSettings()
{
  prefs.begin("lora");
  // LoRa settings
  Frequency = prefs.getUInt("freq", DEFAULT_FREQUENCY);
  Bandwidth = prefs.getUChar("bw", DEFAULT_BW);
  SpreadingFactor = prefs.getUChar("sf", DEFAULT_SF);
  CodeRate = prefs.getUChar("cr", DEFAULT_CR);
  bEnableLNB = prefs.getBool("lnb", true);
  uLOid = prefs.getUChar("loid", 0);
  bEnableLO = prefs.getBool("lo", false);
  bEnableDiseq = prefs.getBool("diseq", false);

  bEnableTlm = prefs.getBool("statlm", false);
  // wifi settings
  String ssid_ap = prefs.getString("apssid", EXAMPLE_ESP_WIFI_SSID);
  String pass_ap = prefs.getString("appass", EXAMPLE_ESP_WIFI_PASS);

  char ssid_sta[32] = {0};
  char pass_sta[32] = {0};

  prefs.getString("stapass", pass_sta, 32);
  //TODO: Replace with arduino wifi AP
  initWifi((char*)ssid_ap.c_str(), (char*)pass_ap.c_str());

  // if SSID and PW is set for WifiClient -> use it
  if (prefs.getString("stassid", ssid_sta, 32))
  {
    wifi_init_sta(ssid_sta, pass_sta);
  }

  prefs.end();
  attachInterrupt(0, setDefaultsIRQ,CHANGE);
}

extern "C" void storeWifiCredsAP(char* ssid, char* pass)
{
  prefs.begin("lora", false);
  prefs.putString("apssid", ssid);
  prefs.putString("appass", pass);
  prefs.end();
}

extern "C" void storeWifiCredsSTA(char* ssid, char* pass, bool tlm)
{
  prefs.begin("lora", false);
  prefs.putString("stassid", ssid);
  prefs.putString("stapass", pass);
  prefs.putBool("statlm", tlm);
  prefs.end();
}

void storeLoraSettings()
{
  prefs.begin("lora", false);
  prefs.putUInt("freq", Frequency);
  prefs.putUChar("bw", Bandwidth);
  prefs.putUChar("sf", SpreadingFactor);
  prefs.putUChar("cr", CodeRate);
  prefs.putBool("lnb", bEnableLNB);
  prefs.putBool("lo", bEnableLO);
  prefs.putUChar("loid", uLOid);
  prefs.putUChar("diseq", bEnableDiseq);
  prefs.end();
}

void rebootDevice()
{
  Serial.println("Rebooting Device on Users request...");
  char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
  sprintf(newLogEntry,"Restarting Device on Users request, Uptime: %lus", millis()/1000);
  logToFile(newLogEntry);
  ESP.restart();
}

void setDefaults()
{
  prefs.begin("lora", false);
  prefs.clear(); // clear settings space
  prefs.end();
  storeWifiCredsAP(EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS); // set defaults
  updateLoraSettings(DEFAULT_FREQUENCY, DEFAULT_BW, DEFAULT_SF, DEFAULT_CR);
  vTaskDelay(1000);
  esp_restart(); // restart device
}

//-------------------------------------------------//

/**
 * Helper function to encode URL with %20 (space) in folder/file name
 * 
 */
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

void urldecode(char* file)
{
    String str = String(file);
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
      yield();
    }
  
    memset(file, 0, strlen(file));
    strlcpy(file, encodedString.c_str(), encodedString.length() + 1);
}

//-------------------------------------------------//

void vTaskGetRunTimeStats2( )
{
  TaskStatus_t *pxTaskStatusArray;
  volatile UBaseType_t uxArraySize, x;
  uint32_t ulTotalRunTime, ulStatsAsPercentage = 0;

   /* Take a snapshot of the number of tasks in case it changes while this
   function is executing. */
   uxArraySize = uxTaskGetNumberOfTasks();

   /* Allocate a TaskStatus_t structure for each task.  An array could be
   allocated statically at compile time. */
   pxTaskStatusArray = (TaskStatus_t *)heap_caps_calloc(uxArraySize, sizeof( TaskStatus_t ) ,MALLOC_CAP_SPIRAM);

   if( pxTaskStatusArray != NULL )
   {
      /* Generate raw status information about each task. */
      uxArraySize = uxTaskGetSystemState( pxTaskStatusArray,
                                 uxArraySize,
                                 &ulTotalRunTime );

      /* For percentage calculations. */
      ulTotalRunTime /= 100UL;

      /* Avoid divide by zero errors. */
      if( ulTotalRunTime > 0 )
      {
         /* For each populated position in the pxTaskStatusArray array,
         format the raw data as human readable ASCII data. */
         for( x = 0; x < uxArraySize; x++ )
         {
            if (strcmp("IDLE", pxTaskStatusArray[ x ].pcTaskName) == 0)
            {
              ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime;
              CPU_USAGE = 100 - ulStatsAsPercentage;
              break;
            }
         }
      }

      /* The array is no longer needed, free the memory it consumes. */
      vPortFree( pxTaskStatusArray );
   }
}
