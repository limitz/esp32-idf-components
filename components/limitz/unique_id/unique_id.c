#include "unique_id.h"
#include <macaddr.h>
#include <esp_system.h>

static char s_unique[16] = {0};

inline char hex4(uint8_t v)
{
	return (v > 9) ? (v - 10) + 'A' : v + '0';
}

inline void hex(char* dst, uint8_t v)
{
	dst[0] = hex4(v >>  4);
	dst[1] = hex4(v & 0xF);
}

static int init()
{
	if (!*s_unique) 
	{
		macaddr_t ownaddr = macaddr();
		
		strcpy(s_unique, CONFIG_LMTZ_UNIQUE_ID_PREFIX);
		int offset = strlen(s_unique);
		int max = sizeof(s_unique)-7;
		if (offset > max) offset = max;
		
		memcpy(s_unique + offset, ownaddr.ptr, 6);
		hex(s_unique+offset+0, (uint8_t)s_unique[offset+3]);
		hex(s_unique+offset+2, (uint8_t)s_unique[offset+4]);
		hex(s_unique+offset+4, (uint8_t)s_unique[offset+5]);
		s_unique[offset+6] = 0;
	}
	return ESP_OK;
}


#if (CONFIG_LMTZ_UNIQUE_ID_SRC_MANUAL)
const char* unique_id() { return CONFIG_LMTZ_UNIQUE_ID_MANUAL; }
#elif (CONFIG_LMTZ_UNIQUE_ID_SRC_MAC)
const char* unique_id() 
{
	ESP_ERROR_CHECK(init());
	return s_unique;
}
#endif
