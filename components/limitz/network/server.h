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
ESP_EVENT_DECLARE_BASE(SERVER_EVENTS);


enum 
{
	SERVER_EVENT_SESSION_BEGIN = 0x0A,
	SERVER_EVENT_SESSION_END   = 0x0B,
	SERVER_EVENT_RECV_PACKET   = 0x0C,
};

enum
{
	SERVER_STATE_ERROR = 0xFF,
	SERVER_STATE_UNKNOWN = 0x00,
	SERVER_STATE_LISTENING,
	SERVER_STATE_IN_SESSION
};

typedef struct _server_t
{
	int sock;
	int client;
	int port;
	int state;
	//RingbufHandle_t rbuffer;
	TaskHandle_t task;
	esp_event_loop_handle_t event_loop;
} server_t;

#if CONFIG_LMTZ_SERVER
#define SERVER_DEFAULT { \
	.sock = -1, \
	.port = CONFIG_LMTZ_SERVER_PORT, \
	.state = SERVER_STATE_UNKNOWN, \
	.task = NULL, \
	.event_loop = NULL \
}
#endif

void server_task(void* arg);

int server_init(server_t* self);
int server_close_session(server_t* self);
int server_destroy(server_t* self);
int server_send(server_t* self, const void* data, size_t len); 
