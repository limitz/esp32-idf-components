#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <esp_system.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/ledc.h>

#include <lvgl/lvgl.h>
#include <lvgl_helpers.h>

#include "gui.h"

#define lv_arc_get_bg_start_angle lv_arc_get_bg_angle_start
#define lv_arc_get_bg_end_angle lv_arc_get_bg_angle_end
#define nullptr NULL
#define LV_TICK_PERIOD_MS 20

static SemaphoreHandle_t s_lock_ui;
static TaskHandle_t s_main_task_handle = nullptr;
static esp_timer_handle_t s_periodic_timer = nullptr;
static lv_style_t s_background;
static lv_obj_t* s_screen;

gui_driver_t GUI = {};

static void proc_tick(void* args)
{
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

#if CONFIG_LMTZ_BACKLIGHT_PWM_EN

static int backlight_fade_to(int level, int time)
{
#if 1
	ledc_set_duty_and_update(LEDC_HIGH_SPEED_MODE, CONFIG_LMTZ_BACKLIGHT_CHANNEL,
			level * ((1<<CONFIG_LMTZ_BACKLIGHT_PWM_BITS)-1)/100,
			0xFFFFF);
	return ESP_OK;
#else

	if (ESP_ERR_INVALID_STATE == ledc_fade_func_install(0)) return ESP_FAIL;

	ESP_LOGE(__func__, "START FADE");
	ESP_ERROR_CHECK(ledc_set_fade_step_and_start(
			LEDC_HIGH_SPEED_MODE, CONFIG_LMTZ_BACKLIGHT_CHANNEL,
			level * ((1<<CONFIG_LMTZ_BACKLIGHT_PWM_BITS)-1)/100,
			time, CONFIG_LMTZ_BACKLIGHT_CYCLES,
			LEDC_FADE_WAIT_DONE));
	ledc_fade_func_uninstall();
	ESP_LOGE(__func__, "STOP_FADE");
	
	return ESP_OK;
#endif
}

static void task_wakeup()
{
}

static void task_sleep()
{
}


static void backlight_init()
{
        ledc_timer_config_t timer = {
                .duty_resolution = CONFIG_LMTZ_BACKLIGHT_PWM_BITS, //LEDC_TIMER_15_BIT,
                .freq_hz = CONFIG_LMTZ_BACKLIGHT_PWM_FREQ,
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .timer_num = CONFIG_LMTZ_BACKLIGHT_TIMER, //LEDC_TIMER_0
        };
        ESP_ERROR_CHECK(ledc_timer_config(&timer));

        ledc_channel_config_t channel = {
                .channel = CONFIG_LMTZ_BACKLIGHT_CHANNEL,
                .duty = 0, //CONFIG_LMTZ_BACKLIGHT_LEVEL_ON,
                .gpio_num = CONFIG_LMTZ_BACKLIGHT_PIN,
                .intr_type = LEDC_INTR_DISABLE,
                .speed_mode = LEDC_HIGH_SPEED_MODE,
                .timer_sel = CONFIG_LMTZ_BACKLIGHT_TIMER, //LEDC_TIMER_0,
        };
      	ESP_ERROR_CHECK(ledc_channel_config(&channel));

//	task_wakeup();
	//xTaskCreatePinnedToCore(task_wakeup, "wakeup", 2048, NULL, 0, NULL, 1);
}

#endif

static void task_gui(void* args)
{
        for (;;)
        {
                vTaskDelay(1);
                if (xSemaphoreTake(s_lock_ui, (TickType_t) 10) == pdTRUE)
                {
			if (GUI.on_frame) GUI.on_frame();
                        lv_task_handler();
                        xSemaphoreGive(s_lock_ui);
                }
        }
}

int gui_init()
{
	ledc_fade_func_install(0);

	s_lock_ui = xSemaphoreCreateMutex();

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


        static lv_style_t* s;
        s = &s_background;
        lv_style_init(s);
        lv_style_set_border_side(s, LV_STATE_DEFAULT, LV_BORDER_SIDE_NONE);
        lv_style_set_outline_width(s, LV_STATE_DEFAULT, 0);
        lv_style_set_radius(s, LV_STATE_DEFAULT, 0);
        lv_style_set_bg_opa(s, LV_STATE_DEFAULT, LV_OPA_100);
        lv_style_set_bg_color(s, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
        //lv_style_set_image_recolor(s, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        //lv_style_set_image_recolor_opa(s, LV_STATE_DEFAULT, LV_OPA_100);
        lv_style_set_text_font(s, LV_STATE_DEFAULT, &lv_font_unscii_8);
        lv_style_set_text_color(s, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xCC,0xCC,0xCC));
        lv_style_set_line_rounded(s, LV_STATE_DEFAULT, true);


	s_screen = lv_scr_act(); //lv_obj_create(lv_scr_act(), NULL);
	lv_obj_add_style(s_screen, LV_OBJ_PART_MAIN, &s_background);
	lv_obj_set_size(s_screen, CONFIG_LVGL_DISPLAY_WIDTH, CONFIG_LVGL_DISPLAY_HEIGHT);
	lv_obj_set_pos(s_screen, 0, 0);

	backlight_init();

	return ESP_OK;
}

int gui_start(bool animate)
{
	if (!s_main_task_handle)
	{
        	const esp_timer_create_args_t periodic_timer_args = {
              		.callback = &proc_tick,
                	.name = "periodic ui timer"
        	};

        	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &s_periodic_timer));
	        ESP_ERROR_CHECK(esp_timer_start_periodic(s_periodic_timer, LV_TICK_PERIOD_MS * 1000));
	}
	else {
		
	        ESP_ERROR_CHECK(esp_timer_start_periodic(s_periodic_timer, LV_TICK_PERIOD_MS * 1000));
		lv_task_handler();
                lv_task_handler();
		proc_tick(NULL);
	}

	backlight_fade_to(60, CONFIG_LMTZ_BACKLIGHT_SCALE_ON);
	xTaskCreatePinnedToCore(task_gui, "GUI task", 4096, NULL, 0, &s_main_task_handle, 0);

	return ESP_OK;
}

int gui_stop(bool animate)
{
	vTaskDelete(s_main_task_handle);
	
	backlight_fade_to(0, CONFIG_LMTZ_BACKLIGHT_SCALE_ON);
	
	ESP_ERROR_CHECK(esp_timer_stop(s_periodic_timer));
	
	return ESP_OK;
}

int gui_deinit()
{
	ledc_fade_func_uninstall();
	ESP_ERROR_CHECK(gui_stop(false));
	//lvgl_driver_deinit();
	lv_deinit();

	vSemaphoreDelete(s_lock_ui);
	return ESP_OK;
}

lv_obj_t* gui_root()
{
	return s_screen;
}
