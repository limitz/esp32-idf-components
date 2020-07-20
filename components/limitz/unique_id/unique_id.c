#include "unique_id.h"
#include "esp_system.h"

static char s_unique[7] = {0};

inline char hex4(uint8_t v)
{
	return (v > 9) ? (v - 10) + 'A' : v + '0';
}

inline void hex(char* dst, uint8_t v)
{
	dst[0] = hex4(v >>  4);
	dst[1] = hex4(v & 0xF);
}

const char* unique_id() 
{
	if (!*s_unique) 
	{
		esp_err_t err = esp_efuse_mac_get_default((uint8_t*)s_unique);
		if (ESP_OK != err)
		{
			return 0;
		}

		hex(s_unique+0, (uint8_t)s_unique[3]);
		hex(s_unique+2, (uint8_t)s_unique[4]);
		hex(s_unique+4, (uint8_t)s_unique[5]);
		s_unique[6] = 0;
	}
	return s_unique;
}
