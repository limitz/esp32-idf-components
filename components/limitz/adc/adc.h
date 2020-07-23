#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "driver/adc.h"

typedef struct
{
	int pin;
	int channel;
	int min, max;
	int last_value;
	int last_voltage;
	int voltage;

	int window_size;
	int *_window;
	int _window_sum;
	int _window_index;
	int _window_count;

} adc_t;


#ifdef CONFIG_LMTZ_ADC1
#define ADC1 { \
	.pin = CONFIG_LMTZ_ADC1_PIN, \
	.channel = CONFIG_LMTZ_ADC1_CHANNEL, \
	.window_size = CONFIG_LMTZ_ADC1_WINDOW_SIZE, \
	.min = 0, \
	.max = 1500, \
}
#endif

#ifdef CONFIG_LMTZ_ADC2
#define ADC2 { \
	.pin = CONFIG_LMTZ_ADC2_PIN, \
	.channel = CONFIG_LMTZ_ADC2_CHANNEL, \
	.window_size = CONFIG_LMTZ_ADC2_WINDOW_SIZE, \
	.min = 0, \
	.max = 1500, \
}
#endif

inline int adc_output_vref(int pin) { return adc2_vref_to_gpio(pin); }

int adc_init(adc_t*);
int adc_destroy(adc_t*);
int adc_update(adc_t*);

