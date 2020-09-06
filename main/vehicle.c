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
	RC.role = RC_ROLE_VEHICLE;

	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( rc_init() );

	for (;;)
	{	
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
