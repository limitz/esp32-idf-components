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
#include <tcpip_adapter.h>
#include <esp_wifi.h>

#include <esp_now.h>
#define esp_now_peer_exists esp_now_is_peer_exist

#define RADIO_BROADCAST_ADDRESS { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF   ESP_IF_WIFI_STA

#define RADIO_KEY_SIZE ESP_NOW_KEY_LEN
#define RADIO_PACKET_MTU (CONFIG_LMTZ_RADIO_PACKET_PAYLOAD_SIZE + sizeof(radio_packet_header_t))
#define RADIO_PACKET_QUEUE_LEN CONFIG_LMTZ_RADIO_PACKET_QUEUE_LEN
#define RADIO_ACCEPT_ANY ((radio_packet_cb) 0x1)
#define RADIO_ACCEPT_NONE NULL


enum 
{
	RADIO_PACKET_TYPE_IDENTITY = 0x00,
	RADIO_PACKET_TYPE_DATA = 0x01,

	RADIO_PACKET_TYPE_CUSTOM
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
	uint8_t addr[6];
	uint8_t connected;
	char    name[24];
	uint8_t features[8];

} __attribute__((packed)) 
radio_identity_t;

struct _radio_packet_t;
typedef int (*radio_packet_cb)(const struct _radio_packet_t* packet);

typedef struct _radio_packet_header_t
{
	uint8_t addr_src[6]; // 6
	uint8_t addr_dst[6]; // 12

	uint8_t type;        // 13
	uint8_t length;      // 14
	uint16_t offset;     // 16
}
__attribute__((packed))
radio_packet_header_t;

#define RADIO_PACKET_MAX_PAYLOAD (RADIO_PACKET_MTU - sizeof(radio_packet_header_t))

typedef struct _radio_packet_t
{
	radio_packet_header_t header;
	uint8_t payload[RADIO_PACKET_MAX_PAYLOAD];
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

	QueueHandle_t queue, send_queue;
	TaskHandle_t task;
} radio_t;

int radio_init();
int radio_unicast(const uint8_t* addr_to, int type, const void* payload, size_t len);
int radio_broadcast(int type, const void* payload, size_t len);
int radio_send_packet(const uint8_t* addr_dst, const radio_packet_t* packet);
int radio_deinit();

extern radio_t RADIO;
