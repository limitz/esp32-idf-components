#include "servo.h"

inline double s_duty_for_pct(double percentage)
{
	if (percentage <= 0) return 0;
	if (percentage > 100) return 100;

	return (percentage / 100.0) * ((2<<14)-1); // LEDC_TIMER_15_BIT
}

inline double s_duty_for_us(double us)
{
		return s_duty_for_pct(((us * 100.0)/(1000000/50)));
}

int servo_init(servo_t* self)
{
	self->channel = LEDC_CHANNEL_0;

	ledc_timer_config_t ltc = {
		.duty_resolution= LEDC_TIMER_15_BIT,
		.freq_hz = 50,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.timer_num = LEDC_TIMER_0
	};
	ledc_timer_config(&ltc);

	ledc_channel_config_t lcc = {
		.channel = self->channel,
		.duty = s_duty_for_us(0.5*(self->minValue + self->maxValue)),
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


int servo_set(servo_t* self, int us)
{;
	ledc_set_duty(LEDC_HIGH_SPEED_MODE,self->channel, s_duty_for_us(us));
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, self->channel);

	return ESP_OK;
}
