#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_heap_caps.h>
#include <gui.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_event_loop.h>
#include <sys/param.h>
#include <esp_private/wifi.h>

#include <radio.h>

#define ESP_INTR_FLAG_DEFAULT 0


//	radio_broadcast(RADIO_PACKET_TYPE_CUSTOM + 3, fb->buf, fb->len);

typedef struct
{
	int32_t velocity;
	int32_t position;
	int32_t destination;

	struct speed
	{
		struct low 
		{
			uint32_t min;
			uint32_t accel;
			uint32_t decel;
			uint32_t max
		};

		struct high 
		{
			uint32_t min;
			uint32_t accel;
			uint32_t decel;
			uint32_t max;
		}
	};

	struct deceleration
	{

		uint32_t initial;
		uint32_t ultimate;
	uint32_t deceleration;
} monorail_state_t;

int handle_receive(const radio_packet_t* packet)
{
}

void app_main()
{
	RADIO.callbacks.on_receive=handle_receive;
	
	ESP_ERROR_CHECK( nvs_flash_init() );
	//ESP_ERROR_CHECK( gui_init() );


	ESP_ERROR_CHECK( radio_init() );

	esp_wifi_internal_set_fix_rate(ESPNOW_WIFI_IF, 1, 
			WIFI_PHY_RATE_11M_L);
			//WIFI_PHY_RATE_MCS7_SGI);
	//gui_start(false);

	for (;;)
	{

		ESP_ERROR_CHECK(_camera_capture());

	}
}


