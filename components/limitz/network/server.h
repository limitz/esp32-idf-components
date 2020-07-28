#pragma once

#include "packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/ringbuf.h>

// single client session for now, no threading or select

typedef enum
{
	SERVER_STATE_ERROR = 0xFF,
	SERVER_STATE_UNKNOWN = 0x00,
	SERVER_STATE_LISTENING,
	SERVER_STATE_IN_SESSION

} server_state_t;

struct _server_t;
typedef int (*server_session_cb)(struct _server_t* server, int client);
typedef int (*server_receive_cb)(struct _server_t* server, int client, const void* data, size_t len);

typedef struct _server_t
{
	int sock;
	int port;
	server_state_t state;
	server_receive_cb on_receive;
	RingbufHandle_t rbuffer;
	
} server_t;

int server_init(server_t* self);
int server_close_session(server_t* self);
int server_destroy(server_t* self);

int server_send(server_t* self, const void* data, size_t len); 
