#pragma once

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"

//extern const uint8_t ota_pem_start[] asm("_binary_ota_pem_start");
//extern const uint8_t ota_pem_stop[]  asm("_binary_ota_pem_end");

#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID ""
#endif

#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD ""
#endif

#ifndef CONFIG_WIFI_RETRIES
#define CONFIG_WIFI_RETRIES 5
#endif

typedef struct
{
    int retries;
    const char* ssid;
    const char* password;
    const char* ota_url;
} wifi_t;

typedef  int (*wifi_cb) (wifi_t* sender, int error);

#define WIFI_DEFAULT \
{ \
    .ssid = CONFIG_WIFI_SSID, \
    .password = CONFIG_WIFI_PASSWORD, \
    .retries = CONFIG_WIFI_RETRIES, \
}


#define WIFI_PROVISION_OTA \
{ \
    .retries = CONFIG_WIFI_RETRIES, \
    .ota_url = "https://ota.splendo.health/release/SplendoFitBridge", \
}

void wifi_init(wifi_t* wifi);
void wifi_destroy(wifi_t* wifi);
