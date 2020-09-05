#include "bma.h"

#include <esp_system.h>
#include <esp_log.h>

// loosely based on https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/src/drive/bma423/bma.cpp

static void bma_delay(unsigned int ms)
{
	vTaskDelay(ms / portTICK_PERIOD_MS);
}

static uint16_t bma_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
	return i2c_send_data(&BMA423.i2c, reg, data, len);
}

static uint16_t bma_read(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t len)
{
	return i2c_recv_data(&BMA423.i2c, reg, data, len);
}

bma_driver_t BMA423 = {
	.i2c = {
		.bus = 1,
		.addr = BMA4_I2C_ADDR_SECONDARY,
	},
	.bma = {
		.dev_addr  = BMA4_I2C_ADDR_SECONDARY,
		.interface = BMA4_I2C_INTERFACE,
		.bus_read = bma_read,
		.bus_write = bma_write,
		.delay = bma_delay,
		.read_write_len = 8,
		.resolution = 12,
		.feature_len = BMA423_FEATURE_SIZE,
	},

	.pin_config = {
		.edge_ctrl = BMA4_LEVEL_TRIGGER,
		.lvl = BMA4_ACTIVE_HIGH,
		.od = BMA4_PUSH_PULL,
		.output_en = BMA4_OUTPUT_ENABLE,
		.input_en = BMA4_INPUT_DISABLE,
	},

	.accel_config = {
		.odr = BMA4_OUTPUT_DATA_RATE_100HZ,
		.range = BMA4_ACCEL_RANGE_2G,
		.bandwidth = BMA4_ACCEL_NORMAL_AVG4,
		.perf_mode = BMA4_CONTINUOUS_MODE,
	}
};

int bma_init(bma_driver_t* self)
{
	int err;

	ESP_ERROR_CHECK(err = bma_reset(self));
	if (ESP_OK != err) return err;

	bma_delay(20);
	
	ESP_ERROR_CHECK(err = bma423_init(&self->bma));
	if (BMA4_OK != err) return err;

	ESP_ERROR_CHECK(err = bma423_write_config_file(&self->bma));
	if (BMA4_OK != err) return err;

	ESP_ERROR_CHECK(err = bma4_set_int_pin_config(&self->pin_config, BMA4_INTR1_MAP, &self->bma));
	if (BMA4_OK != err) return err;

	return ESP_OK;
}

int bma_reset(bma_driver_t* self)
{
	i2c_send_u8(&self->i2c, 0x7E, 0xB6);
	return ESP_OK;
}

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

static int bma_update_direction(bma_driver_t* self)
{
	int err;

	ESP_ERROR_CHECK(err = bma4_read_accel_xyz(&self->accel_data, &self->bma));
	if (BMA4_OK != err) return err;
	
	int x,y,z;
	int absX = ABS(x = self->accel_data.x);
	int absY = ABS(y = self->accel_data.y);
	int absZ = ABS(z = self->accel_data.z);

	if (absZ > MAX(absX, absY)) self->direction = z < 0 ? BMA_DIR_DOWN : BMA_DIR_UP;
	if (absY > MAX(absX, absZ)) self->direction = y < 0 ? BMA_DIR_FORWARD : BMA_DIR_BACK;
	if (absX > MAX(absY, absZ)) self->direction = x < 0 ? BMA_DIR_LEFT : BMA_DIR_RIGHT;

	return ESP_OK;
}

static int bma_update_temperature(bma_driver_t* self)
{
	int err;
	int32_t data = 0;

	ESP_ERROR_CHECK(err = bma4_get_temperature(&data, BMA4_DEG, &self->bma));
	if (BMA4_OK != err) return err;

	if ((data-23) / BMA4_SCALE_TEMP == 0x80) return ESP_FAIL;

	self->temperature = (data * 100) / BMA4_SCALE_TEMP;
	return ESP_OK;
}
/*
static void bma_handle_interrupt()
{
	int irq;
	bma423_read_int_status(&irq, &self->bma);

	if (irq & BMA423_WAKEUP_INT)
		ESP_LOGW(__func__, "WAKEUP EVENT");
	if (irq & BMA423_TILT_INT)
		ESP_LOGW(__func__, "TILT EVENT");
}
*/
int bma_update(bma_driver_t* self)
{
	int err;
	ESP_ERROR_CHECK(err = bma_update_direction(self));
	if (ESP_OK != err) return err;

	ESP_ERROR_CHECK(err = bma_update_temperature(self));
	if (ESP_OK != err) return err;

	return ESP_OK;
}

static int bma_enable_feature(bma_driver_t* self, int features, int en)
{
	int err;
	if (features & BMA_FEATURE_ACCEL)
	{

		ESP_ERROR_CHECK(err = bma4_set_accel_enable(en, &self->bma));
		if (BMA4_OK != err) return err;

		if (en)
		{
			ESP_ERROR_CHECK(err = bma4_set_accel_config(&self->accel_config, &self->bma));
			if (BMA4_OK != err) return err;
		}
	}

	if (features & BMA_FEATURE_STEPCOUNTER)
	{
		ESP_ERROR_CHECK(err = bma423_step_detector_enable(en, &self->bma));
		if (BMA4_OK != err) return err;
		
		ESP_ERROR_CHECK(err = bma423_feature_enable(BMA423_STEP_CNTR, en, &self->bma));
		if (BMA4_OK != err) return err;
	
		ESP_ERROR_CHECK(err = bma423_map_interrupt(
			BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT, en, &self->bma));
		if (BMA4_OK != err) return err;
	
		if (en)
		{
			ESP_ERROR_CHECK(err = bma423_step_counter_set_watermark(100, &self->bma));
			if (BMA4_OK != err) return err;
		}
	}
	if (features & BMA_FEATURE_ACTIVITY)
	{
		ESP_ERROR_CHECK(err = bma423_feature_enable(BMA423_ACTIVITY, en, &self->bma));
		if (BMA4_OK != err) return err;

		ESP_ERROR_CHECK(err = bma423_map_interrupt(
			BMA4_INTR1_MAP, BMA423_ACTIVITY_INT, en, &self->bma));
		if (BMA4_OK != err) return err;
	}
	if (features & BMA_FEATURE_WAKEUP)
	{
		ESP_ERROR_CHECK(err = bma423_feature_enable(BMA423_WAKEUP, en, &self->bma));
		if (BMA4_OK != err) return err;

		ESP_ERROR_CHECK(err = bma423_map_interrupt(
			BMA4_INTR1_MAP, BMA423_WAKEUP_INT, en, &self->bma));
		if (BMA4_OK != err) return err;
	}
	if (features & BMA_FEATURE_TILT)
	{
		ESP_ERROR_CHECK(err = bma423_feature_enable(BMA423_TILT, en, &self->bma));
		if (BMA4_OK != err) return err;
		
		ESP_ERROR_CHECK(err = bma423_map_interrupt(
			BMA4_INTR1_MAP, BMA423_TILT_INT, en, &self->bma));
		if (BMA4_OK != err) return err;
	}
	return ESP_OK;
}

int bma_enable(bma_driver_t* self, int features)
{
	self->features_enabled |= features;
	return bma_enable_feature(self, features, BMA4_ENABLE);
}

int bma_disable(bma_driver_t* self, int features)
{
	self->features_enabled &= ~features;
	return bma_enable_feature(self, features, BMA4_DISABLE);
}

int bma_deinit(bma_driver_t* self)
{
	return ESP_OK;
}



