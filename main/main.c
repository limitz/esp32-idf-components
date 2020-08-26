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
#include "apa102.h"
#include "i2cbus.h"
#include "ht16k33.h"
#include "gui.h"

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
	apa102_t ledstrip;
} local = {
	.steering = SERVO1,
	.throttle = SERVO2,
	.joystick = JOYSTICK,
	.ledstrip = APA102
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

enum 
{
	HEADLIGHT_L_ = 0x0000000F,
	HEADLIGHT_R  = 0x000000F0,
	HEADLIGHTS   = 0x000000FF,
	INDICATOR_L1 = 0x00800100,
	INDICATOR_L2 = 0x00400200,
	INDICATOR_L3 = 0x00200400,
	INDICATOR_L4 = 0x00100800,
	INDICATOR_R1 = 0x00081000,
	INDICATOR_R2 = 0x00042000,
	INDICATOR_R3 = 0x00024000,
	INDICATOR_R4 = 0x00018000,
	INDICATOR_L  = 0x00F00F00,//INDICATOR_L1 | INDICATOR_L2 | INDICATOR_L3 | INDICATOR_L4,
	INDICATOR_R  = 0x000FF000,//INDICATOR_R1 | INDICATOR_R2 | INDICATOR_R3 | INDICATOR_R4,
	INDICATORS   = 0x00FFFF00,//INDICATOR_L  | INDICATOR_R,
	BRAKE_L      = 0x0F000000,
	BRAKE_R      = 0xF0000000,
	BRAKES       = 0xFF000000,
};

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	
	ESP_ERROR_CHECK(i2c_init());
	
	ht16k33_t* seg = &g_ht16k33[0x8];

	seg[0].content = HT16K33_CONTENT_DEGREES;
	seg[0].intval = 120;

	seg[5].content = HT16K33_CONTENT_STRING;
	strcpy(seg[5].stringval, "KAT");

	seg[1].content = HT16K33_CONTENT_INT;
	seg[1].intval = -33;

	seg[2].content = HT16K33_CONTENT_INT;
	seg[2].intval = 1337;

	seg[6].content = HT16K33_CONTENT_INT;
	seg[6].intval = 666;
	
	seg[7].content = HT16K33_CONTENT_STRING;
	strcpy(seg[7].stringval, "LMTZ");
	ESP_ERROR_CHECK(ht16k33_update(&seg[0]));
	ESP_ERROR_CHECK(ht16k33_update(&seg[1]));
	ESP_ERROR_CHECK(ht16k33_update(&seg[5]));
	ESP_ERROR_CHECK(ht16k33_update(&seg[2]));
	ESP_ERROR_CHECK(ht16k33_update(&seg[6]));
	ESP_ERROR_CHECK(ht16k33_update(&seg[7]));


	servo_init(&local.steering);
	servo_init(&local.throttle);
	
	adc_init(&local.joystick.x);
	adc_init(&local.joystick.y);

	//apa102_init(&local.ledstrip);
	local.ledstrip.count = 0;

	local.radio.callbacks.accept = handle_accept;
	local.radio.callbacks.receive = handle_receive;
	
	radio_init(&local.radio);

	int indicator_toggle = 0;
	int flash_toggle = 0;

	gui_init();
	
	static lv_style_t arc_fg, arc_bg;
	
	lv_style_t* s = &arc_fg;
	lv_style_init(s);
	lv_style_set_border_side(s, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);

	lv_style_set_bg_opa(s, LV_STATE_DEFAULT, LV_OPA_0);
	lv_style_set_line_color(s, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF,0x22,0x00));
	lv_style_set_line_width(s, LV_STATE_DEFAULT, 8);
	lv_style_set_line_rounded(s,LV_STATE_DEFAULT, true);
	
	s = &arc_bg;
	lv_style_init(s);
	lv_style_set_border_side(s, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
	lv_style_set_bg_opa(s, LV_STATE_DEFAULT, LV_OPA_0);
	lv_style_set_line_color(s, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33,0x00,0x00));
	lv_style_set_line_width(s, LV_STATE_DEFAULT, 8);
	lv_style_set_line_rounded(s,LV_STATE_DEFAULT, true);


	lv_obj_t* arc = lv_arc_create(gui_root(), NULL);
	lv_obj_add_style(arc, LV_ARC_PART_BG, &arc_bg);
	lv_obj_add_style(arc, LV_ARC_PART_INDIC, &arc_fg);
	lv_obj_set_size(arc, 120,120);
	lv_arc_set_bg_angles(arc,0, 360);
	lv_arc_set_angles(arc, 0, 230);
	lv_obj_align(arc, NULL, LV_ALIGN_CENTER, 0, 0);


	gui_start();


	for (uint8_t c = 0; ++c; c %= 3)
	{
		adc_update(&local.joystick.x);
		adc_update(&local.joystick.y);

		int steering = local.steering.input;
		int throttle = local.throttle.input;//';((local.joystick.x.result-1880) / 13) * 10;

		if (1 == c)
		{
			if (++flash_toggle >= 3) flash_toggle = 0;
			for (uint8_t i=0; i<local.ledstrip.count; i++)
			{
				
				uint32_t sect = 1<<i;
				apa102_color_t* led = local.ledstrip.leds + i;
				if (sect & BRAKES) 
				{
					if (throttle < -500 && (sect & 0x7E000000) && (flash_toggle < 2)) *led = APA102_COLOR(0x00, 0x00, 0x00, 0x00);
					else if (throttle < 0) *led = APA102_COLOR(0xFF, 0x00, 0x00, 0xFF);
					else *led = APA102_COLOR(0xFF, 0x00, 0x00, 0x0F);
				}

				if (sect & HEADLIGHTS)
				{
					if (throttle > 200)
					*led = APA102_COLOR(0xFF, 0xFF, 0xFF, 0xFF);
					else {
					*led = APA102_COLOR(0xFF, 0xFF, 0xFF, 0x44);
					}

				}

				if (sect & INDICATORS)
				{
					if (steering > 150)
					{
						if (indicator_toggle < 0) indicator_toggle = 0;
	
						if ((indicator_toggle > 2 && (sect & INDICATOR_R1)) 
						||  (indicator_toggle > 3 && (sect & INDICATOR_R2))
						||  (indicator_toggle > 4 && (sect & INDICATOR_R3)) 
						||  (indicator_toggle > 5 && (sect & INDICATOR_R4)))
				 		{
							*led = APA102_COLOR(0xFF, 0x33, 0x00, 0xFF);
						}
						else 
						{
							*led = APA102_COLOR(0x00, 0x00, 0x00, 0x00); 
						}
					}
					else if (steering < -150)
					{
						if (indicator_toggle > 0) indicator_toggle = 0;
						if ((indicator_toggle < -2 && (sect & INDICATOR_L1)) 
						||  (indicator_toggle < -3 && (sect & INDICATOR_L2)) 
						||  (indicator_toggle < -4 && (sect & INDICATOR_L3)) 
						||  (indicator_toggle < -5 && (sect & INDICATOR_L4)))
				 		{
							*led = APA102_COLOR(0xFF, 0x33, 0x00, 0xFF);
						}
						else 
						{
							*led = APA102_COLOR(0x00, 0x00, 0x00, 0x00); 
						}
					}
					else 
					{
						*led = APA102_COLOR(0x00, 0x00, 0x00, 0x00);
					}
				}
			}
			if (steering > 50 && ++indicator_toggle >= 8) indicator_toggle = 0;
			if (steering < 50 && --indicator_toggle <= -8) indicator_toggle = 0;

			//apa102_update(&local.ledstrip);
			local.packet.flag |= RADIO_PACKET_FLAG_BROADCAST;
			local.packet.type = RADIO_PACKET_TYPE_IDENTITY;
			local.packet.identity = local.radio.identity;
			radio_send(&local.radio, &local.packet);
		}

		if (local.packet.flag & RADIO_PACKET_FLAG_READY)
		{
			int send_steering = ((local.joystick.y.result-1880) / 13) * 10;
			int send_throttle = ((local.joystick.x.result-1880) / 13) * 10;

			local.packet.flag &= ~RADIO_PACKET_FLAG_BROADCAST;
			local.packet.type = RADIO_PACKET_TYPE_DATA;
			local.packet.payload.steering = send_steering;
			local.packet.payload.throttle = send_throttle;
			radio_send(&local.radio, &local.packet);
		}
		vTaskDelay(2);
	}
}
