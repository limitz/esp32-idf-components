#pragma once

#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_log.h"

#define ACK_CHECK_ENABLE  0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DISABLE 0x0           /*!< I2C master will not check ack from slave *//
#define ACK_VALUE         0x0                 /*!< I2C ack value */
#define NACK_VALUE        0x1                /*!< I2C nack value */

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
	int (*write)(const i2cmessage_t* message);
} i2cbase_t;

extern i2cbase_t I2C;
