#pragma once

#include "macaddr.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <tcpip_adapter.h>
#include <esp_wifi.h>

#include <esp_now.h>
#include <esp32/rom/crc.h>

#define RADIO_BROADCAST_ADDRESS { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }

#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

#define esp_now_peer_exists esp_now_is_peer_exist
//because, seriously...

#ifndef RADIO_PACKET_PAYLOAD_TYPE
#define RADIO_PACKET_PAYLOAD_TYPE uint8_t
#endif

#define RADIO_KEY_SIZE ESP_NOW_KEY_LEN
#define RADIO_QUEUE_SIZE 6

#define RADIO_PACKET_SIZE sizeof(radio_packet_t)

#define MSG_PAYLOAD(type, size) "RADIO: Configured payload size of " \
				#size \
				" insufficient for type " \
				#type

#define MSG_KEYSIZE(size) "RADIO: PMK key size must be " #size

_Static_assert(sizeof(RADIO_PACKET_PAYLOAD_TYPE) <= CONFIG_LMTZ_RADIO_PACKET_PAYLOAD_SIZE,
		MSG_PAYLOAD(RADIO_PACKET_PAYLOAD_TYPE, CONFIG_LMTZ_RADIO_PACKET_PAYLOAD_SIZE));

_Static_assert(strlen(CONFIG_LMTZ_RADIO_PMK) == RADIO_KEY_SIZE, MSG_KEYSIZE(RADIO_KEY_SIZE));

enum 
{
	RADIO_PACKET_TYPE_IDENTITY,
	RADIO_PACKET_TYPE_DATA,
};

enum
{
	RADIO_PACKET_FLAG_NONE = 0x00,
	RADIO_PACKET_FLAG_BROADCAST = 0x01,
	RADIO_PACKET_FLAG_READY = 0x02,
};

enum
{
	RADIO_ERROR = 0xFF,
	RADIO_FAIL = 0xFF,

	RADIO_OK = 0x00,
	RADIO_ACCEPT = 0x00,
	RADIO_DENY,
};

typedef struct
{
	macaddr_t addr;
	char name[16];

} __attribute__((packed)) radio_identity_t;

typedef struct
{
	macaddr_t addr;
	uint8_t type;
	uint8_t flag;
	uint16_t seq;
	uint16_t crc;
	
	union 
	{
		uint8_t data[CONFIG_LMTZ_RADIO_PACKET_PAYLOAD_SIZE];
		RADIO_PACKET_PAYLOAD_TYPE payload;
		radio_identity_t identity;
	};

} __attribute__((packed)) radio_packet_t;

typedef struct
{
	radio_identity_t identity;
	macaddr_t broadcast_addr;

	struct
	{
		int (*accept)(const radio_identity_t* identity);
		int (*receive)(const radio_packet_t* packet);
	} 
	callbacks;

} radio_t;

int radio_init(radio_t* self);
int radio_send(radio_t* self, radio_packet_t* packet);
int radio_destroy(radio_t* self);
