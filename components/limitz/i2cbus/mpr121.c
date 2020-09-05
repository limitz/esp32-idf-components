#include "mpr121.h"
#include "esp_system.h"
#include "esp_log.h"

#if CONFIG_LMTZ_I2C_MPR121_EN
mpr121_driver_t MPR121 = 
{
	.i2c = {
		.bus = 1,
		.addr = MPR121_ADDR,
	},
	.thresholds = {
		{ MPR121_THRESHOLD_TOUCH_0, MPR121_THRESHOLD_RELEASE_0},
		{ MPR121_THRESHOLD_TOUCH_1, MPR121_THRESHOLD_RELEASE_1},
		{ MPR121_THRESHOLD_TOUCH_2, MPR121_THRESHOLD_RELEASE_2},
		{ MPR121_THRESHOLD_TOUCH_3, MPR121_THRESHOLD_RELEASE_3},
		{ MPR121_THRESHOLD_TOUCH_4, MPR121_THRESHOLD_RELEASE_4},
		{ MPR121_THRESHOLD_TOUCH_5, MPR121_THRESHOLD_RELEASE_5},
		{ MPR121_THRESHOLD_TOUCH_6, MPR121_THRESHOLD_RELEASE_6},
		{ MPR121_THRESHOLD_TOUCH_7, MPR121_THRESHOLD_RELEASE_7},
		{ MPR121_THRESHOLD_TOUCH_8, MPR121_THRESHOLD_RELEASE_8},
		{ MPR121_THRESHOLD_TOUCH_9, MPR121_THRESHOLD_RELEASE_9},
		{ MPR121_THRESHOLD_TOUCH_A, MPR121_THRESHOLD_RELEASE_A},
		{ MPR121_THRESHOLD_TOUCH_B, MPR121_THRESHOLD_RELEASE_B},
	},
};
#endif

int mpr121_init(mpr121_driver_t* self)
{
	int err;
	ESP_ERROR_CHECK(err = i2c_send_u8(&self->i2c, MPR121_SOFTRESET, 0x63));
	if (ESP_OK != err) return err;

	vTaskDelay(10);

	i2c_send_u8(&self->i2c, MPR121_ECR, 0x00);

	uint8_t config;
	ESP_ERROR_CHECK(err = i2c_recv_u8(&self->i2c, MPR121_CONFIG2, &config));
	if (ESP_OK != err)  return err;
	if (config != 0x24) 
	{
		ESP_LOGE(__func__, "MPR121_CONFIG2 = %02X", config);
		return ESP_FAIL;
	}

	err |= i2c_send_u8(&self->i2c, MPR121_MHDR, 0x01);
	err |= i2c_send_u8(&self->i2c, MPR121_NHDR, 0x01);
	err |= i2c_send_u8(&self->i2c, MPR121_NCLR, 0x0E);
	err |= i2c_send_u8(&self->i2c, MPR121_FDLR, 0x00);

	err |= i2c_send_u8(&self->i2c, MPR121_MHDF, 0x01);
	err |= i2c_send_u8(&self->i2c, MPR121_NHDF, 0x05);
	err |= i2c_send_u8(&self->i2c, MPR121_NCLF, 0x01);
	err |= i2c_send_u8(&self->i2c, MPR121_FDLF, 0x00);

	err |= i2c_send_u8(&self->i2c, MPR121_NHDT, 0x00);
	err |= i2c_send_u8(&self->i2c, MPR121_NCLT, 0x00);
	err |= i2c_send_u8(&self->i2c, MPR121_FDLT, 0x00);

	err |= i2c_send_u8(&self->i2c, MPR121_DEBOUNCE, 0x00);
	err |= i2c_send_u8(&self->i2c, MPR121_CONFIG1, 0x10);
	err |= i2c_send_u8(&self->i2c, MPR121_CONFIG2, 0x20);
	if (ESP_OK != err) return err;

	/*
	uint8_t ecr = 0x80 + 12;
	ESP_ERROR_CHECK(err = i2c_send_u8(&self->i2c, MPR121_ECR, ecr);
	if (ESP_OK != err) return err;

	ESP_ERROR_CHECK(err = i2c_send_u8(&self->i2c, MPR121_ECR, 0x00);
	if (ESP_OK != err) return err;
	*/
	
	/*for (uint8_t i = 0; i<12; i++)
	{
		i2c_send_u8(&self->i2c, MPR121_TOUCHTH_0 + 2 * i, MPR121_TOUCH_THRESHOLD_DEFAULT);
		i2c_send_u8(&self->i2c, MPR121_RELEASETH_0 + 2 * i, MPR121_RELEASE_THRESHOLD_DEFAULT);
	}
	*/

	/*
	ESP_ERROR_CHECK(err = i2c_send_data(&self->i2c, MPR121_TOUCHTH_0, self->thresholds, 24));
	if (ESP_OK != err) return err;
	*/
	
	i2c_send_u8(&self->i2c, MPR121_AUTOCONFIG0, 0x0B);

    	// correct values for Vdd = 3.3V
   	i2c_send_u8(&self->i2c,MPR121_UPLIMIT, 200);     // ((Vdd - 0.7)/Vdd) * 256
    	i2c_send_u8(&self->i2c,MPR121_TARGETLIMIT, 180); // UPLIMIT * 0.9
    	i2c_send_u8(&self->i2c,MPR121_LOWLIMIT, 130);    // UPLIMIT * 0.65

	ESP_ERROR_CHECK(err = i2c_send_u8(&self->i2c, MPR121_ECR, 0x8C));
	if (ESP_OK != err) return err;

	return ESP_OK;
}

int mpr121_update(mpr121_driver_t* self)
{
	return i2c_recv_u16(&self->i2c, MPR121_TOUCHSTATUS_L, &self->touchpoints);
}

int mpr121_update_filtered(mpr121_driver_t* self)
{
	return i2c_recv_data(&self->i2c, MPR121_FILTDATA_0L, &self->filtered, 24);	
}

int mpr121_update_baseline(mpr121_driver_t* self)
{
	int err;
	uint8_t data[12];
	ESP_ERROR_CHECK(err = i2c_recv_data(&self->i2c, MPR121_BASELINE_0, data, 12));
	if (ESP_OK != err) return err;
	
	for (int i=0; i<12; i++) self->baseline[i] = data[i] << 2;
	return ESP_OK;
}

int mpr121_deinit(mpr121_driver_t* self)
{
	i2c_send_u8(&self->i2c, MPR121_ECR, 0x00);
	return ESP_OK;
}
