#include "battery_monitor.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define TAG "BATMON"
#define ADC_DEFAULT_VREF 1100
#define ADC_NUM_SAMPLES 64
#define BATTERY_MONITOR_MAX_WINDOW 16

// TODO works for the adafruit huzzah esp32, but should be moved to a config struct
static esp_adc_cal_characteristics_t *s_adc_char;
static const adc1_channel_t s_adc_chan = ADC1_GPIO35_CHANNEL;
static const adc_bits_width_t s_adc_bits = ADC_WIDTH_BIT_12;
static const adc_atten_t s_adc_atten = ADC_ATTEN_DB_11;

static int s_percentage = -1;
static int s_voltage = -1;
static int s_window[BATTERY_MONITOR_MAX_WINDOW] = {0};
static int s_window_size = 8;
static int s_window_sum = 0;
static int s_window_index = 0;
static int s_window_count = 0;
static int s_interval = 1000;
static bool s_started = false;

static int s_percent_to_volt_map[] = {
	3270, 
	3610, 3690, 3710, 3730, 3750, 
	3770, 3790, 3800, 3820, 3840, 
	3850, 3870, 3910, 3950, 3980,
	4020, 4080, 4110, 4150, 4200
};

#if 0
static int percent_to_volt(int pct)
{
	if (pct < 0 || pct > 100) return -1;
	
	return s_percent_to_volt_map[(pct / 5) * 5];
}
#endif

static int volt_to_percent( int mv)
{
	int v0 = s_percent_to_volt_map[0];
	if (mv < v0) return 0;

	for (int i=1; i <= 20; i++)
	{
		int v1 = s_percent_to_volt_map[i];
		if (mv < v1)
		{
			return (5 * (i-1)) + (5 * (mv - v0)) / (v1 - v0);
		}
		v0 = v1;
	}
	return 100;
}

#if 0
static void check_efuse()
{
	if (ESP_OK == esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP))
	{
		ESP_LOGI(TAG, "eFuse 2 point supported");
	}
	else 
	{
		ESP_LOGI(TAG, "eFuse 2 point not supported");
	}

	if (ESP_OK == esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF))
	{
		ESP_LOGI(TAG, "eFuse Vref supported");
	}
	else 
	{
		ESP_LOGI(TAG, "eFuse Vref not supported");
	}
}
#endif

static void check_cal_type(esp_adc_cal_value_t type)
{
	switch (type)
	{
	case ESP_ADC_CAL_VAL_EFUSE_TP:
		ESP_LOGI(TAG, "Calibrated using 2 point value");
		break;
	case ESP_ADC_CAL_VAL_EFUSE_VREF:
		ESP_LOGI(TAG,"Calibrated using eFuse Vref");
		break;
	default:
		ESP_LOGW(TAG,"Calibrated using default Vref");
		break;
	}

}

esp_err_t battery_monitor_set_window(int window)
{
	s_window_size = window;
	return ESP_OK;
}

esp_err_t battery_monitor_get_window(int* window)
{
	if (window) *window = s_window_size;
	return ESP_OK;
}

esp_err_t battery_monitor_set_interval(int interval)
{
	s_interval = interval;
	return ESP_OK;
}

esp_err_t battery_monitor_get_interval(int* interval)
{
	if (interval) *interval = s_interval;
	return ESP_OK;
}

esp_err_t battery_get_percentage(int* percentage)
{
	if (percentage) *percentage = s_percentage;
	return ESP_OK;
}

esp_err_t battery_get_voltage(float* voltage)
{
	if (voltage) *voltage = s_voltage;
	return ESP_OK;
}

void battery_monitor_start()
{
	if (s_started) return;
	s_started = true;

	adc1_config_width(s_adc_bits);
	adc1_config_channel_atten(s_adc_chan, s_adc_atten);
	s_adc_char = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t type = esp_adc_cal_characterize(
			ADC_UNIT_1,s_adc_atten,s_adc_bits,
			ADC_DEFAULT_VREF,s_adc_char);
	check_cal_type(type);
}

void battery_monitor_stop()
{
	if (!s_started) return;
	s_started = false;

	free(s_adc_char);
}

void battery_monitor_task(void* param)
{
	battery_monitor_start();
	for (;;)
	{
		battery_monitor_step();
		vTaskDelay(s_interval/portTICK_PERIOD_MS);
	}
}

esp_err_t battery_monitor_step()
{
	uint32_t adc_reading = 0;
	for (int i=0; i<ADC_NUM_SAMPLES; i++) 
	{
		adc_reading += adc1_get_raw(s_adc_chan);
	}
	adc_reading /= ADC_NUM_SAMPLES;

	int v = esp_adc_cal_raw_to_voltage(adc_reading, s_adc_char) * 2;
	s_window_sum += v - s_window[s_window_index];
	s_window[s_window_index] = v;
	s_window_index = (s_window_index + 1) % s_window_size;

	if (s_window_count < s_window_size) s_window_count++;
		
	s_voltage = s_window_sum / s_window_count;
	s_percentage = volt_to_percent(s_voltage);

	ESP_LOGI(TAG, "[reading: %d = %d mV] samples: %d, avg: %d mV, percentage: %d%%",
				adc_reading, v,s_window_count, s_voltage, s_percentage);

	return ESP_OK;
}
