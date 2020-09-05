#pragma once 
#include "i2cbus.h"

// based on https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/src/MPR121.h

#define FT5206_THRESHOLD_TOUCH CONFIG_LMTZ_I2C_FT5206_THRESHOLD_TOUCH

#define FT5206_ADDR 0x38 ///< default I2C address
#define FT520a_VENDOR_ID 0x11
#define FT520b_VENDOR_ID 0xCD
#define CST026_VENDOR_ID 0x26
#define FT5X0X_VENDOR_ID 0x56

#define FT5206_MONITOR  (0x01)
#define FT5206_SLEEP_IN (0x03)

#define FT6206_CHIP_ID  0x06
#define FT6236_CHIP_ID  0x36
#define FT6236U_CHIP_ID 0x64
#define FT5206U_CHIP_ID 0x64

#define FT5206_MAX_TOUCHES 2
enum {
	FT5206_NUM_TOUCHES = 0x02,
	FT5206_TOUCHES = 0x03,
	FT5206_VENDOR_ID = 0xA8,
	FT5206_CHIP_ID = 0xA3,
	FT5206_THRESHOLD = 0x80,
	FT5206_POWER = 0x87,
};

typedef struct
{
	uint16_t x;
	uint16_t y;
	uint16_t id;
} ft5206_touch_t;

typedef struct
{
	i2cdev_t i2c;
	
	uint8_t num_touches;
	ft5206_touch_t touches[FT5206_MAX_TOUCHES];

	uint16_t threshold;
	uint8_t  data;

	struct 
	{
		uint8_t vendor;
		uint8_t chip;
	} id;
} ft5206_driver_t;


int ft5206_init(ft5206_driver_t* self);
int ft5206_update(ft5206_driver_t* self);
int ft5206_sleep(ft5206_driver_t* self);
int ft5206_monitor(ft5206_driver_t* self);
int ft5206_deinit(ft5206_driver_t* self);

extern ft5206_driver_t FT5206;

