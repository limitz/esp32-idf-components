#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "unique_id.h"
#include "adc.h"
#include "lipo.h"
#include "servo.h"

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_LOGW("APP", "unique id = %s", unique_id());


	servo_t sx = SERVO1;
	servo_t sy = SERVO2;
	joystick_t joystick = JOYSTICK;

	servo_init(&sx);
	servo_init(&sy);
	adc_init(&joystick.x);
	adc_init(&joystick.y);

	for (;;)
	{
		adc_update(&joystick.x);
		adc_update(&joystick.y);

		int x = ((joystick.x.result-1880) / 13) * 10;
		int y = ((joystick.y.result-1880) / 13) * 10;

		servo_set(&sx, x);
		servo_set(&sy, y);

		vTaskDelay(1);
	}
}
