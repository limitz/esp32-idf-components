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

#define APA102 { \
	.phase = 0, \
	.device = 0, \
	.count = CONFIG_LMTZ_APA102_NUM_LEDS, \
	.spi_host = CONFIG_LMTZ_APA102_SPI_HOST, \
	.dma_channel = CONFIG_LMTZ_APA102_DMA_CHANNEL, \
	.transaction = { \
		.length = CONFIG_LMTZ_APA102_MAX_TRANSFER, \
	}, \
	.bus_config = { \
		.miso_io_num = -1, \
		.mosi_io_num = CONFIG_LMTZ_APA102_PIN_MOSI, \
		.sclk_io_num = CONFIG_LMTZ_APA102_PIN_SCLK, \
		.quadwp_io_num = -1, \
		.quadhd_io_num = -1, \
		.max_transfer_sz = CONFIG_LMTZ_APA102_MAX_TRANSFER, \
	}, \
	.dev_config = { \
		.clock_speed_hz = CONFIG_LMTZ_APA102_CLOCK_SPEED, \
		.mode = 3, \
		.spics_io_num = -1,\
		.queue_size = 1, \
	}\
}

typedef struct
{
	union 
	{
		apa102_color_t* txbuffer;
		apa102_color_t* leds;
	};
		//[CONFIG_LMTZ_APA102_MAX_TRANSFER/sizeof(apa102_color_t)];
	
	uint16_t phase;
	int spi_host;
	int dma_channel;
	int count;
	spi_bus_config_t bus_config;
	spi_device_interface_config_t dev_config;
	spi_transaction_t transaction;
	spi_device_handle_t device;
} apa102_t;

typedef void (*apa102_refresh_cb)(apa102_t* sender, apa102_color_t* color, size_t len, void* context);

esp_err_t apa102_init(apa102_t* self);
esp_err_t apa102_update(apa102_t* self); //, apa102_refresh_cb cb, void* context);
esp_err_t apa102_destroy(apa102_t* self);
