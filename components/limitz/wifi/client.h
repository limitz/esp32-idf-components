#pragma once

#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/ringbuf.h>

#include <esp_event.h>

// single client session for now, no threading or select
ESP_EVENT_DECLARE_BASE(CLIENT_EVENTS);


enum 
{
	CLIENT_EVENT_SESSION_BEGIN = 0x0A,
	CLIENT_EVENT_SESSION_END   = 0x0B,
	CLIENT_EVENT_RECV_PACKET   = 0x0C,
};

enum
{
	CLIENT_STATE_ERROR = 0xFF,
	CLIENT_STATE_UNKNOWN = 0x00,
	CLIENT_STATE_DISCONNECTED,
	CLIENT_STATE_CONNECTED,
	CLIENT_STATE_IN_SESSION
};

typedef struct
{
	int sock;
	int port;
	int state;
	char* ip;
	//RingbufHandle_t rbuffer;
	TaskHandle_t task;
	esp_event_loop_handle_t event_loop;
} client_t;

#if CONFIG_LMTZ_CLIENT
#define CLIENT_DEFAULT { \
	.sock = -1, \
	.ip = CONFIG_LMTZ_SERVER_IP,\
	.port = CONFIG_LMTZ_SERVER_PORT, \
	.state = CLIENT_STATE_UNKNOWN, \
	.task = NULL, \
	.event_loop = NULL \
}
#endif

void client_task(void* arg);

int client_init(client_t* self);
int client_close_session(client_t* self);
int client_destroy(client_t* self);
int client_send(client_t* self, const void* data, size_t len); 
