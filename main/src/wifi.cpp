#include "Preferences.h"
#include "WiFi.h"
#include "WiFiMulti.h"
#include "HTTPClient.h"
#include "WiFiClientSecure.h"
#include "UrlEncode.h"
#include "wifi.h"
#include "customize.h"
#include "esp_vfs.h"
#include <esp_vfs_fat.h>
#include "esp_sntp.h"
#include "settings.h"

extern bool sdCardPresent;
extern SemaphoreHandle_t send_tlm;
static const char *TAG = "wifi";

WiFiMulti WiFiMulti;

void initWifi(char* ssid_ap, char* pass_ap)
{
    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    sprintf(newLogEntry,"WIFI AP initialization, Uptime: %lus", millis()/1000);
    logToFile(newLogEntry);

    WiFi.mode(WIFI_MODE_APSTA);
    wifi_init_softap(ssid_ap, pass_ap);
}

void wifi_init_softap(char* ssid, char* pass)
{
    WiFi.softAP(ssid, pass);

    Serial.print("ESP32 IP as soft AP: ");
    Serial.println(WiFi.softAPIP());
}

void onWiFiEvent(WiFiEvent_t event)
{
    String eventInfo = "No Info";

    switch (event) {
        case ARDUINO_EVENT_WIFI_READY: 
            eventInfo = "WiFi interface ready";
            break;
        case ARDUINO_EVENT_WIFI_SCAN_DONE:
            eventInfo = "Completed scan for access points";
            break;
        case ARDUINO_EVENT_WIFI_STA_START:
            eventInfo = "WiFi client started";
            break;
        case ARDUINO_EVENT_WIFI_STA_STOP:
            eventInfo = "WiFi clients stopped";
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            eventInfo = "Connected to access point";
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            eventInfo = "Disconnected from WiFi access point";
            break;
        case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
            eventInfo = "Authentication mode of access point has changed";
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            eventInfo = "Obtained IP address";
            //Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            eventInfo = "Lost IP address and IP address is reset to 0";
            break;
        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            eventInfo = "WiFi Protected Setup (WPS): succeeded in enrollee mode";
            break;
        case ARDUINO_EVENT_WPS_ER_FAILED:
            eventInfo = "WiFi Protected Setup (WPS): failed in enrollee mode";
            break;
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            eventInfo = "WiFi Protected Setup (WPS): timeout in enrollee mode";
            break;
        case ARDUINO_EVENT_WPS_ER_PIN:
            eventInfo = "WiFi Protected Setup (WPS): pin code in enrollee mode";
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            eventInfo = "WiFi access point started";
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            eventInfo = "WiFi access point  stopped";
            break;
        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
            eventInfo = "Client connected";
            break;
        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
            eventInfo = "Client disconnected";
            break;
        case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
            eventInfo = "Assigned IP address to client";
            break;
        case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
            eventInfo = "Received probe request";
            break;
        case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
            eventInfo = "AP IPv6 is preferred";
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            eventInfo = "STA IPv6 is preferred";
            break;
        case ARDUINO_EVENT_ETH_GOT_IP6:
            eventInfo = "Ethernet IPv6 is preferred";
            break;
        case ARDUINO_EVENT_ETH_START:
            eventInfo = "Ethernet started";
            break;
        case ARDUINO_EVENT_ETH_STOP:
            eventInfo = "Ethernet stopped";
            break;
        case ARDUINO_EVENT_ETH_CONNECTED:
            eventInfo = "Ethernet connected";
            break;
        case ARDUINO_EVENT_ETH_DISCONNECTED:
            eventInfo = "Ethernet disconnected";
            break;
        case ARDUINO_EVENT_ETH_GOT_IP:
            eventInfo = "Obtained IP address";
            break;
        default: break;
    }
    Serial.print("WIFI-Event: ");
    Serial.println(eventInfo);

    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    sprintf(newLogEntry,"WIFI-Event: %s", eventInfo.c_str());
    logToFile(newLogEntry);
}

