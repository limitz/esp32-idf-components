#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
	uint8_t size;
	uint8_t csum;
	uint8_t type;
	uint8_t index;
	uint8_t part;
	uint8_t flags;
	uint8_t sender[6];
} packet_header_t;


typedef struct
{
	packet_header_t header;
	uint8_t data[0];
} packet_t;


int packet_alloc(packet_t** packet, const void* data, size_t len);
int packet_hash(packet_t* packet);

