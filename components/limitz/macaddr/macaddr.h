#pragma once

#include <stdint.h>
#include <esp_now.h>
#define MACADDR_T macaddr_t
#define MACADDR_FMT "[%02x:%02x:%02x:%02x:%02x:%02x]"
#define MACADDR_ARGS(a) (a)->ptr[0],(a)->ptr[1],(a)->ptr[2],(a)->ptr[3],(a)->ptr[4],(a)->ptr[5]
typedef struct
{
	uint8_t ptr[ESP_NOW_ETH_ALEN + 3];
}
macaddr_t;
macaddr_t macaddr();
macaddr_t macaddr_parse(const char* s);

