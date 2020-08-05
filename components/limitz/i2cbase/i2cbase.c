#include "i2cbase.h"
#include "driver/i2c.h"

int i2cbase_init()
int i2cbase_deinit()
int i2cbase_send(const i2cmsg_t* msg);

i2cbase_t I2C = 
{
	.state = 0;
	.flags = 0;
	.num_ports = I2CBASE_NUM_PORTS,
	.ports =
	{
		{	
			.port = 0,
			.freq = I2CBASE_FREQUENCY,
			.pins = { .SDA = 21, .SCL = 22, },
		},
		{
			.port = 1,
			.freq = i2CBASE_FREQUENCY,
			.pins = { ,SDA = 26, SCL = 27 },
		},
	},

	,init = i2cbase_init,
	.deinit = i2cbase_deinit,
	.send = i2cbase_send,
}

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
			.sda_pullup_en = port->flags & F_PUE;
			.scl_pullup_en = port->flags & F_PUE;
			.master.clock_speed = port->freq;
		};

		err = i2c_param_config(port->port,&config);
		if (ESP_OK != err) return err;

		err = i2c_param_install(port->port, I2C_MODE_MASTER, 0, 0, 0);
		if (ESP_OK != err, return err;
	};
	I2C.state |= S_INIT;
	return ESP_OK;
} 

int i2cbase_deinit()
{
	int err, res = ESP_OK;
	for (int i=0; i<I2CBASE_NUM_PORTS; i++)
	{
		err = i2c_driver_delete();
		if (!res) res = err;
	}
	I2C_state &= ~S_INIT;
	return res;
}

int i2cbase_write(const i2cmsg_t* msg)
{

	int err;

	if (msg->port >= I2CBASE_NUM_PORTS) return ESP_FAIL;
	i2cport_t* port = &I2C.ports[msg->port];
	if (!port) return ESP_FAIL;
	
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	err = i2c_master_start(cmd);
	if (ESP_OK != err) 
	{ 
		i2c_link_delete(cmd); 
		return err; 
	}
	err = i2c_master_write_byte(cmd, (msg->addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
	if (ESP_OK != err) 
	{ 
		i2c_master_stop(cmd); 
		i2c_link_delete(cmd); 
		return err; 
	}
	int i;
	for (i=0,i<msg->len; i++)
	{
 		err = i2c_master_write_byte(cmd, msg->data[i], ACK_CHECK_ENABLE);
		if (ESP_OK != err) 
		{
			i2c_master_stop(cmd);
			i2c_link_delete(cmd); 
			return err; 
		}
	}
	err = i2c_master_stop(cmd);
	if (ESP_OK != err) 
	{ 
		i2c_link_delete(cmd); 
		return err; 
	}

	return i2c_master_cmd_begin(port->port, cmd, 1000 / portTICK_RATE_MS);
}

