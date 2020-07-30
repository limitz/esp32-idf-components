#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "unique_id.h"
#include "adc.h"
#include "lipo.h"
#include "servo.h"

typedef struct
{
	int16_t steering;
	int16_t throttle;

} radio_packet_payload_t;
#define RADIO_PACKET_PAYLOAD_TYPE radio_packet_payload_t

#include "radio.h"
static struct
{
	joystick_t joystick;
	servo_t steering, throttle;
	radio_t radio; radio_packet_t packet;

} local = {
	.steering = SERVO1,
	.throttle = SERVO2,
	.joystick = JOYSTICK,
};



int handle_accept(const radio_identity_t* identity)
{
	ESP_LOGW(__func__, "Accepting %s", identity->name);
	local.packet.addr = identity->addr;
	local.packet.flag |= RADIO_PACKET_FLAG_READY;
	return RADIO_ACCEPT;
}

int handle_receive(const radio_packet_t* packet)
{
	ESP_LOGW(__func__, "[TYPE:%03X FLAGS:%02X SEQ:%04X CRC:%04X] => steering: [ %4d ]  throttle: [ %4d ]", 
			packet->type, packet->flag, packet->seq, packet->crc, 
			packet->payload.steering, packet->payload.throttle);
	servo_set(&local.steering, packet->payload.steering);
	servo_set(&local.throttle, packet->payload.throttle);
	return RADIO_OK;
}


void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	
	servo_init(&local.steering);
	servo_init(&local.throttle);
	
	adc_init(&local.joystick.x);
	adc_init(&local.joystick.y);
	
	local.radio.callbacks.accept = handle_accept;
	local.radio.callbacks.receive = handle_receive;
	
	radio_init(&local.radio);

	for (uint8_t c = 0; ++c; c %= 3)
	{
		adc_update(&local.joystick.x);
		adc_update(&local.joystick.y);

		if (1 == c)
		{
			local.packet.flag |= RADIO_PACKET_FLAG_BROADCAST;
			local.packet.type = RADIO_PACKET_TYPE_IDENTITY;
			local.packet.identity = local.radio.identity;
			radio_send(&local.radio, &local.packet);
		}

		if (local.packet.flag & RADIO_PACKET_FLAG_READY)
		{
			local.packet.flag &= ~RADIO_PACKET_FLAG_BROADCAST;
			local.packet.type = RADIO_PACKET_TYPE_DATA;
			local.packet.payload.steering = ((local.joystick.y.result-1880) / 13) * 10;
			local.packet.payload.throttle = ((local.joystick.x.result-1880) / 13) * 10;
			radio_send(&local.radio, &local.packet);
		}
		vTaskDelay(3);
	}
}
