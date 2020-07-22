#pragma once

typedef struct
{
	int pin_a;
	int pin_b;
	int min_value;
	int max_value;
	int filter_value;
	int unit;
	int channel;
	int value;
	
} encoder_t;

#if CONFIG_LMTZ_ENCODER_1
#define ENCODER_1_DEFAULT { \
	.pin_a = CONFIG_LMTZ_ENCODER_1_PIN_A,\
	.pin_b = CONFIG_LMTZ_ENCODER_1_PIN_B,\
	.min_value = CONFIG_LMTZ_ENCODER_1_MIN_VALUE,\
	.max_value = CONFIG_LMTZ_ENCODER_1_MAX_VALUE,\
	.filter_value = CONFIG_LMTZ_ENCODER_1_FILTER_VALUE,\
	.unit = CONFIG_LMTZ_ENCODER_1_UNIT, \
	.channel = CONFIG_LMTZ_ENCODER_1_CHANNEL, \
}
#endif

int encoder_init(encoder_t* self);
int encoder_update(encoder_t* self);
int encoder_destroy(encoder_t* self);


