#include <apa102.h>

#define TAG "APA102"

static esp_err_t g_err;

#define LOG_ERROR(x) ESP_LOGE(TAG, "ERROR: " #x " returned 0x%08X (%d)", g_err, g_err)
#define LOG_INFO(x)  ESP_LOGI(TAG, #x)
#define _CHECK(tag, x, onsuccess, onerror) if (ESP_OK != (g_err = (int)(x))) onerror else onsuccess
#define ROE(x) _CHECK(TAG, x, { LOG_INFO(x); }, { LOG_ERROR(x); return g_err; } )
#define XOE(x) _CHECK(TAG, x, { LOG_INFO(x); }, { LOG_ERROR(x); return; } )

#define NOT_IMPLEMENTED { ESP_LOGE(TAG, "%s not implemented", __func__); return SF_NOT_IMPLEMENTED; }


esp_err_t apa102_init(apa102_t* self)
{
	memset(self->txbuffer, 0, sizeof(self->txbuffer));
	self->txbuffer[0] = 0x00000000;
	for (int i=0; i<CONFIG_APA102_NUM_LEDS; i++)
	{
		self->txbuffer[1+i] = 0xFF000011;
	}
	self->txbuffer[1+CONFIG_APA102_NUM_LEDS] = 0xFFFFFFFF;

	spi_bus_initialize(self->spi_host, &self->bus_config, self->dma_channel);
	spi_bus_add_device(self->spi_host, &self->dev_config, &self->device);

	return ESP_OK;
}

esp_err_t apa102_update(apa102_t* self, apa102_refresh_cb cb, void* context)
{
	if (!self->transaction.tx_buffer)
	{
		self->transaction.tx_buffer = self->txbuffer;
		cb(self, self->txbuffer+1, CONFIG_APA102_NUM_LEDS, context);
	}

	spi_device_queue_trans(self->device, &self->transaction, portMAX_DELAY);

	self->phase += 1;
	cb(self, self->txbuffer+1, CONFIG_APA102_NUM_LEDS, context);

	spi_transaction_t* t;
	spi_device_get_trans_result(self->device, &t, portMAX_DELAY);
	return ESP_OK;
}

esp_err_t apa102_destroy(apa102_t* self)
{
	return ESP_OK;
}
