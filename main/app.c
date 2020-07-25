#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"

#include "unique_id.h"
#include "adc.h"
#include "lipo.h"
#include "wifi.h"

#include "logo_96x24.inc"

#define LV_TICK_PERIOD_MS 10


SemaphoreHandle_t lock_ui;

static adc_t     battery_monitor = ADC1;
static lv_obj_t* battery_label;
static lv_obj_t* battery_icon;
static lv_obj_t* logo_image;




static void process_battery_level()
{
	static char* icons[5] = { 
		LV_SYMBOL_BATTERY_EMPTY,
		LV_SYMBOL_BATTERY_1,
		LV_SYMBOL_BATTERY_2,
		LV_SYMBOL_BATTERY_3,
		LV_SYMBOL_BATTERY_FULL
	};
	static int icon_idx = 0;
	char status[16];
	char icon[16];

	adc_update(&battery_monitor);

	switch (battery_monitor.result)
	{
	case BATTERY_EMPTY:
		strcpy(icon, ((icon_idx/30) % 2) ? icons[0] : "");
		strcpy(status, "EMPTY");
		icon_idx++;
		break;
	case BATTERY_CHARGING:
		strcpy(icon, icons[(icon_idx/30)%5]);
		strcpy(status, "CHARGING");
		icon_idx++;
		break;
	default:
		sprintf(status, "%d%%", battery_monitor.result);
		strcpy(icon, icons[battery_monitor.result/20]);
		break;
	}
	if (xSemaphoreTake(lock_ui, (TickType_t)100) == pdTRUE)
	{
		lv_label_set_text(battery_label, status);
		lv_label_set_text(battery_icon, icon);
		xSemaphoreGive(lock_ui);
	}
}

static void process_tick(void* args) 
{ 
	process_battery_level();
	lv_tick_inc(LV_TICK_PERIOD_MS); 
}

static void task_ui(void* param)
{
	lock_ui = xSemaphoreCreateMutex();

	lv_init();
	lvgl_driver_init();

	static lv_color_t buffer1[DISP_BUF_SIZE];
	static lv_color_t buffer2[DISP_BUF_SIZE];
	static lv_disp_buf_t disp_buf;
	lv_disp_buf_init(&disp_buf, buffer1, buffer2, DISP_BUF_SIZE);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.flush_cb = disp_driver_flush;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &process_tick,
		.name = "periodic ui timer"
	};

	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

	static lv_style_t s;
    	static lv_style_t s_batt;

	lv_style_init(&s);
	lv_style_set_border_side(&s, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
	lv_style_set_outline_width(&s, LV_STATE_DEFAULT, 0);
	lv_style_set_radius(&s, LV_STATE_DEFAULT, 0);
	lv_style_set_bg_opa(&s, LV_STATE_DEFAULT, LV_OPA_100);
	lv_style_set_bg_color(&s, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_image_recolor(&s, LV_STATE_DEFAULT, LV_COLOR_WHITE);
	lv_style_set_image_recolor_opa(&s, LV_STATE_DEFAULT, LV_OPA_100);
	lv_style_set_text_font(&s, LV_STATE_DEFAULT, &lv_font_unscii_8);
       	lv_style_set_text_color(&s, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xAA,0xAA,0xAA));

	lv_style_init(&s_batt);
	lv_style_set_text_font(&s_batt, LV_STATE_DEFAULT, &lv_font_montserrat_12);
	lv_obj_add_style(lv_scr_act(),LV_OBJ_PART_MAIN, &s);

	lv_obj_t* bg = lv_obj_create(lv_scr_act(), NULL);
	lv_obj_add_style(bg, LV_OBJ_PART_MAIN, &s);
	lv_obj_set_size(bg, 240, 136);
	lv_obj_set_pos(bg, 0, 0);

	logo_image = lv_img_create(bg, NULL);
	lv_obj_add_style(logo_image, LV_IMG_PART_MAIN, &s);
	lv_obj_set_size(logo_image, 96, 24);
	lv_obj_align(logo_image, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 4);
	lv_img_set_src(logo_image, &splendo_logo);

	battery_label = lv_label_create(bg, NULL);     /*Add a button the current screen*/
	lv_obj_align(battery_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -30, 4);
	lv_obj_set_auto_realign(battery_label, 1);

	lv_obj_t* version_label = lv_label_create(bg, NULL);     /*Add a button the current screen*/
	lv_label_set_text(version_label, unique_id());
	lv_obj_align(version_label, NULL, LV_ALIGN_IN_TOP_RIGHT, -6, 16);
	//lv_obj_set_auto_realign(version_label, 1);


	battery_icon = lv_label_create(bg, NULL);     /*Add a button the current screen*/
	lv_obj_add_style(battery_icon, LV_OBJ_PART_MAIN, &s_batt);
	//lv_label_set_align(battery_icon, LV_LABEL_ALIGN_RIGHT);
	lv_obj_align(battery_icon, NULL, LV_ALIGN_IN_TOP_RIGHT, -6, 0);
	lv_obj_set_auto_realign(battery_icon, 1);

	for (;;)
	{
		vTaskDelay(1);
		if (xSemaphoreTake(lock_ui, (TickType_t) 10) == pdTRUE) 
		{
			lv_task_handler();
			xSemaphoreGive(lock_ui);
		}
	}
}

int wifi_connected(wifi_t* wifi, int  e)
{
	if (ESP_OK != e)
	{
		ESP_LOGW("APP", "Error in callback: %x", e);
	}
	return e;
}

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_LOGW("APP", "unique id = %s", unique_id());

	adc_init(&battery_monitor);

	xTaskCreatePinnedToCore(task_ui, "task_ui", 8192, NULL, 0, NULL, 1);
}




	//adc_t joy_x = ADC1;
	//adc_init(&joy_x);	
	//adc_t joy_y = ADC2;
	//adc_init(&joy_y);
	
	//apa102_t ledstrip = APA102_DEFAULT;
	//apa102_init(&ledstrip);
	
	//encoder_t enc1 = ENCODER_1_DEFAULT;
	//encoder_init(&enc1);
	
	//servo_t servo1 = SERVO1;
	//servo_t servo2 = SERVO2;
	//servo_init(&servo1);
	//servo_init(&servo2);

	//ht16k33_t display1 = HT16K33_DEFAULT(0x71);
	//ht16k33_t display2 = HT16K33_DEFAULT(0x73);
	//ht16k33_init(&display1);
	//ht16k33_init(&display2);

	//static wifi_t wifi = WIFI_PROVISION_OTA;
	//wifi.ssid = CONFIG_WIFI_SSID;
	//wifi.password = CONFIG_WIFI_PASSWORD;
	//wifi.ota_url = "https://ota.splendo.health/release/SplendoFitBridge";
	//wifi_init(&wifi);
