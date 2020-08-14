#pragma once

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_log.h"

#if CONFIG_LMTZ_I2CPORT0_EN
#define I2C0_SDA 	CONFIG_LMTZ_I2CPORT0_SDA
#define I2C0_SCL	CONFIG_LMTZ_I2CPORT0_SCL
#define I2C0_FREQ 	CONFIG_LMTZ_I2CPORT0_FREQUENCY
#endif

#if CONFIG_LMTZ_I2CPORT1_EN
#define I2C1_SDA	CONFIG_LMTZ_I2CPORT1_SDA
#define I2C1_SCL	CONFIG_LMTZ_I2CPORT1_SCL
#define I2C1_FREQ  	CONFIG_LMTZ_I2CPORT1_FREQUENCY
#endif


#define I2CBASE_NUM_PORTS 2


typedef enum
{
	S_INIT = 1,
	I2CSTATE_INITIALIZED = 1,

} i2cstate_t;

typedef enum 
{
	I2CFLAG_NONE,
	F_PUE = 1,		I2CFLAG_PULLUP_ENABLED = 1,
	F_EN  = 2,		I2CFLAG_ENABLED = 2, 		 

} i2cflags_t;



typedef struct
{
	uint8_t port;

	union {
		uint8_t address;
		uint8_t addr;
	};

} i2cendpoint_t;

typedef struct
{
        int port;
	int state;
	i2cflags_t flags;
	union
	{
		uint32_t freq;
		uint32_t frequency;
	};


	struct
	{
		union 
		{
			uint8_t SDA;
			uint8_t sda;
		};

		union
		{
			uint8_t SCL;
			uint8_t scl;
		};
	} pins;

} i2cport_t;


typedef struct
{
	i2cstate_t state;
	i2cflags_t flags;
	i2cport_t ports[I2CBASE_NUM_PORTS];
	int (*init)();
	int (*deinit)();
	int (*read) (i2cendpoint_t* src, uint8_t* reg, void* data, int len);
	int (*write)(const i2cendpoint_t* dst, uint8_t reg, const void* data, int len);


} i2cbase_t;

extern i2cbase_t I2C;
