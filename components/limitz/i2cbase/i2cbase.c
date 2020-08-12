#include "i2cbase.h"
#include "driver/i2c.h"

int i2cbase_init();
int i2cbase_deinit();
int i2cbase_write_message(const i2cmsg_t* msg);
int i2cbase_write(const i2cendpoint_t* endpoint, const i2creg_t* reg);

i2cbase_t I2C = 
{
	.state = 0,
	.flags = 0,
	.ports =
	{
#if CONFIG_LMTZ_I2CPORT0_EN
		{	
			.port = 0,
			.freq = I2C0_FREQ,
			.pins = { .SDA = I2C0_SDA, .SCL = I2C0_SCL },
		},
#endif
#if CONFIG_LMTZ_I2CPORT0_EN

		{
			.port = 1,
			.freq = I2C1_FREQ,
			.pins = { .SDA = I2C1_SDA, .SCL = I2C1_SCL },
		},
	},
#endif
	.init = i2cbase_init,
	.deinit = i2cbase_deinit,
	.write_message = i2cbase_write_message,
	.write = i2cbase_write,
};

int i2cbase_init()
{
	int i;
	int err;
	i2cport_t* port;

	for (i=0; i<I2CBASE_NUM_PORTS; i++)
	{
		port = &I2C.ports[i];

		i2c_config_t config = 
		{
			.mode = I2C_MODE_MASTER,
			.sda_io_num = port->pins.SDA,
			.scl_io_num = port->pins.SCL,
			.sda_pullup_en = port->flags & F_PUE,
			.scl_pullup_en = port->flags & F_PUE,
			.master.clk_speed = port->freq,
		};

		err = i2c_param_config(port->port,&config);
		if (ESP_OK != err) return err;

		err = i2c_driver_install(port->port, I2C_MODE_MASTER, 0, 0, 0);
		if (ESP_OK != err) return err;
	};
	I2C.state |= S_INIT;
	return ESP_OK;
} 

int i2cbase_deinit()
{
	int err, res = ESP_OK;
	for (int i=0; i<I2CBASE_NUM_PORTS; i++)
	{
		i2cport_t* port = &I2C.ports[i];
		err = i2c_driver_delete(port->port);
		if (!res) res = err;
	}
	I2C.state &= ~S_INIT;
	return res;
}

int i2cbase_write(const i2cendpoint_t* endpoint, const i2creg_t* reg)
{
	// TODO check if mode contains write bit
	i2cmsg_t msg = {
		.port = endpoint->port | 0x80,
		.addr = endpoint->addr,
		.len = reg->mode.size & 0xF,
	};
	memcpy(msg.data, reg->value, msg.len);
	
	return i2cbase_write_message(&msg);
}

int i2cbase_write_message(const i2cmsg_t* msg)
{

	int err;

	if (msg->port >= I2CBASE_NUM_PORTS) return ESP_FAIL;
	i2cport_t* port = &I2C.ports[msg->port];
	if (!port) return ESP_FAIL;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	err = i2c_master_start(cmd);
	if (ESP_OK != err) 
	{ 
		i2c_cmd_link_delete(cmd); 
		return err; 
	}
	err = i2c_master_write_byte(cmd, (msg->addr << 1) | I2C_MASTER_WRITE, 0x01);
	if (ESP_OK != err) 
	{ 
		i2c_master_stop(cmd); 
		i2c_cmd_link_delete(cmd); 
		return err; 
	}
	int i;
	for (i=0; i<msg->len; i++)
	{
 		err = i2c_master_write_byte(cmd, msg->data[i], 0x01);
		if (ESP_OK != err) 
		{
			i2c_master_stop(cmd);
			i2c_cmd_link_delete(cmd); 
			return err; 
		}
	}
	err = i2c_master_stop(cmd);
	if (ESP_OK != err) 
	{ 
		i2c_cmd_link_delete(cmd); 
		return err; 
	}

	return i2c_master_cmd_begin(port->port, cmd, 1000 / portTICK_RATE_MS);
}