void setUnixtime(int32_t unixtime) {
    Serial.println("Syncing time with Client Device");
    timeval epoch = {unixtime, 0};
    settimeofday((const timeval*)&epoch, 0);
    
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    Serial.printf("Current Devicetime: %02d:%02d:%02d - %02d.%02d.%d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
}

void wifi_init_sta(char* ssid, char* pass)
{
    char *newLogEntry = (char*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);

    WiFiMulti.addAP(ssid, pass);

    Serial.println("Connecting to WiFi Network... ");
    sprintf(newLogEntry,"WIFI Client enabled, Connecting...");
    logToFile(newLogEntry);

    WiFi.onEvent(onWiFiEvent);

    int connectTrys = 0;
    while(connectTrys < 30) {
        connectTrys++;
        if(WiFiMulti.run() == WL_CONNECTED){
            Serial.print("Connected :)");
            sprintf(newLogEntry,"Connected :)");
            break;
        } else {
            Serial.print(".");
            sprintf(newLogEntry,".");
        }  
        logToFile(newLogEntry);
        delay(500);
    }
    Serial.print("WiFi connected - ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    sprintf(newLogEntry,"Connected to WIFI, RSSI: %d, Got IP: %s", WiFi.RSSI() ,WiFi.localIP().toString().c_str());
    logToFile(newLogEntry);

    // connected to wifi and maybe internet, try to set device time over ntp
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    Serial.printf("Current Devicetime: %02d:%02d:%02d - %02d.%02d.%d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
    if (timeinfo.tm_year < (2023 - 1900)) {
        Serial.println("Time is not set yet. Connecting to WiFi and getting time over NTP.");
        //Init SNTP
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();

        int retry = 0;
        const int retry_count = 10;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
             Serial.printf("Waiting for system time to be set... (%d/%d)\n", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        time(&now);
        localtime_r(&now, &timeinfo);
        Serial.printf("Current Devicetime: %02d:%02d:%02d - %02d.%02d.%d \n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
    }

    extern char myIP[20];
    memset(myIP, 0, 20);
    sprintf(myIP,"%s", WiFi.localIP().toString().c_str());
}

void getFreeSpace(uint64_t* used_space, uint64_t* max_space)
{
    if(sdCardPresent) {
        FATFS *fs;
        DWORD c;
        if (f_getfree("/files", &c, &fs) == FR_OK)
        {
            *used_space =
                ((uint64_t)fs->csize * (fs->n_fatent - 2 - fs->free_clst)) * fs->ssize;
            *max_space = ((uint64_t)fs->csize * (fs->n_fatent - 2)) * fs->ssize;
        }
    } else {
        *used_space = 1;
        *max_space = 1;
    }
}

const char telemetry_rootca[] PROGMEM = R"=====(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)=====";

static const char telemetry_json[] = \
    "{\"lanip\":\"%s\",\
    \"macid\":\"%s\",\
    \"platform\":\"%s\",\
    \"version\":\"%s\",\
    \"heap\":\"%d\",\
    \"minheap\":\"%d\",\
    \"psram\":\"%d\",\
    \"freq\":\"%.3f\",\
    \"22khz\":\"%d\",\
    \"crc_ok\":\"%d\",\
    \"rssi\":\"%d\",\
    \"snr\":\"%d\",\
    \"crc_err\":\"%d\",\
    \"header\":\"%d\",\
    \"cpu\":\"%d\",\
    \"ldo\":\"%d\",\
    \"volt\":\"%.1f\",\
    \"freq_offset\":\"%d\",\
    \"bitrate\":\"%d\",\
    \"packetrate\":\"%.2f\",\
    \"used\":\"%llu\",\
    \"max\":\"%llu\",\
    \"filename\":\"%s\"}";

void send_telemetry(void *pvParameter)
{
    String host = "status.othernet.is";
    String url = "https://"+host+"/esp32based/status";
    
    while(1)
    {
        if( xSemaphoreTake( send_tlm, portMAX_DELAY ) == pdTRUE )
        {
            extern uint32_t packetsRX;
            extern unsigned int filepacket;
            extern unsigned int filepackets;
            extern int detectedLNB;  // bit2 - connected, bit5 - LDO_ON, bit1 - in range
            extern float voltage;
            extern uint32_t Frequency; 
            extern uint32_t bitrate;
            extern float pktrate;
            extern uint8_t CPU_USAGE;
            extern char filename[260];
            extern uint16_t offset;
            extern bool bEnableDiseq;

            // send telemetry to server
            char telemetryPayload[1500];

            uint64_t used_space = 100;
            uint64_t max_space = 30000;
            getFreeSpace(&used_space, &max_space);

            int8_t snr = 1, rssi = 2;
            uint16_t crc = 11, header = 12;
            getStats(&crc, &header);
            getPacketStats(&rssi, &snr);
            float freq = (float)Frequency/1000000;

            sprintf(telemetryPayload, telemetry_json,
                WiFi.localIP().toString().c_str(),
                WiFi.macAddress().c_str(),
                HW_VERSION,
                FW_VERSION,
                heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL),
                heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                freq,
                bEnableDiseq,
                packetsRX,
                rssi,
                snr,
                crc,
                header,
                CPU_USAGE,
                detectedLNB,
                voltage,
                0,
                bitrate,
                pktrate,
                used_space,
                max_space,
                filename
            );

            //Serial.println(telemetryPayload);
            String data = urlEncode(telemetryPayload);
            //Serial.print("Encoded Data: ");
            //Serial.println(data);

            WiFiClientSecure client;
            //client.setInsecure(); //the magic line, use with caution
            client.setCACert(telemetry_rootca);
            client.connect(host.c_str(), 443);
            Serial.println("Connected to Telemetry Server");

            HTTPClient https;      
            https.begin(client, String(url+"?"+data));

            Serial.println("Try https get");
            String payload;
            
            if (https.GET() == 200) {   
                payload = https.getString(); 
                //Serial.println(payload);
            }
            https.end();
            client.stop();
        }
    }
}
