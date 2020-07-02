#pragma once

#include <driver/ledc.h>

typedef struct
{
	ledc_channel_t channel;
	int minValue ;
	int maxValue; 
	int duty ;
	int pin ;
} servo_t;

int servo_init(servo_t* ptr);
int servo_destroy(servo_t* self);
int servo_set(servo_t*, int v);

