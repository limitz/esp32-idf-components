#pragma once 
#include "i2cbus.h"

// based on https://github.com/Xinyuan-LilyGO/TTGO_TWatch_Library/blob/master/src/MPR121.h

#define FT5206_THRESHOLD_TOUCH CONFIG_LMTZ_I2C_FT5206_THRESHOLD_TOUCH

#define FT5206_ADDR 0x38 ///< default I2C address
#define FT520a_VENDOR_ID 0x11
#define FT520b_VENDOR_ID 0xCD
#define CST026_VENDOR_ID 0x26
#define FT5X0X_VENDOR_ID 0x56


#define FT6206_CHIP_ID  0x06
#define FT6236_CHIP_ID  0x36
#define FT6236U_CHIP_ID 0x64
#define FT5206U_CHIP_ID 0x64

#define FT5206_MAX_TOUCHES 5

// DEVICE MODE, INFO AND TEST ARE RESERVED
enum {
	FT5206_DEVICE_MODE_NORMAL = 0x00,
	FT5206_DEVICE_MODE_INFO   = 0x01,
	FT5206_DEVICE_MODE_TEST   = 0x04,
};

// GESTURES
enum {
	FT5206_GESTURE_NONE       = 0x00,
	FT5206_GESTURE_MOVE       = 0x10,
	FT5206_GESTURE_MOVE_UP    = 0x10,
	FT5206_GESTURE_MOVE_LEFT  = 0x14,
	FT5206_GESTURE_MOVE_DOWN  = 0x18,
	FT5206_GESTURE_MOVE_RIGHT = 0x1C,
	FT5206_GESTURE_ZOOM       = 0x40,
	FT5206_GESTURE_ZOOM_IN    = 0x48,
	FT5206_GESTURE_ZOOM_OUT   = 0x49,
};

enum
{
	FT5206_TOUCH_DOWN         = 0x00,
	FT5206_TOUCH_UP           = 0x01,
	FT5206_TOUCH_CONTACT      = 0x02,
};

enum 
{
	FT5206_CTRL_DEFAULT = 0x00,
	FT5206_CTRL_AUTO_JUMP = 0x01,
};

enum
{
	FT5206_AUTO_CALIB_DISABLE = 0xFF,
	FT5206_AUTO_CALIB_ENABLE = 0x00,
};

enum
{
	FT5206_INT_MODE_POLLING = 0x00,
	FT5206_INT_MODE_TRIGGER = 0x01,
};

enum
{
	FT5206_POWER_MODE_ACTIVE = 0x00,
	FT5206_POWER_MODE_MONITOR = 0x01,
	FT5206_POWER_MODE_HIBERNATE = 0x03,
};

enum
{
	FT5206_RUN_MODE_CONFIGURE = 0x00,
	FT5206_RUN_MODE_WORK = 0x01,
	FT5206_RUN_MODE_CALIBRATION = 0x02,
	FT5206_RUN_MODE_FACTORY = 0x03,
	FT5206_RUN_MODE_AUTO_CALIBRATION = 0x04,
};

enum
{
	FT5206_ERR_OK = 0x00,
	FT5206_ERR_INCONSISTENT_WRITE = 0x03,
	FT5206_ERR_START_FAIL = 0x05,
	FT5206_ERR_CALIB_INPUT_MATCH = 0x01A,
};

// REGISTER MAP
enum {
	FT5206_REG_DEVICE_MODE    = 0x00,

	FT5206_REG_TOUCH_BEGIN    = 0x01,
	FT5206_REG_GEST_ID        = 0x01,
	FT5206_REG_TD_STATUS      = 0x02,

	#define FT5206_REG_TOUCHPOINT_DECL(n) \
	FT5206_REG_TOUCH ## n ## _XH = ((FT5206_REG_TOUCHPOINTS_BEGIN) + (6*n) + 0), \
	FT5206_REG_TOUCH ## n ## _XL = ((FT5206_REG_TOUCHPOINTS_BEGIN) + (6*n) + 1), \
	FT5206_REG_TOUCH ## n ## _YH = ((FT5206_REG_TOUCHPOINTS_BEGIN) + (6*n) + 2), \
	FT5206_REG_TOUCH ## n ## _YL = ((FT5206_REG_TOUCHPOINTS_BEGIN) + (6*n) + 3)

