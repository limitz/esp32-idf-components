#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/i2c.h>
#include <esp_log.h>

#if CONFIG_LMTZ_I2C_BUS0_EN && CONFIG_LMTZ_I2C_BUS1_EN
#define I2C_BUS_COUNT 2
#elif CONFIG_LMTZ_I2C_BUS0_EN || CONFIG_LMTZ_I2C_BUS1_EN
#define I2C_BUS_COUNT 1
#else
#define I2C_BUS_COUNT 0
#endif

#if I2C_BUS0_EN

#define I2C_BUS0_FREQ CONFIG_LMTZ_I2C_BUS0_FREQ
#define I2C_BUS0_SDA  CONFIG_LMTZ_I2C_BUS0_SDA
#define I2C_BUS0_SCL  CONFIG_LMTZ_I2C_BUS0_SCL

#endif

#if I2C_BUS1_EN
#define I2C_BUS1_FREQ CONFIG_LMTZ_I2C_BUS1_FREQ
#define I2C_BUS1_SDA  CONFIG_LMTZ_I2C_BUS1_SDA
#define I2C_BUS1_SCL  CONFIG_LMTZ_I2C_BUS1_SCL
#endif

typedef struct
{
	int index;
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
	} pin;

} i2cbus_t;

typedef struct 
{
	uint8_t bus;
	union
	{
		uint8_t addr;
		uint8_t address;
	};
} i2cdevice_t, i2cdev_t;

typedef struct
{
	i2cbus_t bus[I2C_BUS_COUNT];
} i2c_t;

int i2c_init();
int i2c_deinit();

int i2c_send_cmd(i2cdev_t* dev, uint8_t cmd);
int i2c_send_data(i2cdev_t* dev, int reg, const void* data, int len);
int i2c_send_u8(i2cdev_t* dev, uint8_t reg, uint8_t value);
int i2c_send_u16(i2cdev_t* dev, uint8_t reg, uint16_t value);
int i2c_send_u32(i2cdev_t* dev, uint8_t reg, uint32_t value);

int i2c_recv_data(i2cdev_t* dev, int reg, void* data, int len);
int i2c_recv_u8(i2cdev_t* dev, uint8_t reg, uint8_t* value);
int i2c_recv_u16(i2cdev_t* dev, uint8_t reg, uint16_t* value);
int i2c_recv_u32(i2cdev_t* dev, uint8_t reg, uint32_t* value);


#define i2c_read i2c_recv_data
#define i2c_read8 i2c_recv_u8
#define i2c_read16 i2c_recv_u16
#define i2c_read32 i2c_recv_u32

#define i2c_write i2c_send_data
#define i2c_write0 i2c_send_cmd
#define i2c_write8 i2c_send_u8
#define i2c_write16 i2c_send_u16
#define i2c_write32 i2c_send_u32


