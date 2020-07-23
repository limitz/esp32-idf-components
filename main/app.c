#include <battery_monitor.h>
#include <unique_id.h>
#include <servo.h>
#include <encoder.h>
#include <apa102.h>
#include <ht16k33.h>
#include <wifi.h>
#include <adc.h>
#include "driver/gpio.h"

//static TaskHandle_t s_battery_monitor_task;

#define BRIGHTNESS 0x1F

#if (CONFIG_LMTZ_APA102_NUM_LEDS > 0)
void app_refresh(apa102_t* ledstrip, apa102_color_t* buffer, size_t len, void* context)
{
	if (ledstrip->phase & 0xF) return;

	uint16_t p = (ledstrip->phase>>4) % (CONFIG_LMTZ_APA102_NUM_LEDS/2);
	for (int i=0; i<CONFIG_LMTZ_APA102_NUM_LEDS; i++)
	{
		if (i<CONFIG_LMTZ_APA102_NUM_LEDS/2 &&p == i)
		{
			buffer[i] = APA102_COLOR(0xFF,0x00, 0x11, BRIGHTNESS);
			buffer[CONFIG_LMTZ_APA102_NUM_LEDS - i - 1] = APA102_COLOR(0xFF,0x33, 0x00, BRIGHTNESS);
		}
		else 
		{
			int b = buffer[i] & BRIGHTNESS;
			buffer[i] = (buffer[i] ^ b) | (b*5/6); // APA102_COLOR(0x00,0x00, 0x00, 0x00);
		}

	}
}
#endif

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

#if CONFIG_LMTZ_ADC1
	adc_t joy_x = ADC1;
	adc_init(&joy_x);	
#endif 

#if CONFIG_LMTZ_ADC2
	adc_t joy_y = ADC2;
	adc_init(&joy_y);
#endif
#if (CONFIG_LMTZ_APA102_NUM_LEDS > 0)
	apa102_t ledstrip = APA102_DEFAULT;
	apa102_init(&ledstrip);
#endif
#if CONFIG_LMTZ_ENCODER_1
	encoder_t enc1 = ENCODER_1_DEFAULT;
	encoder_init(&enc1);
#endif
#if (CONFIG_SERVO1 && CONFIG_SERVO2)	
	servo_t servo1 = SERVO1;
	servo_t servo2 = SERVO2;
	servo_init(&servo1);
	servo_init(&servo2);
#endif

	//ht16k33_t display1 = HT16K33_DEFAULT(0x71);
	//ht16k33_t display2 = HT16K33_DEFAULT(0x73);
	//ht16k33_init(&display1);
	//ht16k33_init(&display2);

	//static wifi_t wifi = WIFI_PROVISION_OTA;
	//wifi.ssid = CONFIG_WIFI_SSID;
	//wifi.password = CONFIG_WIFI_PASSWORD;
	//wifi.ota_url = "https://ota.splendo.health/release/SplendoFitBridge";
	//wifi_init(&wifi);

	for(;;)	
	{
		int x = 1900;
		int y = 1900;
#if CONFIG_LMTZ_ADC1
		adc_update(&joy_x);
		x = joy_x.result;
#endif

#if CONFIG_LMTZ_ADC2
		adc_update(&joy_y);
		y = joy_y.result;
#endif

#if CONFIG_LMTZ_ENCODER_1
		encoder_update(&enc1);
		ESP_LOGI("ENCODER", "%d", a,b,enc1.value);
#endif
		ESP_LOGI("JOY", "x: %d, y: %d", x, y);

		//ESP_LOGI("ANALOG", "Last value: 0x%03X => %d mV (@ window index: %d)", 
		//		analog_in.last_value, 
		//		analog_in.last_voltage, 
		//		analog_in._window_index);

		//ESP_LOGI("ANALOG", "Sliding window: %d mV (%d out of %d measurements)", 
		//		analog_in.voltage, 
		//		analog_in._window_count,
		//		analog_in.window_size);
		
		//apa102_update(&ledstrip, app_refresh, NULL);	
		
		//servo_set(&servo1, (x / 19) - 100);
		//servo_set(&servo2, (y/ 19) - 100);
		

		//display1.text = "IT'S";
		//display2.text = "TIME";
		//ht16k33_update(&display1);
		//ht16k33_update(&display2);
		
		vTaskDelay(1);
	}
}
