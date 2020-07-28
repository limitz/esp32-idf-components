#include "client.h"

#include <esp_system.h>
#include <esp_log.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#define TAG "CLIENT"

ESP_EVENT_DEFINE_BASE(CLIENT_EVENTS);

static void client_post_event(client_t* self, int event_id, void* data, size_t len)
{
	if (self->event_loop)
	{
		esp_event_post_to(
			self->event_loop,
			CLIENT_EVENTS, event_id,
			data, len, 100/portTICK_PERIOD_MS);
	}
	else 
	{
		esp_event_post(
			CLIENT_EVENTS, event_id,
			data, len, 100/portTICK_PERIOD_MS);
	}
}

void client_task(void* arg)
{
	client_t* self = (client_t*) arg;
	int opt = 1;
	int error = 0;
	uint8_t  rxbuffer[256] = { 0 };
	uint16_t rxsize = 0;
	
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(self->port);
	error = inet_aton(self->ip, &dest.sin_addr);
	
	if (error == 0) 
	{
		ESP_LOGW(__func__, "Unable to parse %s as an ip address", self->ip);
		vTaskDelete(NULL);
		return;
	}
	for (;;)
	{
		switch (self->state)
		{
		case CLIENT_STATE_ERROR:
			ESP_LOGW(TAG, "Error %08X", error);
			vTaskDelay(1000);
			self->state = CLIENT_STATE_UNKNOWN;
			continue;
			
		case CLIENT_STATE_IN_SESSION:
		case CLIENT_STATE_DISCONNECTED:
		case CLIENT_STATE_UNKNOWN:
			if (self->sock) close(self->sock);
	
			ESP_LOGI(__func__, "create socket");
			self->sock = error = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (error  < 0) { self->state = CLIENT_STATE_ERROR; continue;}
			setsockopt(self->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

			ESP_LOGI(__func__, "connect");
			error = connect(self->sock, (struct sockaddr*)&dest, sizeof(dest));
			if (error != 0) { self->state = CLIENT_STATE_ERROR; continue;}
				
			self->state = CLIENT_STATE_CONNECTED;
			continue;
		
		case CLIENT_STATE_CONNECTED:
			self->state = CLIENT_STATE_IN_SESSION;
			
			ESP_LOGI(__func__, "session");
			client_post_event(self, CLIENT_EVENT_SESSION_BEGIN, NULL, 0);
			while (CLIENT_STATE_IN_SESSION == self->state)
			{
				rxsize = 0, *rxbuffer = sizeof(packet_t);
				while (rxsize < *rxbuffer)
				{
					error = recv(self->sock, rxbuffer+rxsize, rxbuffer[0] - rxsize, MSG_WAITALL);
					if (error  < 0) { 
						if (errno == EAGAIN) continue;
						if (errno == EINTR) continue;
						self->state = CLIENT_STATE_ERROR; 
						break; 
					}
					if (error == 0) 
					{ 
						self->state = CLIENT_STATE_DISCONNECTED; 
						break; 
					}
					rxsize += error;
				}
				if (error <= 0) break;

				//error = packet_hash((packet_t*)rxbuffer);
				//if (error != 0) { self->state = CLIENT_STATE_ERROR; break; }

				client_post_event(self, CLIENT_EVENT_RECV_PACKET, rxbuffer, rxsize);
			}
			client_post_event(self, CLIENT_EVENT_SESSION_END, NULL, 9);
			continue;
		}
	}
}

int client_init(client_t* self)
{
	if (!self->task)
	{
		return xTaskCreate(client_task, "client task", 2*2048, self, 0, &self->task);
	}
	else
	{
		ESP_LOGW(TAG, "Skipping creation of task because client->task != NULL");
		return ESP_OK;
	}
}


int client_close_session(client_t* self)
{
	return ESP_OK;
}

int client_destroy(client_t* self)
{
	return ESP_OK;
}

int client_send(client_t* self, const void* data, size_t len)
{
	packet_t* p;
	packet_alloc(&p, data, len);
	p->header.hash = packet_hash(p);
	send(self->sock, p, p->header.size, 0);
	return ESP_OK;
}
