#include "macaddr.h"
#include <esp_system.h>
#include <esp_log.h>
macaddr_t macaddr()
{
	static macaddr_t result = {0};
	if (!*result.ptr) esp_efuse_mac_get_default(result.ptr);
	return result;
}

macaddr_t macaddr_parse(const char* s)
{
	macaddr_t r = { 0 };
	int ptr[6];
	if (6 != sscanf(s, "%02x:%02x:%02x:%02x:%02x:%02x", 
				ptr+0, ptr+1, ptr+2, 
				ptr+3, ptr+4, ptr+5))
	{
		ESP_LOGE(__func__, "Invalid input string %s", s);
	}
	for (int i=0; i<6; i++)
	{
		r.ptr[i] = (uint8_t)ptr[i];
	}
	return r;
}

