#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <driver/spi_master.h>

typedef uint32_t apa102_color_t;

#define APA102_COLOR(r,g,b,lum) ((r<<24) | (g<<16) | (b<<8) | ((lum)) | 0xE0) 

#if (CONFIG_LMTZ_APA102_SPI_HSPI)
#define CONFIG_LMTZ_APA102_SPI_HOST HSPI_HOST
#elif (CONFIG_LMTZ_APA102_SPI_VSPI)
#define CONFIG_LMTZ_APA102_SPI_HOST VSPI_HOST
#endif

#define CONFIG_LMTZ_APA102_MAX_TRANSFER (8*((2+CONFIG_LMTZ_APA102_NUM_LEDS)*sizeof(apa102_color_t)))

typedef struct
{
	union 
	{
		apa102_color_t* txbuffer;
		apa102_color_t* leds;
	};
		//[CONFIG_LMTZ_APA102_MAX_TRANSFER/sizeof(apa102_color_t)];

	int (*init)();
	int (*deinit)();
	int (*update)();

	uint16_t phase;
	int spi_host;
	int dma_channel;
	int count;
	spi_bus_config_t bus_config;
	spi_device_interface_config_t dev_config;
	spi_transaction_t transaction;
	spi_device_handle_t device;
} apa102_t;

//typedef void (*apa102_refresh_cb)(apa102_t* sender, apa102_color_t* color, size_t len, void* context);

extern apa102_t APA102;
