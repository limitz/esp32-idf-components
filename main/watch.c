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


//#include "xploading.c"
#include "../resources/w95_computer.c"


#define LDO_BACKLIGHT AXP202_LDO2
#define nullptr NULL


static lv_obj_t* p1;
static lv_obj_t* canvas;
static float x = 0;
static float y = 0;
static float dx = 1.3;
static float dy = 1.37;


void on_frame()
{
}

void do_wakeup()
{
	gui_start(true);
	axp202_set_output(LDO_BACKLIGHT, AXP202_ON);
}

void do_sleep()
{
	axp202_set_output(LDO_BACKLIGHT, AXP202_OFF);
	gui_stop(true);
//	ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(60000000));
	
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

	GUI.on_frame = on_frame;	
	do_wakeup();
	
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
#else

	lv_coord_t w = 160;
	lv_coord_t h = 160;
	lv_coord_t wc = 80;
	lv_coord_t hc = 80;

	int type = LV_IMG_CF_INDEXED_4BIT;

	int bpp= lv_img_cf_get_px_size(type);
	int doc_nbytes = ((bpp * w) >> 3) * h;
	int act_nbytes = lv_img_buf_get_img_size(w,  h,  type);	
	int act_tocopy = lv_img_buf_get_img_size(wc, hc, type);
	int src_stride = lv_img_buf_get_img_stride(w, type);

	printf("bpp %d - doc bytes %d - act nbytes %d - act tocopy %d\n",
		bpp,     doc_nbytes,    act_nbytes,     act_tocopy);

	void* pbuf = heap_caps_calloc(act_nbytes,1, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
	void* cbuf = heap_caps_calloc(act_tocopy,1, MALLOC_CAP_SPIRAM);

	memset(pbuf, 0x11,  act_nbytes);

	canvas = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas, pbuf, w, h, type);
	lv_obj_set_pos(canvas, 40, 80);
	lv_obj_set_size(canvas, 160, 160);
	
#if 1
	int pentries = lv_img_buf_get_palette_size(type) / sizeof(lv_color32_t);
	printf("Number of palette entries: %d\n", pentries);

	for (int pent = 0; pent < pentries; pent++)
	{
		lv_color_t c = LV_COLOR_MAKE(
				(pent * 0xFF) / pentries,
				((pentries - pent) * 0xFF) / pentries,
				0x80);
		
		
		lv_canvas_set_palette(canvas, pent, c);
	}

	for (int pent = 0; pent < pentries; pent++)
	{
		printf("%X: %08X\n", pent, ((uint32_t*)pbuf)[pent]);
	}
	lv_obj_invalidate(canvas);

#else
	lv_canvas_set_palette(canvas, 0x0, LV_COLOR_MAKE( 0x33, 0x00, 0xCC ));
	lv_canvas_set_palette(canvas, 0x1, LV_COLOR_MAKE( 0x66, 0x00, 0x99 ));
	lv_canvas_set_palette(canvas, 0x2, LV_COLOR_MAKE( 0x99, 0x00, 0x66 ));
	lv_canvas_set_palette(canvas, 0x3, LV_COLOR_MAKE( 0xCC, 0x00, 0x33 ));

	lv_canvas_set_palette(canvas, 0x4, LV_COLOR_MAKE( 0xFF, 0x33, 0x00 ));
	lv_canvas_set_palette(canvas, 0x5, LV_COLOR_MAKE( 0xFF, 0x66, 0x00 ));
	lv_canvas_set_palette(canvas, 0x6, LV_COLOR_MAKE( 0xFF, 0x99, 0x00 ));
	lv_canvas_set_palette(canvas, 0x7, LV_COLOR_MAKE( 0xFF, 0xCC, 0x00 ));

	lv_canvas_set_palette(canvas, 0x8, LV_COLOR_MAKE( 0xCC, 0xFF, 0x33 ));
	lv_canvas_set_palette(canvas, 0x9, LV_COLOR_MAKE( 0x99, 0xFF, 0x66 ));
	lv_canvas_set_palette(canvas, 0xA, LV_COLOR_MAKE( 0x66, 0xFF, 0x99 ));
	lv_canvas_set_palette(canvas, 0xB, LV_COLOR_MAKE( 0x33, 0xFF, 0xCC ));

	lv_canvas_set_palette(canvas, 0xC, LV_COLOR_MAKE( 0x00, 0xCC, 0xFF ));
	lv_canvas_set_palette(canvas, 0xD, LV_COLOR_MAKE( 0x00, 0x99, 0xFF ));
	lv_canvas_set_palette(canvas, 0xE, LV_COLOR_MAKE( 0x00, 0x66, 0xFF ));
	lv_canvas_set_palette(canvas, 0xF, LV_COLOR_MAKE( 0x00, 0x44, 0xFF ));
#endif

#if 1
	int skip_palette = lv_img_buf_get_palette_size(type);
	int v = 0x01;
	switch(bpp) {
		case 8: v = 0x01; break;
		case 4: v = 0x11; break;
		case 2: v = 0x33; break;
		case 1: v = 0xFF; break;
	}
	for (int y=0; y<hc; y++)
	{
		for (int x=0; x < 8 * src_stride; x += bpp)
		{
			int py = y * pentries / hc;
			int px = (x / 8) * pentries / wc;
			int cv = (((py + px)/2) & (pentries-1)) << (x & 0x03);
			((uint8_t*)cbuf)[skip_palette + y * src_stride + x / 8] |= cv;
		}
	}
	lv_canvas_copy_buf(canvas, cbuf, 40, 40, 80, 80); // center it
#endif
#endif

	/*
	for (int x=0; x<w; x++)
	{
		for (int y=0; y<h; y++)
		{
			lv_color_t color;
			color.full = x/10;
			//lv_color_t color = LV_COLOR_MAKE(x, y, 0xFF);//{ . = (x / 10) & 0xF };
			lv_canvas_set_px(canvas, x/2, y/2, color);
		}
	}
	*/

#if 0
	size_t image_nbytes = 240 * 240 * 2;
	static lv_color_t* image = (lv_color_t*) malloc(nbytes);
	memset(image, 0x80, nbytes);
	static lv_obj_t* canvas = lv_canvas_create(gui_root(), NULL);

#endif

	//gui_start(true);


	
	for (int i=0;1;i++)
	{
		ft5206_read_touches();
	
		if (0xFFF == (i & 0xFFF))
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
