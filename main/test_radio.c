#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
enum
{
	LIGHT_OFF    = 0x00,
	LIGHT_HEAD   = 0x01,
	LIGHT_BEAM   = 0x02,
	LIGHT_TAIL   = 0x04,
	LIGHT_BRAKE  = 0x08,
	LIGHT_IND_L  = 0x10,
	LIGHT_IND_R  = 0x20,
	LIGHT_FLASH  = 0x40,
	LIGHT_CUSTOM = 0x80,
};

enum
{
	ROLE_DISCOVERING,
	ROLE_RECEIVER,
	ROLE_TRANSMITTER,
};

static const char* role_names[] = {
	"DISCOVERING",
	"RECEIVER",
	"TRANSMITTER"
};

typedef struct 
{
	uint16_t lights;
	uint16_t throttle;
	uint16_t braking;
	int16_t steering;
} payload_t;

#define RADIO_PACKET_PAYLOAD_TYPE payload_t
#include <radio.h>

#define GPIO_RECEIVER_SELECT_PIN 23

static int role = ROLE_DISCOVERING;
static macaddr_t peer;

int is_receiver()
{
	gpio_config_t config = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = 1 << GPIO_RECEIVER_SELECT_PIN,
		.pull_up_en = 1
	};

	gpio_config(&config);
	return gpio_get_level(GPIO_RECEIVER_SELECT_PIN);
}

int on_accept(const radio_identity_t* identity)
{
	peer = identity->addr;
	role = is_receiver() ? ROLE_RECEIVER : ROLE_TRANSMITTER;

	ESP_LOGW(__func__, "I am now a %s - Accepting peer %s", role_names[role], identity->name);
	return RADIO_ACCEPT;
}

int on_receive(const radio_packet_t* packet)
{
	if (role == ROLE_RECEIVER)
	{
		ESP_LOGI(__func__, "RECV: L:%04X T:%5d B:%5d S:%d", 
				packet->payload.lights, 
				packet->payload.throttle,
				packet->payload.braking,
				packet->payload.steering
		);
	}
	return RADIO_OK;
}


void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );

	static radio_packet_t packet = { 0 };
	static radio_t radio = 
	{ 
		.callbacks = {
			.accept = on_accept, 
			.receive = on_receive,
		}
	};

	radio_init(&radio);

	for (int i = 0; 1; i++)
	{
		if (0 == (i & 0x0F))
		{
			packet.flag = RADIO_PACKET_FLAG_BROADCAST;
			packet.type = RADIO_PACKET_TYPE_IDENTITY;
			packet.identity = radio.identity;
			radio_send(&radio, &packet);
		}

		if (role == ROLE_TRANSMITTER)
		{
			ESP_LOGI(__func__, "SENDING PACKET");

			packet.flag = RADIO_PACKET_FLAG_READY;
			packet.type = RADIO_PACKET_TYPE_DATA;
			packet.addr = peer;
			packet.payload.lights = LIGHT_HEAD | LIGHT_TAIL;
			packet.payload.throttle = 0;
			packet.payload.braking = 1000;
			packet.payload.steering = 0;
			radio_send(&radio, &packet);
		}
		vTaskDelay(2);
	}

}



