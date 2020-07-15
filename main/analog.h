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

} analog_input_t;


#define ANALOG_INPUT_DEFAULT(gpio) { \
	.pin = gpio, \
	.channel = ADC1_GPIO##gpio##_CHANNEL, \
	.window_size = 4, \
	.min = 0, \
	.max = 1500, \
}

inline esp_err_t analog_reference_on(int pin)
{
	return adc2_vref_to_gpio(pin);
}

int analog_init();
int analog_input_init(analog_input_t*);
int analog_input_destroy(analog_input_t*);
int analog_input_update(analog_input_t*);

