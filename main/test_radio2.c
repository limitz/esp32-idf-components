#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_heap_caps.h>
#include <gui.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_event_loop.h>
#include <sys/param.h>
#include <esp_private/wifi.h>

#include <radio.h>
#include <esp_camera.h>

typedef struct
{
	camera_config_t config;
} camera_t;

int _camera_init();
int _camera_capture();
int _camera_deinit();

extern camera_t CAMERA;

#define B1 0
#define B2 35

#define L1 32
#define L2 1000
#define ESP_INTR_FLAG_DEFAULT 0

camera_t CAMERA = {
	.config = {
		.pin_pwdn = -1,
		.pin_reset = -1,
		.pin_xclk = 4,
		.pin_sscb_sda = 18,
		.pin_sscb_scl = 23,

		.pin_d7 = 36,
		.pin_d6 = 37,
		.pin_d5 = 38,
		.pin_d4 = 39,
		.pin_d3 = 35,
		.pin_d2 = 26,
		.pin_d1 = 13,
		.pin_d0 = 34,

		.pin_vsync = 5,
		.pin_href = 27,
		.pin_pclk = 25,

		.xclk_freq_hz = 20000000,
		.ledc_timer = LEDC_TIMER_0,
		.ledc_channel = LEDC_CHANNEL_0,
		.pixel_format = PIXFORMAT_JPEG,
		.frame_size = FRAMESIZE_QVGA,
		.jpeg_quality = 12,
		.fb_count = 2,
	},
};

int _camera_init()
{
	int err;

	err = esp_camera_init(&CAMERA.config);

	if (ESP_OK != err) return err;

	return ESP_OK;
}

int _camera_deinit()
{
	// TODO
	return ESP_FAIL;
}

int _camera_capture()
{
	camera_fb_t *fb = esp_camera_fb_get();
	if (!fb) return ESP_FAIL;

	ESP_LOGI(__func__, "W: %d H:%d LEN:%d FMT:%d", fb->width, fb->height, fb->len, fb->format);
	radio_broadcast(RADIO_PACKET_TYPE_CUSTOM + 3, fb->buf, fb->len);

	esp_camera_fb_return(fb);
	return ESP_OK;
}

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	//ESP_ERROR_CHECK( gui_init() );


	_camera_init();

/*
	gpio_config_t io_conf;

	io_conf.intr_type = GPIO_INTR_POSEDGE;
  	io_conf.pin_bit_mask = (1ULL<<0) | (1ULL<<35);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);
	
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    	gpio_isr_handler_add(B1, gpio_isr_handler, (void*) B1);
    	gpio_isr_handler_add(B2, gpio_isr_handler, (void*) B2);
	
*/
	//RADIO.callbacks.on_receive=handle_receive;
	radio_init();

	esp_wifi_internal_set_fix_rate(ESPNOW_WIFI_IF, 1, 
			WIFI_PHY_RATE_11M_L);
			//WIFI_PHY_RATE_MCS7_SGI);
	//gui_start(false);
	/*
	s_type = LV_IMG_CF_INDEXED_4BIT;
	s_bpp= lv_img_cf_get_px_size(s_type);
	s_nbytes = lv_img_buf_get_img_size(s_w,  s_h,  s_type);	
	s_tocopy = lv_img_buf_get_img_size(s_wc, s_hc, s_type);
	s_src_stride = lv_img_buf_get_img_stride(s_wc, s_type);
	s_dst_stride = lv_img_buf_get_img_stride(s_w, s_type);
	
	void* pbuf = heap_caps_calloc(s_nbytes,1, MALLOC_CAP_8BIT);

	for (int i=0; i<s_h; i++)
	{
		for (int j=0; j<s_w; j++)
		{
			uint8_t* ptr = (uint8_t*) pbuf;
			ptr += lv_img_buf_get_palette_size(s_type);
			ptr[i*s_dst_stride + j * s_bpp / 8] = 0xFF;
		}
	}
	
	s_canvas = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(s_canvas, pbuf, s_w, s_h, s_type);
	
	//int pentries = lv_img_buf_get_palette_size(s_type) / sizeof(lv_color32_t);
	
	lv_canvas_set_palette(s_canvas, 0x0, LV_COLOR_MAKE( 0x33, 0x00, 0xCC ));
	lv_canvas_set_palette(s_canvas, 0x1, LV_COLOR_MAKE( 0x66, 0x00, 0x99 ));
	lv_canvas_set_palette(s_canvas, 0x2, LV_COLOR_MAKE( 0x99, 0x00, 0x66 ));
	lv_canvas_set_palette(s_canvas, 0x3, LV_COLOR_MAKE( 0xCC, 0x00, 0x33 ));

	lv_canvas_set_palette(s_canvas, 0x4, LV_COLOR_MAKE( 0xFF, 0x33, 0x00 ));
	lv_canvas_set_palette(s_canvas, 0x5, LV_COLOR_MAKE( 0xFF, 0x66, 0x00 ));
	lv_canvas_set_palette(s_canvas, 0x6, LV_COLOR_MAKE( 0xFF, 0x99, 0x00 ));
	lv_canvas_set_palette(s_canvas, 0x7, LV_COLOR_MAKE( 0xFF, 0xCC, 0x00 ));

	lv_canvas_set_palette(s_canvas, 0x8, LV_COLOR_MAKE( 0xCC, 0xFF, 0x33 ));
	lv_canvas_set_palette(s_canvas, 0x9, LV_COLOR_MAKE( 0x99, 0xFF, 0x66 ));
	lv_canvas_set_palette(s_canvas, 0xA, LV_COLOR_MAKE( 0x66, 0xFF, 0x99 ));
	lv_canvas_set_palette(s_canvas, 0xB, LV_COLOR_MAKE( 0x33, 0xFF, 0xCC ));

	lv_canvas_set_palette(s_canvas, 0xC, LV_COLOR_MAKE( 0x00, 0xCC, 0xFF ));
	lv_canvas_set_palette(s_canvas, 0xD, LV_COLOR_MAKE( 0x00, 0x99, 0xFF ));
	lv_canvas_set_palette(s_canvas, 0xE, LV_COLOR_MAKE( 0x00, 0x66, 0xFF ));
	lv_canvas_set_palette(s_canvas, 0xF, LV_COLOR_MAKE( 0x00, 0x44, 0xFF ));
	
	lv_obj_set_pos(s_canvas, (s_W - s_w) / 2, (s_H - s_h) / 2);
	*/

	for (;;)
	{
		//i//radio_broadcast(RADIO_PACKET_TYPE_IDENTITY, &RADIO.identity, sizeof(radio_identity_t));
		//vTaskDelay(100);

		ESP_ERROR_CHECK(_camera_capture());

	}
}


