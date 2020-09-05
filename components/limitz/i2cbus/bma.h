#pragma once

#include "bma423/bma423.h"
#include "i2cbus.h"

enum
{
	BMA_DIR_UNKNOWN,
	BMA_DIR_FORWARD = 0x01,
	BMA_DIR_BACK    = 0x02,
	BMA_DIR_LEFT	= 0x04,
	BMA_DIR_RIGHT   = 0x08,
	BMA_DIR_UP      = 0x10,
	BMA_DIR_DOWN    = 0x20,
};

enum
{
	BMA_FEATURE_NONE         = 0x00,
	BMA_FEATURE_ACCEL        = 0x01,
	BMA_FEATURE_STEPCOUNTER  = 0x02,
	BMA_FEATURE_STEPDETECT   = 0x04,
	BMA_FEATURE_ACTIVITY = 0x08,
	BMA_FEATURE_TILT     = 0x10,
	BMA_FEATURE_WAKEUP   = 0x20,
	BMA_FEATURE_STEP     = 0x40,
//	BMA_FEATURE_ANY_NOMOTION = 0x80,
};

enum
{
	BMA_ACTIVITY_UNKNOWN = 0x00,
	BMA_ACTIVITY_STATIONARY = BMA423_USER_STATIONARY,
	BMA_ACTIVITY_WALKING = BMA423_USER_WALKING,
	BMA_ACTIVITY_RUNNING = BMA423_USER_RUNNING,
	BMA_ACTIVITY_INVALID = BMA423_STATE_INVALID,
};

typedef struct _bma_driver_t
{
	i2cdevice_t i2c;

	int direction;
	int temperature;
	int activity;

	int features_enabled;
	int irq_status;

	struct bma4_dev bma;
	struct bma4_accel accel_data;
	struct bma4_accel_config accel_config;
	struct bma4_int_pin_config pin_config;
} bma_driver_t;



int bma_init(bma_driver_t* self);
int bma_reset(bma_driver_t* self);
int bma_update(bma_driver_t* self);
int bma_enable(bma_driver_t* self, int features);
int bma_disable(bma_driver_t* self, int features);
int bma_deinit(bma_driver_t* self);

extern bma_driver_t BMA423;
