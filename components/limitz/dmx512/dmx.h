#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"

#define DMX_U1_EN CONFIG_LMTZ_DMX_U1_EN

#if DMX_U1_EN
#define DMX_U1_UART CONFIG_LMTZ_DMX_U1_UART
#define DMX_U1_NUM_CHANNELS CONFIG_LMTZ_DMX_U1_NUM_CHANNELS
#define DMX_U1_PIN CONFIG_LMTZ_DMX_U1_PIN

#define DMX_NUM_UNIVERSES 1
#define DMX_NUM_FRAME_BUFFERS
#define DMX_UART_BAUD_RATE 250000
#define DMX_NUM_BUFFERS CONFIG_LMTZ_DMX_NUM_BUFFERS

typedef struct
{
	uint8_t channel[DMX_U1_NUM_CHANNELS];
	uint16_t num_channels;
} dmx_frame_t;

typedef struct
{
	dmx_frame_t* frame;
	uint8_t uart;
	uint8_t pin;
	uint8_t _fbidx;
	dmx_frame_t _fb[DMX_NUM_BUFFERS];

} dmx_universe_t;

// DRIVER 
typedef struct
{
	union 
	{
		dmx_universe_t U;
		dmx_universe_t universe[DMX_NUM_UNIVERSES];
	};
//	TaskHandle_t task;
	QueueHandle_t queue;

} dmx_driver_t;


// API
int dmx_init();
int dmx_deinit();

int dmx_update();
int dmx_universe_update(dmx_universe_t* self);

extern dmx_driver_t DMX;
#endif
