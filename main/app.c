#include <battery_monitor.h>
#include <unique_id.h>
#include <servo.h>
#include <apa102.h>
#include <ht16k33.h>

//static TaskHandle_t s_battery_monitor_task;


void app_refresh(apa102_t* ledstrip, apa102_color_t* buffer, size_t len, void* context)
{
	uint16_t p = ledstrip->phase;
	if (p<len) buffer[p] = 0xFF440033;
	if (p & 0xF) return;
	buffer[(p>>4)%len] ^= 0xFFFFFF00; 
}

void app_main()
{
	ESP_LOGW("APP", "unique id = %s", unique_id());
	
	apa102_t ledstrip = APA102_DEFAULT;
	apa102_init(&ledstrip);

	ht16k33_t display = HT16K33_DEFAULT;
	display.address = 0x71;

	ht16k33_init(&display);

	ESP_LOGW("display", "%d %d 0x%02x", display.i2c_pin_sda, display.i2c_pin_scl, display.address);	

	for(;;)	{
		apa102_refresh(&ledstrip, app_refresh, NULL);
		ht16k33_display(&display);
		vTaskDelay(10/portTICK_PERIOD_MS);
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
