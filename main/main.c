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

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "unique_id.h"
#include "adc.h"
#include "lipo.h"
#include "servo.h"
#include "wifi.h"
#include "server.h"
#include "client.h"

#if CONFIG_LMTZ_SERVER
static server_t server = SERVER_DEFAULT;
static servo_t sx = SERVO1;
static servo_t sy = SERVO2;

void server_event_handler(void* arg, esp_event_base_t base, int event_id, void* event_data)
{

	switch (event_id)
	{
		case SERVER_EVENT_RECV_PACKET:
		{
			packet_t* packet = (packet_t*) event_data;
			int16_t* v = (int16_t*) packet->data;

			esp_log_buffer_hex(__func__, packet, packet->header.size);
			int16_t x = v[0];
			int16_t y = v[1];

			servo_set(&sx, x);
			servo_set(&sy, y);
		}
		break;
	}
}
#endif

#if CONFIG_LMTZ_CLIENT
static client_t client = CLIENT_DEFAULT;
static joystick_t joystick = JOYSTICK;
#endif

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_LOGW("APP", "unique id = %s", unique_id());

	wifi_t wifi = WIFI_DEFAULT;
	wifi_init(&wifi);

#if CONFIG_LMTZ_SERVER
	server_init(&server);
	servo_init(&sx);
	servo_init(&sy);
	
	esp_event_handler_register(
			SERVER_EVENTS, ESP_EVENT_ANY_ID,
			&server_event_handler, NULL);

	for (;;)
	{
		vTaskDelay(2);
	}
#endif

#if CONFIG_LMTZ_CLIENT
	client_init(&client);
	adc_init(&joystick.x);
	adc_init(&joystick.y);


	for (;;)
	{

		adc_update(&joystick.x);
		adc_update(&joystick.y);

		int16_t v[2];
		v[0] = ((joystick.x.result-1880) / 13) * 10;
		v[1] = ((joystick.y.result-1880) / 13) * 10;

		ESP_LOGI("CLIENT", "X:%d Y:%d", v[0], v[1]);

		if (client.state == CLIENT_STATE_IN_SESSION)
			client_send(&client, v, 4);

		vTaskDelay(2);
	}
#endif
}
