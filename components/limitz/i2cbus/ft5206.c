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
};
#endif
#define SELF FT5206
#define I2C  &FT5206.i2c

int ft5206_init()
{
	ft5206_read_info();
	
	int err = i2c_write8(I2C, FT5206_REG_DEVICE_MODE, FT5206_DEVICE_MODE_NORMAL);
	if (ESP_OK != err) return err;
	
	err = ft5206_set_power_mode(FT5206_POWER_MODE_ACTIVE);
	if (ESP_OK != err) return err;

	ft5206_read_touches();
	ft5206_read_thresholds();
	ft5206_read_power();

	return ESP_OK;
}

int ft5206_read_touches()
{
	int err = i2c_read(I2C, FT5206_REG_TOUCH_BEGIN, &SELF.touch, 2);
	if (ESP_OK != err) return err;
	if (SELF.touch.count == 0 || 
	    SELF.touch.count > FT5206_MAX_TOUCHES) return ESP_OK;

	err = i2c_read(I2C, FT5206_REG_TOUCHPOINTS_BEGIN, SELF.touch.points, 
		       FT5206_REG_TOUCH_END - FT5206_REG_TOUCHPOINTS_BEGIN);
	if (ESP_OK != err) return err;

	//swizzle
	for (int i=0; i<SELF.touch.count; i++)
	{
		ft5206_touch_t* p = &SELF.touch.points[i];
		p->id = (0xF0 & p->y) >> 4;
		p->flag = (0xF0 & p->x) >> 4;
		p->x = 0xFFF & ((p->x >> 8) | (p->x << 8));
		p->y = 0xFFF & ((p->y >> 8) | (p->y << 8));
	}

	return ESP_OK;
}

int ft5206_read_info()
{
	int err = i2c_read(I2C, FT5206_REG_INFO_BEGIN, &SELF.info, 
	                   FT5206_REG_INFO_END - FT5206_REG_INFO_BEGIN);
	if (ESP_OK != err) return err;
	
	return ESP_OK;
}

int ft5206_read_thresholds()
{
	int err;

	err = i2c_read( I2C, FT5206_REG_THRESHOLDS_BEGIN, &SELF.thresholds, 
			FT5206_REG_THRESHOLDS_END - FT5206_REG_THRESHOLDS_BEGIN);	
	if (ESP_OK != err) return err;

	err = i2c_read8(I2C, FT5206_REG_ID_G_B_AREA_TH, &SELF.thresholds.big_area);
	if (ESP_OK != err) return err;

	return ESP_OK;
}

int ft5206_read_power()
{
	int err;
	err = i2c_read( I2C, FT5206_REG_POWER_BEGIN, &SELF.power,
			FT5206_REG_POWER_END - FT5206_REG_POWER_BEGIN);
	if (ESP_OK != err) return err;

	return ESP_OK;
}

int ft5206_set_power_mode(int mode)
{
	int err;

	// TODO sanity check
	err = i2c_write8(I2C, FT5206_REG_ID_G_PMODE, mode);
	if (ESP_OK != err) return err;

	SELF.info.power_mode = mode;
	return ESP_OK;
}

int ft5206_write_thresholds()
{
	int err;

	err = i2c_write(I2C, FT5206_REG_THRESHOLDS_BEGIN, &SELF.thresholds,
			FT5206_REG_THRESHOLDS_END - FT5206_REG_THRESHOLDS_BEGIN);
	if (ESP_OK != err) return err;

	return ESP_OK;
}
int ft5206_write_power()
{
	int err;

	err = i2c_write(I2C, FT5206_REG_POWER_BEGIN, &SELF.power,
			FT5206_REG_POWER_END - FT5206_REG_POWER_BEGIN);
	if (ESP_OK != err) return err;

	return ESP_OK;
}

int ft5206_deinit()
{
	ft5206_set_power_mode(FT5206_POWER_MODE_HIBERNATE);
	return ESP_OK;
}
