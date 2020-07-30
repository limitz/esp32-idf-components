#pragma once

#include <stdint.h>
#include <esp_now.h>
#define MACADDR_T macaddr_t


typedef struct
{
	uint8_t ptr[ESP_NOW_ETH_ALEN + 3];
}
macaddr_t;
macaddr_t macaddr();
macaddr_t macaddr_parse(const char* s);

