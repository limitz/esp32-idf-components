#include <battery_monitor.h>
#include <unique_id.h>
#include <servo.h>
#include <apa102.h>
#include <ht16k33.h>
#include <wifi.h>

//static TaskHandle_t s_battery_monitor_task;


void app_refresh(apa102_t* ledstrip, apa102_color_t* buffer, size_t len, void* context)
{
	uint16_t p = ledstrip->phase;
	if (p<len) buffer[p] = 0xFF440033;
	if (p & 0xF) return;
	buffer[(p>>4)%len] ^= 0xFFFFFF00;
}

int wifi_connected(wifi_t* wifi, int  e)
{
	if (ESP_OK != e)
	{
		ESP_LOGW("APP", "Error in callback: %x", e);
	}
	return e;
}

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );

	ESP_LOGW("APP", "unique id = %s", unique_id());

	apa102_t ledstrip = APA102_DEFAULT;
	apa102_init(&ledstrip);

	ht16k33_t display1 = HT16K33_DEFAULT(0x71);
	ht16k33_t display2 = HT16K33_DEFAULT(0x73);
	//ht16k33_init(&display1);
	ht16k33_init(&display2);

	static wifi_t wifi = WIFI_PROVISION_OTA;
	wifi.ssid = CONFIG_WIFI_SSID;
	wifi.password = CONFIG_WIFI_PASSWORD;
	//wifi.ota_url = "https://ota.splendo.health/release/SplendoFitBridge";
	//wifi_init(&wifi);

	for(;;)	{
		apa102_update(&ledstrip, app_refresh, NULL);
		display1.text = "IT'S";
		display2.text = "TIME";
		//ht16k33_update(&display1);
		vTaskDelay(100/portTICK_PERIOD_MS);
		ht16k33_update(&display2);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}
	//apa102_destroy(&ledstrip);

#if 0
	servo_t steering = {
		.channel = LEDC_CHANNEL_0,
		.minValue = 500,
		.maxValue = 2800,
		.pin = 13
	};

	servo_init(&steering);
	servo_set(&steering, 0x8000);
//	servo_destroy(&steering);
//	servo_destroy(&steering);

	xTaskCreate(
		battery_monitor_task,
		"Battery monitor",
		2048, NULL, tskIDLE_PRIORITY,
		&s_battery_monitor_task);

	configASSERT(s_battery_monitor_task);
#endif
}
