
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void initWifi(char*, char*);
void wifi_init_softap(char* ssid, char* pass);
void wifi_init_sta(char* ssid, char* pass);
void ip_to_webusb(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t mode);
void urldecode(char* file);
void getStats(uint16_t*, uint16_t*);
void getPacketStats(int8_t*, int8_t*);

void web_server();
void send_telemetry(void *pvParameter);

void getFreeSpace(uint64_t* used_space, uint64_t* max_space);

void setUnixtime(int32_t unixtime);

struct ft8_data{
  const char *message;
  bool isFreeMessage;
  bool useOddSlot;
  bool readyToTx;
};
extern struct ft8_data ft8Data;

struct cw_data{
  bool enabled;
  int type;
};
extern struct cw_data cwData;

extern SemaphoreHandle_t send_tlm;

#ifdef __cplusplus
}
#endif
