#include "encoder.h"

void encoder_init(encoder_t* self)
{
	pcnt_config_t config = {
		.lctrl_mode = PCNT_MODE_KEEP,
		.hctrl_mode = PCNT_MODE_REVERSE,
		.pos_mode = PCNT_MODE_INC,
		.neg_mode = PCNT_MODE_DEC,
	};

	config.pulse_gpio_num = self->pin_a;
	config.ctrl_gpio_num  = self->pin_b;
	config.counter_h_lim = self->max;
	config.counter_l_lim = self->min;
	config.unit = self->idx;
	config.channel = self->channel;

	pcnt_unit_config(&config);
	pcnt_counter_clear(self->idx);
	pcnt_set_filter_value(self->idx, 256);
	pcnt_filter_enable(self->idx);
}

void encoder_destroy(encoder_t* self)
{
	pcnt_counter_clear(self->idx);
	//ISRs?
}
