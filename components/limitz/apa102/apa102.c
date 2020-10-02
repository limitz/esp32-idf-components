#include <apa102.h>

#define TAG "APA102"


static int apa102_init()
{
	size_t size = (CONFIG_LMTZ_APA102_NUM_LEDS + 2) * sizeof(apa102_color_t);
	
	APA102.txbuffer = heap_caps_malloc(size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
	memset(APA102.txbuffer, 0, size);
	APA102.txbuffer[0] = 0x00000000;

	for (int i=0; i<CONFIG_LMTZ_APA102_NUM_LEDS; i++)
	{
		APA102.txbuffer[1+i] = 0xE0000000;
	}
	APA102.txbuffer[1+CONFIG_LMTZ_APA102_NUM_LEDS] = 0x00000000;

	spi_bus_initialize(APA102.spi_host, &APA102.bus_config, APA102.dma_channel);
	spi_bus_add_device(APA102.spi_host, &APA102.dev_config, &APA102.device);

	return ESP_OK;
}

static int apa102_update() //, apa102_refresh_cb cb, void* context)
{
	if (!APA102.transaction.tx_buffer)
	{
		APA102.transaction.tx_buffer = APA102.txbuffer;
		//cb(APA102, APA102.txbuffer+1, CONFIG_LMTZ_APA102_NUM_LEDS, context);
	}

	spi_device_queue_trans(APA102.device, &APA102.transaction, portMAX_DELAY);

	APA102.phase += 1;
	//cb(APA102, APA102.txbuffer+1, CONFIG_LMTZ_APA102_NUM_LEDS, context);

	spi_transaction_t* t;
	spi_device_get_trans_result(APA102.device, &t, portMAX_DELAY);
	return ESP_OK;
}

static int apa102_deinit()
{
	free(APA102.txbuffer);
	return ESP_OK;
}

apa102_t APA102 = { 
	.phase = 0, 
	.device = 0, 
	.count = CONFIG_LMTZ_APA102_NUM_LEDS, 
	.spi_host = CONFIG_LMTZ_APA102_SPI_HOST, 
	.dma_channel = CONFIG_LMTZ_APA102_DMA_CHANNEL, 
	.transaction = { 
		.length = CONFIG_LMTZ_APA102_MAX_TRANSFER, 
	}, 
	.bus_config = { 
		.miso_io_num = -1, 
		.mosi_io_num = CONFIG_LMTZ_APA102_PIN_MOSI, 
		.sclk_io_num = CONFIG_LMTZ_APA102_PIN_SCLK, 
		.quadwp_io_num = -1, 
		.quadhd_io_num = -1, 
		.max_transfer_sz = CONFIG_LMTZ_APA102_MAX_TRANSFER, 
	}, 
	.dev_config = { 
		.clock_speed_hz = CONFIG_LMTZ_APA102_CLOCK_SPEED, 
		.mode = 3, 
		.spics_io_num = -1,
		.queue_size = 1, 
	},

	.init = apa102_init,
	.update = apa102_update,
	.deinit = apa102_deinit,
};

