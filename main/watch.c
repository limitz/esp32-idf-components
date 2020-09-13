#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <i2cbus.h>
#include <ft5206.h>
#include <bma.h>
#include <gui.h>
#include <axp202.h>
#include "rc.h"

#include "xploading.c"

#define LDO_BACKLIGHT AXP202_LDO2
#define nullptr NULL

void do_wakeup()
{
	gui_start(true);
	axp202_set_output(LDO_BACKLIGHT, AXP202_ON);
}

void do_sleep()
{
	axp202_set_output(LDO_BACKLIGHT, AXP202_OFF);
	gui_stop(true);
	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(60000000));
	
	ESP_LOGE(__func__, "Sleeping");
	fflush(stdout);
	fflush(stderr);
	esp_light_sleep_start();

	ESP_LOGW(__func__, "WOKEN UP");
	do_wakeup();
}

void app_main()
{
	RC.role = RC_ROLE_CONTROLLER;

	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( i2c_init() );
	ESP_ERROR_CHECK( rc_init() );
	ESP_ERROR_CHECK( ft5206_init() );
	ESP_ERROR_CHECK( axp202_init() );

	ESP_LOGW("FT5206", "VENDOR: %02x CHIP: %02x", 
			FT5206.info.ctpm_vendor_id, 
			FT5206.info.chip_vendor_id);

	ESP_ERROR_CHECK(gui_init());

	

	#define style_set(s, k, v) lv_style_set_ ## k (&s, LV_STATE_DEFAULT, v)
	#define style_init(s) lv_style_init(&s)

	static lv_style_t afg;
	style_init(afg);
	style_set(afg, border_side, LV_BORDER_SIDE_NONE);
	style_set(afg, bg_opa, LV_OPA_0);
	style_set(afg, line_width, 8);
	style_set(afg, line_rounded, true);
	style_set(afg, line_color, LV_COLOR_MAKE(0xFF, 0x22, 0x00));

	static lv_style_t abg;
	style_init(abg);
	style_set(abg, border_width, 0); 
	style_set(abg, border_side, LV_BORDER_SIDE_NONE);
	style_set(abg, bg_opa, LV_OPA_0);

	style_set(abg, line_width, 8);
	style_set(abg, line_rounded, true);
	style_set(abg, line_color, LV_COLOR_MAKE(0x33, 0x00, 0x00));

	static lv_style_t nostyle;
	style_init(abg);
	style_set(abg, border_width, 0); 
	style_set(abg, border_side, LV_BORDER_SIDE_NONE);
	style_set(abg, bg_opa, LV_OPA_0);
#if 0
	lv_obj_t* arc = lv_arc_create(gui_root(), NULL);
	lv_obj_add_style(arc, LV_ARC_PART_BG, &abg);
	lv_obj_add_style(arc, LV_ARC_PART_INDIC, &afg);
	lv_obj_set_size(arc, 120, 120);
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_arc_set_angles(arc, 0, 230);
	lv_obj_align(arc, NULL, LV_ALIGN_CENTER, 0, 0);
#endif

	lv_obj_t* background = lv_img_create(gui_root(), NULL);
	lv_obj_add_style(background, LV_IMG_PART_MAIN, &nostyle);
	lv_obj_set_size(background, 212, 210);
	lv_img_set_src(background, &xploading);
	lv_obj_align(background, NULL, LV_ALIGN_CENTER, 0, 0);

	do_wakeup();

	for (int i=1;1;i++)
	{
		ft5206_read_touches();
		if (0 == (i & 0xFF))
		{
			axp202_update();
			ESP_LOGW(__func__, "AXP202 BATT[%s%s - %d%% %dmV @%dmA %dmW] (%0d C) VBUS[%s %dmV %dmA]",
				AXP202.battery.is_connected ? "OK:" : "NC!",
				AXP202.battery.is_charging  ? "charging @": "not charging ",
				AXP202.battery.percentage,
				AXP202.battery.voltage,
				AXP202.battery.current,
				AXP202.battery.power,

				AXP202.temperature,
				AXP202.vbus.is_connected ? "OK:" : "NA!",
				AXP202.vbus.voltage,
				AXP202.vbus.current);
				do_sleep();
		}

		if (FT5206.touch.count == 1)
		{
			ft5206_touch_t p = FT5206.touch.points[0];
			RC.payload.steering = 1000 * (120-p.x) / 120;
			RC.payload.throttle = 1000 * (120-p.y) / 120;
			
		}
		else
		{
			RC.payload.steering = 0;
			RC.payload.throttle = 0;
		}

		//ESP_LOG_BUFFER_HEX("PAYLOAD", &RC.packets.controls.payload, sizeof(rc_payload_t));
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
}
