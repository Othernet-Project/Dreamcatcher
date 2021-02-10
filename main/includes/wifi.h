
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void initWifi(const char*, const char*);
void wifi_init_softap(char* ssid, char* pass, uint8_t auth);
void wifi_init_sta(char* ssid, char* pass);
void ip_to_webusb(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t mode);
void urldecode(char* file);
void getStats(uint16_t*, uint16_t*);
void getPacketStats(int8_t*, int8_t*, int8_t*);

void web_server();

#ifdef __cplusplus
}
#endif
