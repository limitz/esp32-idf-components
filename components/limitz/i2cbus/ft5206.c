#include "ft5206.h"
#include "esp_system.h"
#include "esp_log.h"

#if CONFIG_LMTZ_I2C_FT5206_EN
ft5206_driver_t FT5206 = 
{
	.i2c = {
		.bus = 1,
		.addr = FT5206_ADDR,
	},
	.threshold = FT5206_THRESHOLD_TOUCH,
};
#endif

int ft5206_init(ft5206_driver_t* self)
{
	int err;

	ESP_ERROR_CHECK(err = i2c_read8(&self->i2c, FT5206_VENDOR_ID, &self->id.vendor));
	if (ESP_OK != err) return err;

	ESP_ERROR_CHECK(err = i2c_read8(&self->i2c, FT5206_CHIP_ID, &self->id.chip));
	if (ESP_OK != err) return err;

	ESP_ERROR_CHECK(err = i2c_write8(&self->i2c, FT5206_THRESHOLD, self->threshold));
	if (ESP_OK != err) return err;

	return ESP_OK;
}

int ft5206_update(ft5206_driver_t* self)
{

	int err;
	
	ESP_ERROR_CHECK(err = i2c_read8(&self->i2c, FT5206_NUM_TOUCHES, &self->num_touches));
	if (ESP_OK != err) return err;

	if (self->num_touches == 0) return ESP_OK;
	if (self->num_touches  > FT5206_MAX_TOUCHES) 
	{
		ESP_LOGE(__func__, "TOUCHES = %d", self->num_touches);
		return ESP_OK;
	}

	ESP_ERROR_CHECK(err = i2c_read(&self->i2c, FT5206_TOUCHES, self->touches, self->num_touches * 6));
	if (ESP_OK != err) return err;

	//fix msb
	for (int i=0; i<self->num_touches; i++)
	{
		ft5206_touch_t* t = &self->touches[i];
		t->id = (t->y >> 4) & 0x0F;
		t->x = 0xFFF & ((t->x >> 8) | (t->x << 8));
		t->y = 0xFFF & ((t->y >> 8) | (t->y << 8));
	}

	return ESP_OK;
}

int ft5206_sleep(ft5206_driver_t* self)
{
	return i2c_write8(&self->i2c, FT5206_POWER, FT5206_SLEEP_IN);
}

int ft5206_monitor(ft5206_driver_t* self)
{
	return i2c_write8(&self->i2c, FT5206_POWER, FT5206_MONITOR);
}

int ft5206_deinit(ft5206_driver_t* self)
{
	ft5206_sleep(self);
	return ESP_OK;
}
