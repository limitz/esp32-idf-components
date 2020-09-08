#pragma once

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <radio.h>
#include <servo.h>

enum
{
	RC_LIGHT_OFF    = 0x00,
	RC_LIGHT_HEAD   = 0x01,
	RC_LIGHT_BEAM   = 0x02,
	RC_LIGHT_TAIL   = 0x04,
	RC_LIGHT_BRAKE  = 0x08,
	RC_LIGHT_IND_L  = 0x10,
	RC_LIGHT_IND_R  = 0x20,
	RC_LIGHT_FLASH  = 0x40,
	RC_LIGHT_CUSTOM = 0x80,
};

enum 
{
	RC_ACTION_NONE = 0x00,
	RC_ACTION_HONK = 0x01,
};

enum
{
	RC_ROLE_INVALID,
	RC_ROLE_VEHICLE = 0x20,
	RC_ROLE_CONTROLLER = 0x30,
};

enum
{
	RC_PACKET_TYPE_CONTROL = 0x10,
};

typedef struct 
{
	uint8_t lights;
	uint8_t actions;
	int16_t throttle;	
	int16_t steering;

} rc_payload_t;


typedef struct
{
	rc_payload_t payload;
	radio_identity_t peer;

	int role;
	int packet_interval;
	int discover_interval;

	union
	{
		struct
		{
			struct 
			{
				servo_t steering;
				servo_t throttle;
	
			} servos;

		} vehicle;
	};
} rc_driver_t;


extern rc_driver_t RC;

int rc_init();
int rc_deinit();
