#include "encoder.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"

volatile int s_counts[8] = {0};
volatile int s_states[8] = {0};

static void encoder_isr(void* arg)
{
	encoder_t* self = (encoder_t*)arg;
	int a = gpio_get_level(self->pin_a);
	int b = gpio_get_level(self->pin_b);
	switch ((s_states[self->unit] = (a << 1) | (b) | ((s_states[self->unit] & 3)<<4)))
	{
	case 0x01: a=+1; break;
	case 0x13: a=+1; break;
	case 0x32: a=+1; break;
	case 0x20: a=+1; break;
	case 0x02: a=-1; break;
	case 0x23: a=-1; break;
	case 0x31: a=-1; break;
	case 0x10: a=-1; break;
	default: a = 0; break;
	}
	s_counts[self->unit] += a;
	//self->value += d;
}

static int setup_pin(encoder_t* self, int pin)
{
	gpio_reset_pin(pin);
	gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
	gpio_isr_handler_add(pin, encoder_isr, self);
	gpio_set_direction(pin, GPIO_MODE_INPUT);
	if (self->pin_a < 34) 
	{
		gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
	}
	else
	{
		ESP_LOGW("ENCODER", "Pin %d needs manual pull resistor", pin);
	}
	gpio_intr_enable(pin);
	return 0;
}

int encoder_init(encoder_t* self)
{
	ESP_LOGW("ENCODER", "Claims pin %d and %d", self->pin_a, self->pin_b);
	gpio_install_isr_service(0);
	setup_pin(self,self->pin_a);
	setup_pin(self,self->pin_b);
	return 0;
}

int encoder_update(encoder_t* self)
{
	self->value = s_counts[self->unit];
	return 0;
}

int encoder_destroy(encoder_t* self)
{
	return 0;
}
