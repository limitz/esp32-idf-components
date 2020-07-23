#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_log.h>
#include <driver/spi_master.h>

#if (CONFIG_LMTZ_MAX6921_SPI_HSPI)
#define CONFIG_LMTZ_MAX6921_SPI_HOST HSPI_HOST
#elif (CONIG_LMTZ_PA102_SPI_VSPI)
#define CONFIG_LMTZ_MAX6921_SPI_HOST VSPI_HOST
#endif

#define CONFIG_LMTZ_MAX6921_MAX_TRANSFER 32 

#define MAX6921_DEFAULT { \
	.phase = 0, \
	.device = 0, \
	.spi_host = CONFIG_LMTZ_MAX6921_SPI_HOST, \
	.dma_channel = CONFIG_LMTZ_MAX6921_DMA_CHANNEL, \
	,pin_blank = CONFIG_LMTZ_MAX6921_PIN_BLANK, \
	.pin_load = CONFIG_LMTZ_MAX6921_PIN_LOAD, \
	.transaction = { \
		.length = CONFIG_LMTZ_MAX6921_MAX_TRANSFER, \
		.flags = SPI_TRANS_USE_TXDATA, \
		.tx_data = 0, \
	}, \
	.bus_config = { \
		.miso_io_num = -1, \
		.mosi_io_num = CONFIG_LMTZ_MAX6921_PIN_MOSI, \
		.sclk_io_num = CONFIG_LMTZ_MAX6921_PIN_SCLK, \
		.quadwp_io_num = -1, \
		.quadhd_io_num = -1, \
		.max_transfer_sz = CONFIG_LMTZ_MAX6921_MAX_TRANSFER, \
	}, \
	.dev_config = { \
		.clock_speed_hz = CONFIG_LMTZ_MAX6921_CLOCK_SPEED, \
		.mode = 3, \
		.spics_io_num = -1,\
		.queue_size = 1, \
	}\
}

typedef struct
{
	uint32_t state;
	uint16_t phase;
	int spi_host;
	int dma_channel;
	int pin_load;
	int pin_blank;
	spi_bus_config_t bus_config;
	spi_device_interface_config_t dev_config;
	spi_transaction_t transaction;
	spi_device_handle_t device;
} max6921_t;

typedef void (*max6921_refresh_cb)(max6921_t* sender, uint8_t* state, size_t len, void* context);

esp_err_t max6921_init(max6921_t* self);
esp_err_t max6921_update(max6921_t* self, max6921_refresh_cb cb, void* context);
esp_err_t max6921_destroy(max6921_t* self);
