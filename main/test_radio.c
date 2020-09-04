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
	ACTION_HONK = 0x01
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
	uint8_t lights;
	uint8_t actions;
	int16_t throttle;
	
	int16_t steering;
} payload_t;

#define RADIO_PACKET_PAYLOAD_TYPE payload_t
#include <radio.h>
#include <servo.h>
#include <adc.h>

#define GPIO_RECEIVER_SELECT_PIN 23

#define GPIO_INDICATOR_LEFT 	21
#define GPIO_INDICATOR_RIGHT 	22
#define GPIO_RED                17
#define GPIO_GREEN		15
#define GPIO_BLUE		13
#define GPIO_STICK		27

static radio_packet_t packet = { 0 };
static radio_packet_t discovery = {0};

QueueHandle_t gpio_event_queue;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	uint32_t gpio = (uint32_t)arg;
	xQueueSendFromISR(gpio_event_queue, &gpio, NULL);
}

static void gpio_task(void* arg)
{
	uint32_t gpio;
	for (;;)
	{
		if (xQueueReceive(gpio_event_queue, &gpio, portMAX_DELAY))
		{
			int level = gpio_get_level(gpio);
			ESP_LOGI(__func__, "GPIO %d %d", gpio,level);
		
			if (gpio == GPIO_STICK)
			{
				if (!level)
				{
					ESP_LOGE(__func__, "HONK!");
					packet.payload.actions = ACTION_HONK;
				}
				else
				{
					packet.payload.actions = 0;
				}
			}
		}
	}
}

static macaddr_t peer;
static int role = ROLE_DISCOVERING;

// set these in menuconfig
static servo_t servo_steering = SERVO1;
static servo_t servo_throttle = SERVO2;
static joystick_t joystick = JOYSTICK;

// Get the level of pin (23) 
// if high:  TRANSMITTER
// if low:   RECEIVER
int is_receiver()
{
	gpio_config_t config = {
		.intr_type = GPIO_PIN_INTR_DISABLE,
		.mode = GPIO_MODE_INPUT,
		.pin_bit_mask = 1 << GPIO_RECEIVER_SELECT_PIN,
		.pull_up_en = 1
	};

	gpio_config(&config);
	return gpio_get_level(GPIO_RECEIVER_SELECT_PIN) ? 0 : 1;
}

// A peer was discovered on the broadcast channel
// if RECEIVER: initialize servos
// if TRANSMITTER: initialize joystick
int on_accept(const radio_identity_t* identity)
{
	peer = identity->addr;
	role = is_receiver() ? ROLE_RECEIVER : ROLE_TRANSMITTER;

	ESP_LOGW(__func__, "ACCEPTING %s", identity->name);
	switch (role)
	{
	case ROLE_RECEIVER:
		servo_init(&servo_steering);
		servo_init(&servo_throttle);
		break;

	case ROLE_TRANSMITTER:
		adc_init(&joystick.x);
		adc_init(&joystick.y);
		
		//buttons
		gpio_config_t config = {
			.intr_type = GPIO_PIN_INTR_ANYEDGE,
			.mode = GPIO_MODE_INPUT,
			.pin_bit_mask = (1 << GPIO_INDICATOR_LEFT) |
					(1 << GPIO_INDICATOR_RIGHT) |
					(1 << GPIO_RED) |
					(1 << GPIO_BLUE) |
					(1 << GPIO_GREEN) |
					(1 << GPIO_STICK),
			.pull_up_en = 1,
		};

		gpio_config(&config);
		gpio_event_queue = xQueueCreate(12, sizeof(uint32_t));
		xTaskCreate(gpio_task, "GPIO", 3072, NULL, 10, NULL);
		gpio_install_isr_service(0);
		gpio_isr_handler_add(GPIO_INDICATOR_LEFT, gpio_isr_handler, (void*) GPIO_INDICATOR_LEFT);
		gpio_isr_handler_add(GPIO_INDICATOR_RIGHT, gpio_isr_handler, (void*) GPIO_INDICATOR_RIGHT);
		gpio_isr_handler_add(GPIO_RED,   gpio_isr_handler, (void*) GPIO_RED);
		gpio_isr_handler_add(GPIO_BLUE,  gpio_isr_handler, (void*) GPIO_BLUE);
		gpio_isr_handler_add(GPIO_GREEN, gpio_isr_handler, (void*) GPIO_GREEN);
		gpio_isr_handler_add(GPIO_STICK, gpio_isr_handler, (void*) GPIO_STICK);

		
		break;
	}

	ESP_LOGW(__func__, "I am now a %s - Accepting peer %s", role_names[role], identity->name);
	return RADIO_ACCEPT;
}

int on_receive(const radio_packet_t* packet)
{
	static int timeout = 0;

	if (role == ROLE_RECEIVER)
	{
		//packet->payload.lights, 
		servo_set(&servo_steering, packet->payload.steering);
		servo_set(&servo_throttle, packet->payload.throttle);
	
		if (packet->payload.actions & ACTION_HONK) 
		{
			//honk();
		}
	}
	return RADIO_OK;
}


void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
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
		if (0 == (i & 0xFF))
		{
			packet.flag = RADIO_PACKET_FLAG_BROADCAST;
			packet.type = RADIO_PACKET_TYPE_IDENTITY;
			packet.identity = radio.identity;
			//packet.payload.actions = 0;
			radio_send(&radio, &packet);
		}

		if (role == ROLE_TRANSMITTER)
		{
			//ESP_LOGI(__func__, "SENDING PACKET");

			adc_update(&joystick.x);
			adc_update(&joystick.y);

			int send_steering = ((1660 - joystick.x.result) / 17) * 10;
			int send_throttle = ((joystick.y.result - 1660) / 17) * 10;

			//ESP_LOGI(__func__, "T: %6d S: %6d", send_throttle, send_steering);
			packet.flag = RADIO_PACKET_FLAG_READY;
			packet.type = RADIO_PACKET_TYPE_DATA;
			packet.addr = peer;
			packet.payload.lights = LIGHT_HEAD | LIGHT_TAIL;
			packet.payload.throttle = send_throttle;
			packet.payload.steering = send_steering;
			radio_send(&radio, &packet);
			//packet.payload.actions = 0;
		}
		vTaskDelay(2);
	}

}



