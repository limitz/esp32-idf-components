#include "i2cbus.h"

#define CLEANUP _ ## __func__ ##_cleanup

static i2cbus_t s_bus[I2C_BUS_COUNT] = 
{
	{
		.frequency = I2C_BUS0_FREQ,
		.pin.SDA   = I2C_BUS0_SDA,
		.pin.SCL   = I2C_BUS0_SCL,
	},
	{
		.frequency = I2C_BUS1_FREQ,
		.pin.SDA   = I2C_BUS1_SDA,
		.pin.SCL   = I2C_BUS1_SCL,
	}
};

int i2cbus_init(i2cbus_t* bus)
{
	int err = ESP_OK;
	i2c_config_t config = 
	{
		.mode = I2C_MODE_MASTER,
		.sda_io_num = bus->pin.SDA,
		.scl_io_num = bus->pin.SCL,
		.sda_pullup_en = 1,
		.scl_pullup_en = 1,
		.master.clk_speed = bus->freq,
	};

	err = i2c_param_config(bus->index, &config);
	if (ESP_OK != err) return err;

	err = i2c_driver_install(bus->index, I2C_MODE_MASTER, 0, 0, 0);
	if (ESP_OK != err) return err;

	return ESP_OK;
}


int i2c_init()
{
	int index,err;
	for (index = 0; index < I2C_BUS_COUNT; index++)
	{
		s_bus[index].index = index;
		err = i2cbus_init(&s_bus[index]);
		if (ESP_OK != err) return err;
	}
	return ESP_OK;
}

int i2c_deinit()
{
	int index, err, res = ESP_OK;
	for (index = 0; index < I2C_BUS_COUNT; index++)
	{
		err = i2c_driver_delete(index);
		if (ESP_OK != err) res = err;
	}
	return res;
}

int i2c_send_cmd(i2cdev_t* dev, uint8_t cmd)
{
	return i2c_send_data(dev, &cmd, 1);
}

int i2c_send_data(i2cdev_t* dev, const void* data, int len)
{
	int err;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	
	err = i2c_master_start(cmd);
	if (ESP_OK != err) return err; //delete cmd

	err = i2c_master_write_byte(cmd, (dev->addr << 1) | I2C_MASTER_WRITE, 0x01);
	if (ESP_OK != err) return err; // delete and stop

	err = i2c_master_write(cmd, data, len, 0x01);
	if (ESP_OK != err) return err; // delete and stop
	
	/*
	const uint8_t* ptr = (const uint8_t*) data;
	while (len-- > 0)
	{
		ESP_ERROR_CHECK(err = i2c_master_write_byte(cmd, *(ptr++), 0x01));

		if (ESP_OK != err) return err; // delete and stop
	}
	*/
	err = i2c_master_stop(cmd);
	if (ESP_OK != err) return err; // delete

	i2c_master_cmd_begin(dev->bus, cmd, 1000 / portTICK_RATE_MS);
	return err;
}

int i2c_recv(i2cdev_t* dev, uint8_t reg, void* data, int* len)
{
	return ESP_FAIL;
}


