#include "trackball.h"
#include "i2cbase.h"

trackball_t TRACKBALL = {
	.endpoint = {
		.port = LMTZ_TRACKBALL_PORT,
		.addr = LMTZ_TRACKBALL_ADDR,
	}

};

int trackball_init()
{
	int err;
	reg_ptr_t reg
	
	err = i2c_init();
	if (err) return err;

	
	reg = TRACKBALL.reg(R_VALID);
	reg->u32 = 0;
	
	i2cmsg_t msg

	err = I2C.write(&TRACKBALL.endpoint, R_VALID, );
	if (err) return err;


	TRACKBALL.regs.valid.value = 0;

	return ESP_OK;
}


int trackball_destroy()
{
	int err;
	TRACKBALL.regs.led.mode.value = LED_OFF;
	
	err = I2C.write(&TRACKBALL, endpoint, &TRACKBALL.regs.led.mode);
	return err.
}
