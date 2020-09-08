#include "rc.h"
#include "servo.h"


static int handle_accept(const radio_packet_t* identity);
static int handle_receive(const radio_packet_t* packet);

rc_driver_t RC = 
{	
	.role = RC_ROLE_INVALID,
	.vehicle.servos =
	{
		.steering = SERVO1,
		.throttle = SERVO2,
	},
	.discover_interval = 2000,
	.packet_interval = 20,
};

static int handle_accept(const radio_packet_t* packet)
{
	ESP_LOGE(__func__, "ACCEPT");
	
	RC.peer = packet->identity;
	RC.peer.connected = 1;
	return RADIO_ACCEPT;
}

static int handle_receive(const radio_packet_t* packet)
{
	switch (packet->type)
	{
		case RADIO_PACKET_TYPE_DATA:
		{	
			rc_payload_t* payload = (rc_payload_t*) packet->payload;
			servo_set(&RC.vehicle.servos.steering, payload->steering);
			servo_set(&RC.vehicle.servos.throttle, payload->throttle);
			break;
		}
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
			discover = RC.discover_interval / RC.packet_interval;
			radio_broadcast(RADIO_PACKET_TYPE_IDENTITY, &RADIO.identity, sizeof(radio_identity_t));
		}
		switch (RC.role)
		{
			case RC_ROLE_CONTROLLER:
				if (RC.peer.connected)
				{
					radio_unicast(
						&RC.peer.addr,
						RADIO_PACKET_TYPE_DATA, 
						&RC.payload, 
						sizeof(rc_payload_t));
				}
				break;
		};
		vTaskDelay(RC.packet_interval / portTICK_PERIOD_MS);
	}
}

int rc_init()
{
	RADIO.callbacks.on_accept = handle_accept;
	RADIO.callbacks.on_receive= handle_receive;
	
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
	ESP_ERROR_CHECK( radio_init());
	xTaskCreate(rc_task, "rc task", 3072, &RC, 4, NULL);

	return ESP_OK;
}

int rc_deinit()
{
	return radio_deinit();

	// TODO
}

