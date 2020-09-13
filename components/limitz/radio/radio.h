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
#include <freertos/task.h>

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

#define RADIO_KEY_SIZE ESP_NOW_KEY_LEN
#define MSG_KEYSIZE(size) "RADIO: PMK key size must be " #size

#define RADIO_PACKET_MAX_PAYLOAD 64
#define RADIO_PACKET_QUEUE_SIZE 6

#define RADIO_ACCEPT_ANY ((radio_packet_cb) 0x1)
#define RADIO_ACCEPT_NONE NULL


_Static_assert(strlen(CONFIG_LMTZ_RADIO_PMK) == RADIO_KEY_SIZE, MSG_KEYSIZE(RADIO_KEY_SIZE));

enum 
{
	RADIO_PACKET_TYPE_IDENTITY = 0x00,
	RADIO_PACKET_TYPE_DATA = 0x01,
};

enum
{
	RADIO_PACKET_FLAG_NONE = 0x00,
	RADIO_PACKET_FLAG_BROADCAST = 0x01,
	RADIO_PACKET_FLAG_READY = 0x02,
};

enum
{
	RADIO_ERROR  = 0xFF,
	RADIO_FAIL   = 0xFF,
	RADIO_OK     = 0x00,
	RADIO_ACCEPT = 0x00,
	RADIO_DENY,
};

typedef struct
{
	macaddr_t addr;
	uint8_t connected;
	char    name[24];
	uint8_t features[8];

} __attribute__((packed)) 
radio_identity_t;

struct _radio_packet_t;
typedef int (*radio_packet_cb)(const struct _radio_packet_t* packet);

typedef struct _radio_packet_t
{
	uint8_t  type;
	uint8_t  flags;
	uint16_t length; 
	uint16_t offset;

	union 
	{
		radio_identity_t identity;
		uint8_t payload[RADIO_PACKET_MAX_PAYLOAD];
	};

}
__attribute__((packed)) 
radio_packet_t;

typedef struct
{
	radio_identity_t identity;

	struct
	{
		radio_packet_cb on_accept;
		radio_packet_cb on_receive;
	//	int (*on_accept)(const radio_packet_t* identity);
	//	int (*on_receive)(const radio_packet_t* packet);
	} 
	callbacks;

	QueueHandle_t queue;
	TaskHandle_t task;
} radio_t;

int radio_init();
int radio_unicast(const macaddr_t* to, int type, const void* payload, size_t len);
int radio_broadcast(int type, const void* payload, size_t len);
int radio_send_packet(const macaddr_t* to, const radio_packet_t* packet);
int radio_deinit();

extern radio_t RADIO;
