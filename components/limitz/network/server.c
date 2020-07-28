#include "server.h"

#include <esp_system.h>
#include <esp_log.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

static void server_task(void* arg)
{
	server_t* self = (server_t*) arg;

	int error;
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
			continue;
			
		case SERVER_STATE_IN_SESSION:
		case SERVER_STATE_UNKNOWN:
			if (self->sock) close(self->sock);
			
			self->sock = error = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (error  < 0) { self->state = SERVER_STATE_ERROR; continue;}
			
			error = bind(self->sock, (struct sockaddr*)&dest, sizeof(dest));
			if (error != 0) { self->state = SERVER_STATE_ERROR; continue;}
				
			error = listen(self->sock, 10);
			if (error != 0) { self->state = SERVER_STATE_ERROR; continue; }

			self->state = SERVER_STATE_LISTENING;
			continue;
		
		case SERVER_STATE_LISTENING:
			error = accept(self->sock, (struct sockaddr*) &source, &addr_len);
			if (error != 0) { self->state = SERVER_STATE_ERROR; continue; }
			self->state = SERVER_STATE_IN_SESSION;

			while (SERVER_STATE_IN_SESSION == self->state)
			{
				rxsize = 0, *rxbuffer = sizeof(packet_t);
				while (rxsize < *rxbuffer)
				{
					rxsize += error = recv(self->sock, rxbuffer+rxsize, rxbuffer[0] - rxsize, 0);
					if (error  < 0) { self->state = SERVER_STATE_ERROR; break; }
					if (error == 0) { self->state = SERVER_STATE_LISTENING; break; }
				}
				if (error <= 0) break;

				error = packet_hash((packet_t*)rxbuffer);
				if (error != 0) { self->state = SERVER_STATE_ERROR; break; }

				error = xRingbufferSend(self->rbuffer, rxbuffer, rxsize, pdMS_TO_TICKS(10000));
				if (pdTRUE!= error) { self->state = SERVER_STATE_ERROR; break; }
			}
			continue;
		}
	}
}

int server_init(server_t* self)
{
	return ESP_OK;
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
	return ESP_OK;
}
