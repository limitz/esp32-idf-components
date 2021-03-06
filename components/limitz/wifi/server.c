#include "server.h"

#include <esp_system.h>
#include <esp_log.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#define TAG "SERVER"

ESP_EVENT_DEFINE_BASE(SERVER_EVENTS);

void server_task(void* arg)
{
	server_t* self = (server_t*) arg;
	int opt = 1;
	int error = 1;
	uint8_t  rxbuffer[256] = { 0 };
	uint16_t rxsize = 0;
	
	struct sockaddr_in dest, source;
	uint32_t addr_len = sizeof(source);
	dest.sin_family = AF_INET;
	dest.sin_port = htons(self->port);
	dest.sin_addr.s_addr = htonl(INADDR_ANY);

	for (;;)
	{
		switch (self->state)
		{
		case SERVER_STATE_ERROR:
			ESP_LOGW("SERVER", "Error %08X", error);
			self->state = SERVER_STATE_UNKNOWN;
			vTaskDelay(1000);
			continue;
			
		case SERVER_STATE_IN_SESSION:
		case SERVER_STATE_UNKNOWN:
			if (self->sock) close(self->sock);

			ESP_LOGI(__func__, "create socket");
			self->sock = error = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (error  < 0) { self->state = SERVER_STATE_ERROR; continue;}
			setsockopt(self->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

			ESP_LOGI(__func__, "bind");
			error = bind(self->sock, (struct sockaddr*)&dest, sizeof(dest));
			if (error != 0) { self->state = SERVER_STATE_ERROR; continue;}
				
			ESP_LOGI(__func__, "listen");
			error = listen(self->sock, 10);
			if (error != 0) { self->state = SERVER_STATE_ERROR; continue; }

			self->state = SERVER_STATE_LISTENING;
			continue;
		
		case SERVER_STATE_LISTENING:

			ESP_LOGI(__func__, "accept");
			self->client = error = accept(self->sock, (struct sockaddr*) &source, &addr_len);
			if (error <0) { self->state = SERVER_STATE_ERROR; continue; }
			
			ESP_LOGI(__func__, "session");
			self->state = SERVER_STATE_IN_SESSION;

			while (SERVER_STATE_IN_SESSION == self->state)
			{
				rxsize = 0, *rxbuffer = sizeof(packet_t);
				while (rxsize < *rxbuffer)
				{
					error = recv(self->client, rxbuffer+rxsize, rxbuffer[0] - rxsize, MSG_WAITALL);
					if (error < 0)
					{
						if (errno == EAGAIN) continue;
						if (errno == EINTR) continue;
						self->state = SERVER_STATE_ERROR; 
						error = errno;	
						break;
					}
					if (error == 0)
					{
						self->state = SERVER_STATE_LISTENING; 
						break; 
					}
					rxsize += error;
				}
				if (error <= 0) break;

				//error = packet_hash((packet_t*)rxbuffer);
				//if (error != 0) { self->state = SERVER_STATE_ERROR; break; }

				//error = xRingbufferSend(self->rbuffer, rxbuffer, rxsize, pdMS_TO_TICKS(10000));
				//if (pdTRUE!= error) { self->state = SERVER_STATE_ERROR; break; }
			
				if (self->event_loop)
				{
					esp_event_post_to(
						self->event_loop,
						SERVER_EVENTS, SERVER_EVENT_RECV_PACKET,
						rxbuffer, rxsize, 100/portTICK_PERIOD_MS);
				}
				else 
				{
					esp_event_post(
						SERVER_EVENTS, SERVER_EVENT_RECV_PACKET,
						rxbuffer, rxsize, 100/portTICK_PERIOD_MS);
				}

			}
			continue;
		}
	}
}

int server_init(server_t* self)
{
	if (!self->task)
	{
		return xTaskCreate(server_task, "server task", 2*2*2048, self, 0, &self->task);
	}
	else
	{
		ESP_LOGW(TAG, "Skipping creation of task because server->task != NULL");
		return ESP_OK;
	}
}


int server_close_session(server_t* self)
{
	return ESP_OK;
}

int server_destroy(server_t* self)
{
	return ESP_OK;
}

int server_send(server_t* self, const void* data, size_t len)
{
	packet_t* p;
	packet_alloc(&p, data, len);
	p->header.hash = packet_hash(p);
	send(self->client, p, p->header.size, 0);
	return ESP_OK;
}
