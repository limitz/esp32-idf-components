#pragma once

#include <stdint.h>

typedef struct
{
	struct
	{
		const void *data;
		int width;
		int height
		int pos;
	} in;

	struct
	{
		void* data;
		int width;
		int height;
	} out;

} jpg_dev_t;

int jpg_decode(jpg_dev_t* dev);
