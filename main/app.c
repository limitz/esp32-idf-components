#include <battery_monitor.h>
#include <unique_id.h>
#include <servo.h>
#include <apa102.h>
#include <ht16k33.h>
#include <wifi.h>
#include <analog.h>

//static TaskHandle_t s_battery_monitor_task;

#define BRIGHTNESS 0x1F

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

	analog_init();
	analog_reference_on(27);
	analog_input_t analog_in = ANALOG_INPUT_DEFAULT(33);
	analog_input_init(&analog_in);
	apa102_t ledstrip = APA102_DEFAULT;
	apa102_init(&ledstrip);

	servo_t steering = SERVO_1_DEFAULT;
	servo_init(&steering);

	ESP_LOGW("SERVO CONFIG", "MINMAX INPUT %d %d", steering._minValue_us, steering._maxValue_us);


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
		analog_input_update(&analog_in);

		//ESP_LOGI("ANALOG", "Last value: 0x%03X => %d mV (@ window index: %d)", 
		//		analog_in.last_value, 
		//		analog_in.last_voltage, 
		//		analog_in._window_index);

		//ESP_LOGI("ANALOG", "Sliding window: %d mV (%d out of %d measurements)", 
		//		analog_in.voltage, 
		//		analog_in._window_count,
		//		analog_in.window_size);
		
		//apa102_update(&ledstrip, app_refresh, NULL);	
		
		servo_set(&steering, (analog_in.voltage / 17) - 100);
		//display1.text = "IT'S";
		//display2.text = "TIME";
		//ht16k33_update(&display1);
		//ht16k33_update(&display2);
		
		vTaskDelay(1);
	}

	servo_destroy(&steering);
	analog_input_destroy(&analog_in);
	apa102_destroy(&ledstrip);
}
