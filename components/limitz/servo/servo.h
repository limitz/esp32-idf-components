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
	int input;
} servo_t;

#if (CONFIG_LMTZ_SERVO1)
#define SERVO1 { \
	.channel = CONFIG_LMTZ_SERVO1_CHANNEL, \
	._minValue_us = CONFIG_LMTZ_SERVO1_MIN_VALUE, \
	._maxValue_us = CONFIG_LMTZ_SERVO1_MAX_VALUE, \
	.minInput = CONFIG_LMTZ_SERVO1_MIN_INPUT,\
	.maxInput = CONFIG_LMTZ_SERVO1_MAX_INPUT,\
	.pin = CONFIG_LMTZ_SERVO1_PIN, \
}
#endif

#if (CONFIG_LMTZ_SERVO2)
#define SERVO2 { \
	.channel = CONFIG_LMTZ_SERVO2_CHANNEL, \
	._minValue_us = CONFIG_LMTZ_SERVO2_MIN_VALUE, \
	._maxValue_us = CONFIG_LMTZ_SERVO2_MAX_VALUE, \
	.minInput = CONFIG_LMTZ_SERVO2_MIN_INPUT,\
	.maxInput = CONFIG_LMTZ_SERVO2_MAX_INPUT,\
	.pin = CONFIG_LMTZ_SERVO2_PIN, \
}
#endif

int servo_init(servo_t* ptr);
int servo_destroy(servo_t* self);
int servo_set(servo_t*, int v);

