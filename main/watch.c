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
#include "rc.h"

void app_main()
{
	RC.role = RC_ROLE_CONTROLLER;

	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( i2c_init() );
	ESP_ERROR_CHECK( rc_init() );
	ESP_ERROR_CHECK( ft5206_init() );

	ESP_LOGW("FT5206", "VENDOR: %02x CHIP: %02x", 
			FT5206.info.ctpm_vendor_id, 
			FT5206.info.chip_vendor_id);

	for (;;)
	{
		ft5206_read_touches();

		if (FT5206.touch.count == 1)
		{
			ft5206_touch_t p = FT5206.touch.points[0];
			RC.payload.steering = 1000 * (120-p.x) / 120;
			RC.payload.throttle = 1000 * (120-p.y) / 120;
			
		}
		else
		{
			RC.payload.steering = 0;
			RC.payload.throttle = 0;
		}

		//ESP_LOG_BUFFER_HEX("PAYLOAD", &RC.packets.controls.payload, sizeof(rc_payload_t));
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}
