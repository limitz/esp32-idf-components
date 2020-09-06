#include "rc.h"
#include "servo.h"


static int handle_accept(radio_t* radio, const radio_identity_t* identity);
static int handle_receive(radio_t* radio, const radio_packet_t* packet);

rc_driver_t RC = 
{	
	.role = RC_ROLE_INVALID,
	.radio = 
	{
		.callbacks = {
			.on_accept = handle_accept,
			.on_receive = handle_receive,
		},
	},
		
	.packets = 
	{
		.discover = 
		{
			.flag = RADIO_PACKET_FLAG_BROADCAST,
			.type = RADIO_PACKET_TYPE_IDENTITY,
		},
		.controls = 
		{
			.type = RADIO_PACKET_TYPE_DATA,
			.payload.lights = RC_LIGHT_HEAD | RC_LIGHT_TAIL,
			.payload.steering = 0,
			.payload.throttle = 0,
		},
	},
	.vehicle.servos =
	{
		.steering = SERVO1,
		.throttle = SERVO2,
	},
	.discover_interval = 500,
	.packet_interval = 20,
};

static int handle_accept(radio_t* radio, const radio_identity_t* identity)
{
	ESP_LOGE(__func__, "ACCEPT");

	switch (RC.role)
	{
	case RC_ROLE_VEHICLE:
		ESP_LOGW(__func__, "ACCEPT %s (CONTROLLER)", identity->name);
		RC.packets.discover.flag |= RADIO_PACKET_FLAG_READY;
		return RADIO_ACCEPT;

	case RC_ROLE_CONTROLLER:
		ESP_LOGW(__func__, "ACCEPT %s (VEHICLE)", identity->name);
		RC.packets.discover.flag |= RADIO_PACKET_FLAG_READY;
		RC.packets.controls.flag |= RADIO_PACKET_FLAG_READY;
		RC.packets.controls.addr = identity->addr;
		return RADIO_ACCEPT;
	}
	return RADIO_ACCEPT;
}

static int handle_receive(radio_t* radio, const radio_packet_t* packet)
{
	switch (packet->type)
	{
	case RADIO_PACKET_TYPE_DATA:
		servo_set(&RC.vehicle.servos.steering, packet->payload.steering);
		servo_set(&RC.vehicle.servos.throttle, packet->payload.throttle);
		break;
	}
	return RADIO_OK;
}

static void rc_task(void* param)
{
	int discover = 0;

	for (;;)
	{
		if (0 == discover--)
		{
			//ESP_LOGI(__func__, "SENDING DISCOVERY PACKET");
			discover = RC.discover_interval / RC.packet_interval;
			RC.packets.discover.identity = RC.radio.identity;
			radio_send(&RC.radio, &RC.packets.discover);
		}
		switch (RC.role)
		{
			case RC_ROLE_CONTROLLER:
				if (RC.packets.controls.flag & RADIO_PACKET_FLAG_READY)
				{
					radio_send(&RC.radio, &RC.packets.controls);
				}
				break;
		};
		vTaskDelay(RC.packet_interval / portTICK_PERIOD_MS);
	}
}

int rc_init()
{
	switch (RC.role)
	{
	case RC_ROLE_INVALID:
		ESP_LOGE(__func__, "Set RC.radio.identity.role to one of RC_ROLE_*");
		return ESP_FAIL;

	case RC_ROLE_VEHICLE:
		servo_init(&RC.vehicle.servos.steering);
		servo_init(&RC.vehicle.servos.throttle);
		break;
	}
	ESP_ERROR_CHECK( radio_init(&RC.radio) );
	xTaskCreate(rc_task, "rc task", 3072, &RC, 4, NULL);

	return ESP_OK;
}

int rc_deinit()
{
	return radio_deinit(&RC.radio);
	// TODO
}

