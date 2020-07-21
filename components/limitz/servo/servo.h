#pragma once

#include <driver/ledc.h>

typedef struct
{
	ledc_channel_t channel;
	int _minValue_us;
	int _maxValue_us;
	int minInput;
	int maxInput;
	int duty;
	int pin;
} servo_t;

#if (CONFIG_LMTZ_SERVO_1)
#define SERVO_1_DEFAULT { \
	.channel = CONFIG_LMTZ_SERVO_1_CHANNEL, \
	._minValue_us = CONFIG_LMTZ_SERVO_1_MIN_VALUE, \
	._maxValue_us = CONFIG_LMTZ_SERVO_1_MAX_VALUE, \
	.minInput = CONFIG_LMTZ_SERVO_1_MIN_INPUT,\
	.maxInput = CONFIG_LMTZ_SERVO_1_MAX_INPUT,\
	.pin = CONFIG_LMTZ_SERVO_1_PIN, \
}
#endif

#if (CONFIG_LMTZ_SERVO_2)
#define SERVO_2_DEFAULT { \
	.channel = CONFIG_LMTZ_SERVO_2_CHANNEL, \
	._minValue_us = CONFIG_LMTZ_SERVO_2_MIN_VALUE, \
	._maxValue_us = CONFIG_LMTZ_SERVO_2_MAX_VALUE, \
	.minInput = CONFIG_LMTZ_SERVO_2_MIN_INPUT,\
	.maxInput = CONFIG_LMTZ_SERVO_2_MAX_INPUT,\
	.pin = CONFIG_LMTZ_SERVO_2_PIN, \
}
#endif

int servo_init(servo_t* ptr);
int servo_destroy(servo_t* self);
int servo_set(servo_t*, int v);

