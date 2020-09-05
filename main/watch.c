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

	ESP_ERROR_CHECK(ft5206_init());
	ESP_LOGW("FT5206", "VENDOR: %02x CHIP: %02x", 
			FT5206.info.ctpm_vendor_id, 
			FT5206.info.chip_vendor_id);

	ESP_LOG_BUFFER_HEX("DUMP", &FT5206, sizeof(FT5206));

	for (;;)
	{
		ESP_ERROR_CHECK(ft5206_read_touches());

		ESP_LOGI("FT5206", "GESTURE: %02X, TOUCHES: %03X", 
				FT5206.touch.gesture,
				FT5206.touch.count);
		
		for (int i=0 ;i<FT5206.touch.count; i++)
		{
			ft5206_touch_t p = FT5206.touch.points[i];
			ESP_LOGI("FT5206", "x: 0x%03X y: 0x%03X (id = 0x%01X)", p.x, p.y, p.id);
		}
		vTaskDelay(10);
	
	}
}
