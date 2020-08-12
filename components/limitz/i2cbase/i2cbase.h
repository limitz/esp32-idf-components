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
	F_PUE =  1,
	I2CFLAG_PULLUP_ENABLED = 1,

} i2cflags_t;


typedef union
{
	uint8_t write;
	uint8_t read;
	uint8_t len;
	uint8_t length;
	uint8_t size;
} i2cmode_t;

typedef struct { uint8_t addr; i2cmode_t mode; uint8_t value[0]; } 
i2cregister_t, i2creg_t;

typedef struct { uint8_t addr; i2cmode_t mode; uint8_t value; } 
i2cregister_uint8_t, i2creg_u8_t;

typedef struct { uint8_t addr; i2cmode_t mode; uint16_t value; } 
i2cregister_uint16_t, i2creg_u16_t;

typedef struct { uint8_t addr; i2cmode_t mode; uint32_t value; }
i2cregister_uint32_t, i2creg_u32_t;


typedef struct
{
	uint8_t port;
	i2cflags_t flags;

	union
	{
		uint8_t addr;
		uint8_t address;
	};

	union {
		uint8_t size;
		uint8_t len;
		uint8_t length;
	};
	
	uint8_t data[0];
} i2cmessage_t;
typedef i2cmessage_t i2cmsg_t;


typedef struct
{
	uint8_t port;

	union {
		uint8_t address;
		uint8_t addr;
	};

	int (*write)(const void* buffer, int len);
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
	int (*read) (i2cendpoint_t* src, i2creg_t* reg);
	int (*write)(const i2cendpoint_t* dst, const i2creg_t* reg);
	int (*write_message)(const i2cmessage_t* message);
} i2cbase_t;

extern i2cbase_t I2C;
