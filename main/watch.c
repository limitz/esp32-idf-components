#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <i2cbus.h>
#include <ft5206.h>
#include <bma.h>

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );

	i2c_init();

	ESP_ERROR_CHECK(ft5206_init(&FT5206));
	ESP_LOGW("FT5206", "VENDOR: %02x CHIP: %02x", FT5206.id.vendor, FT5206.id.chip);

	for (;;)
	{
		ESP_ERROR_CHECK(ft5206_update(&FT5206));
		ESP_LOGI("FT5206", "TOUCHES: %03X", FT5206.num_touches);
		
		for (int i=0 ;i<FT5206.num_touches; i++)
		{
			ft5206_touch_t t = FT5206.touches[i];
			ESP_LOGI("FT5206", "x: 0x%03X y: 0x%03X (id = 0x%01X)", t.x, t.y, t.id);
		}
		vTaskDelay(100);
	
	}
}
