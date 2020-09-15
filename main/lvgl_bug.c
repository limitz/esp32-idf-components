#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_heap_caps.h>
#include <gui.h>


static lv_obj_t* canvas;

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( gui_init() );

	gui_start(false);
	
	lv_coord_t w = 140;
	lv_coord_t h = 140;
	lv_coord_t wc = 80;
	lv_coord_t hc = 80;

	int type = LV_IMG_CF_INDEXED_4BIT;

	int bpp= lv_img_cf_get_px_size(type);
	int doc_nbytes = ((bpp * w) >> 3) * h;
	int act_nbytes = lv_img_buf_get_img_size(w,  h,  type);	
	int act_tocopy = lv_img_buf_get_img_size(wc, hc, type);
	int src_stride = lv_img_buf_get_img_stride(wc, type);
	int dst_stride = lv_img_buf_get_img_stride(w, type);
	
	printf("bpp %d - doc bytes %d - act nbytes %d - act tocopy %d\n",
		bpp,     doc_nbytes,    act_nbytes,     act_tocopy);

	void* pbuf = heap_caps_calloc(act_nbytes,1, MALLOC_CAP_8BIT);
	void* cbuf = heap_caps_calloc(act_tocopy, 1, MALLOC_CAP_8BIT);

	memset(pbuf, 0x11,  act_nbytes);
	for (int i=0; i<h; i++)
	{
		for (int j=0; j<w; j++)
		{
			uint8_t* ptr = (uint8_t*) pbuf;
			ptr += lv_img_buf_get_palette_size(type);
			ptr[i*dst_stride + j * bpp / 8] = 0xFF;
		}
	}
	
	canvas = lv_canvas_create(lv_scr_act(), NULL);
	lv_canvas_set_buffer(canvas, pbuf, w, h, type);
	
	int pentries = lv_img_buf_get_palette_size(type) / sizeof(lv_color32_t);
	printf("Number of palette entries: %d\n", pentries);
#if 0

	for (int pent = 0; pent < pentries; pent++)
	{
		lv_color_t c = LV_COLOR_MAKE(
				(pent * 0xFF) / pentries,
				((pentries - pent) * 0xFF) / pentries,
				0x80);
		
		
		lv_canvas_set_palette(canvas, pent, c);
	}


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
	for (int pent = 0; pent < pentries; pent++)
	{
		printf("%X: %08X\n", pent, ((uint32_t*)pbuf)[pent]);
	}
	lv_obj_invalidate(canvas);

#if 1
	int skip_palette = lv_img_buf_get_palette_size(type);	
	for (int y=0; y<hc; y++)
	{
		for (int x=0; x < wc; x++)
		{
			int py = (y / 2) * 0x11;
			((uint8_t*)cbuf)[skip_palette + y * src_stride + x * bpp / 8] = py;
		}
	}

	lv_canvas_copy_buf(canvas, cbuf, (w-wc)/2, (h-hc)/2, wc, hc); // center it
	lv_obj_set_pos(canvas, 120 - w / 2, 120 - w/2);
	
#endif
}
