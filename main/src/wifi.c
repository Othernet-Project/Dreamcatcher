#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "wifi.h"
#include "customize.h"

esp_netif_t *ap_netif = NULL;

static const char *TAG = "wifi softAP";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
        ESP_LOGI(TAG, "station connected");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "station disconnected");
        vTaskDelay(1000);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        extern char myIP[20];
        memset(myIP, 0, 20);
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        sprintf(myIP, IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip: %s", myIP );
    }
}

void initWifi(const char* ssid_ap, const char* pass_ap)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    ap_netif = esp_netif_create_default_wifi_ap();

    esp_netif_t *esp_netif = NULL;
    esp_netif = esp_netif_next(esp_netif);
    esp_netif_set_hostname(esp_netif, "Othernet-DC4");

    esp_netif = esp_netif_next(esp_netif);
    esp_netif_set_hostname(esp_netif, "Othernet-DC4");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));

    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                    IP_EVENT_STA_GOT_IP,
                                                    &wifi_event_handler,
                                                    NULL,
                                                    &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL) );
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_start() );
    // wifi_init_sta(EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, WIFI_MODE_STA);
    wifi_init_softap(ssid_ap, pass_ap, WIFI_AUTH_WPA2_PSK);
    esp_wifi_set_ps(WIFI_PS_NONE);
}

void wifi_init_softap(char* ssid, char* pass, uint8_t auth)
{

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ssid),
            .channel = 0,
            .max_connection = 3,
            .authmode = auth
        },
    };
    strcpy((char*)wifi_config.ap.ssid, ssid);
    strcpy((char*)wifi_config.ap.password, pass);
    if (strlen(pass) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             ssid, pass);
}

void wifi_init_sta(char* ssid, char* pass)
{

    wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, pass);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    esp_wifi_disconnect(); // if we're trying to connect with wrong credentials, don't bomb.
    ESP_ERROR_CHECK( esp_wifi_connect() );

    ESP_LOGI(TAG, "wifi_init_sta finished. SSID:%s password:%s",
             ssid, pass);
}