	FT5206_REG_TOUCHPOINTS_BEGIN  = 0x03,
	FT5206_REG_TOUCHPOINT_DECL(0),
	FT5206_REG_TOUCHPOINT_DECL(1),
	FT5206_REG_TOUCHPOINT_DECL(2),
	FT5206_REG_TOUCHPOINT_DECL(3),
	FT5206_REG_TOUCHPOINT_DECL(4),
	FT5206_REG_TOUCH_END,
	
	FT5206_REG_THRESHOLDS_BEGIN = 0x80,
	FT5206_REG_ID_G_THGROUP     = 0x80, // default 280/4 = 70
	FT5206_REG_ID_G_THPEAK      = 0x81,
	FT5206_REG_ID_G_THCAL       = 0x82,
	FT5206_REG_ID_G_THWATER     = 0x83,
	FT5206_REG_ID_G_THTEMP      = 0x84,
	FT5206_REG_ID_G_DIFF        = 0x85,
	FT5206_REG_THRESHOLDS_END,

	FT5206_REG_POWER_BEGIN             = 0x86,
	FT5206_REG_ID_G_CTRL               = 0x86,
	FT5206_REG_ID_G_TIME_ENTER_MONITOR = 0x87,
	FT5206_REG_ID_G_PERIOD_ACTIVE      = 0x88,
	FT5206_REG_ID_G_PERIOD_MONITOR     = 0x89,
	FT5206_REG_POWER_END,

	FT5206_REG_INFO_BEGIN              = 0xA0,
	FT5206_REG_ID_G_AUTO_CLB_MODE      = 0xA0,
	FT5206_REG_ID_G_LIB_VERSION_H      = 0xA1,
	FT5206_REG_ID_G_LIB_VERSION_L      = 0xA2,
	FT5206_REG_ID_G_CHIP_ID            = 0xA3,
	FT5206_REG_ID_G_MODE               = 0xA4,
	FT5206_REG_ID_G_PMODE              = 0xA5,
	FT5206_REG_ID_G_FIRMID             = 0xA6,
	FT5206_REG_ID_G_STATE              = 0xA7,
	FT5206_REG_ID_G_CTPM_ID            = 0xA8,
	FT5206_REG_ID_G_ERR                = 0xA9,
	FT5206_REG_ID_G_CLB                = 0xAA,
	FT5206_REG_INFO_END,

	FT5206_REG_ID_G_B_AREA_TH          = 0xAE,
	FT5206_REG_LOG_MSG_CNT             = 0xFE,
	FT5206_REG_LOG_CUR_CHA             = 0xFF,
};

#pragma pack(push, 1)
typedef struct
{
	uint16_t x;
	uint16_t y;
	union 
	{
		uint16_t _reserved;
		struct 
		{
			uint8_t flag;
			uint8_t id;
		};
	};

} ft5206_touch_t;
#pragma pack(pop)

typedef struct
{
	i2cdev_t i2c;


	uint8_t device_mode;
	
	struct {
		uint8_t gesture;
		uint8_t count;
		ft5206_touch_t points[FT5206_MAX_TOUCHES];
	} touch;

	struct {
		uint8_t group;
		uint8_t peak;
		uint8_t cal;
		uint8_t water;
		uint8_t temp;
		uint8_t big_area;
	} thresholds;

	struct
	{
		uint8_t run_mode;
		uint8_t delay_monitor;
		uint8_t period_active;
		uint8_t period_monitor;
	} power;

	struct
	{
		uint8_t auto_calibration;
		uint8_t version_major;
		uint8_t version_minor;
		uint8_t chip_vendor_id;
		uint8_t int_mode;
		uint8_t power_mode;
		uint8_t firmware_id;
		uint8_t run_mode;
		uint8_t ctpm_vendor_id;
		uint8_t error_code;
		uint8_t calibrate_during_test;
	} info;


} ft5206_driver_t;

int ft5206_init();

int ft5206_read_touches();
int ft5206_read_info();
int ft5206_read_thresholds();
int ft5206_read_power();

int ft5206_set_power_mode(int mode);

int ft5206_write_thresholds();
int ft5206_write_power();

int ft5206_deinit();

extern ft5206_driver_t FT5206;

