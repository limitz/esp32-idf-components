#pragma once


#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_wifi.h>

#include <esp_now.h>
#include <tcpip_adapter.h>

#define RADIO_BROADCAST_ADDRESS { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
#define RADIO_WIFI_MODE WIFI_MODE_STA
#define RADIO_WIFI_IF   WIFI_IF_WIFI_STA

#define radio_peer_exists exp_now_is_peer_exist

#define RADIO_KEY_LEN ESP_NOW_KEY_LEN
#define RADIO_PACKET_MAX_PAYLOAD 	64
#define RADIO_PACKET_QUEUE_SIZE 	6

enum
{
	RADIO_OK = 0x00,
	RADIO_ACCEPT = 0x00,
	RADIO_ERROR,
	RADIO_FAIL,
	RADIO_DENY,
};

