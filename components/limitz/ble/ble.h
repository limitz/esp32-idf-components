#pragma once

#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_event.>"
#include <esp_log.h>


typedef struct
{
    	const char* ssid;
  	const char* password;

	struct {
		const char* url;
		const char* pem;
    	} ota;
} wifi_t;

#if CONFIG_LMTZ_WIFI
#define WIFI_DEFAULT \
{ \
    .ssid = CONFIG_LMTZ_WIFI_SSID, \
    .password = CONFIG_LMTZ_WIFI_PASSWORD, \
    .ota = { \
	    .url = CONFIG_LMTZ_WIFI_OTA_URL,\
	    .pem = CONFIG_LMTZ_WIFI_OTA_PEM, \
    }, \
}
#endif

int wifi_init(wifi_t* network);
int wifi_ota_update(wifi_t* network);
int wifi_destroy(wifi_t* network);
