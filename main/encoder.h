#pragma once

typedef struct
{
	int idx;
	int filter;
} encoder_t;

void encoder_init(encoder_t*)
void encoder_destroy(encoder_t*);


