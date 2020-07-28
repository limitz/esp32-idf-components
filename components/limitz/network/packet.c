#include "packet.h"


#include <esp_log.h>
#include <esp_system.h>

int packet_alloc(packet_t** packet, const void* data, size_t len)
{
	size_t size = sizeof(packet_t) + len;
	
	packet_t* p = (packet_t*) malloc(size);
	if (!p) return ESP_FAIL;

	bzero(p, size);
	p->header.size = size;
	memcpy(p->data, data, len);
	*packet = p;
	return ESP_OK;
}

int packet_hash(packet_t* packet)
{
	uint8_t hash = 0;
	uint8_t* ptr = (uint8_t*) packet;
	for (int i=0; i<packet->header.size; i++) hash ^= ptr[i];
	return hash;
}
