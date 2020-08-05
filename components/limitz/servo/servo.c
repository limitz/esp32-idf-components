#include "servo.h"
#include "esp_log.h"

#define TAG "SERVO"
//#define SERVO_DEBUG

inline double s_duty_for_us(double us)
{
	return (1<<15) * us / (20000);
}

int servo_init(servo_t* self)
{
	double duty = s_duty_for_us(0.5*(self->_minValue_us + self->_maxValue_us));
	ESP_LOGW(TAG, "claiming pin %d", self->pin);
	ESP_LOGI(TAG, "channel %d, timer %d, duty: %f", self->channel, LEDC_TIMER_0, duty);
	
	ledc_timer_config_t ltc = {
		.duty_resolution= LEDC_TIMER_15_BIT,
		.freq_hz = 50,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0
	};
	ledc_timer_config(&ltc);

	ledc_channel_config_t lcc = {
		.channel = self->channel,
		.duty = duty,
		.gpio_num = self->pin,
		.intr_type = LEDC_INTR_DISABLE,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_sel = LEDC_TIMER_0	
	};
	ledc_channel_config(&lcc);
	return ESP_OK;
}

int servo_destroy(servo_t* self)
{
	ledc_stop(LEDC_HIGH_SPEED_MODE, self->channel, 0);
	return ESP_OK;
}


int servo_set(servo_t* self, int v)
{
	if (v < self->minInput) v = self->minInput;
	else if (v > self->maxInput) v = self->maxInput;

	self->input = v;

	double us = ((v - self->minInput) / (double)(self->maxInput - self->minInput))
		  * (self->_maxValue_us - self->_minValue_us) + self->_minValue_us;

	double duty = s_duty_for_us(us);

#ifdef SERVO_DEBUG
	ESP_LOGW(TAG, "PIN %d CHANNEL %d INPUT", self->pin, self->channel);
	ESP_LOGI(TAG, "=> Input value: %d", v);
	ESP_LOGI(TAG, "   => Value in us: %f", us);
	ESP_LOGI(TAG, "      => Duty cycle:  %f", duty);
#endif

	ledc_set_duty(LEDC_HIGH_SPEED_MODE,self->channel, s_duty_for_us(us));
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, self->channel);

	return ESP_OK;
}
