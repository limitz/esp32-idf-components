#include <max6921.h>
#define TAG "MAX6921"

esp_err_t max6921_init(max6921_t* self)
{
	spi_bus_initialize(self->spi_host, &self->bus_config, self->dma_channel);
	spi_bus_add_device(self->spi_host, &self->dev_config, &self->device);

	return ESP_OK;
}

esp_err_t max6921_update(max6921_t* self, max6921_refresh_cb cb, void* context)
{
	//spi_device_queue_trans(self->device, &self->transaction, portMAX_DELAY);

	self->phase += 1;
	cb(self, self->transaction.tx_data, 20, context);
	spi_device_polling_transmit(self->device, &self->transaction);

	//spi_transaction_t* t;
	//spi_device_get_trans_result(self->device, &t, portMAX_DELAY);
	return ESP_OK;
}

esp_err_t max6921_destroy(max6921_t* self)
{
	return ESP_OK;
}
