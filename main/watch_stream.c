#include <stdint.h>
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

#include <i2cbus.h>
#include <radio.h>
#include <axp202.h>
#include <ft5206.h>
#include <esp32/rom/tjpgd.h>
#include <esp_private/wifi.h>


#define WIDTH 240
#define HEIGHT 240
#define TYPE LV_IMG_CF_TRUE_COLOR
#define LDO_BACKLIGHT AXP202_LDO2

#define STREAM_WIDTH 320
#define STREAM_HEIGHT 240
#define WORKSPACE 3100

extern const uint8_t image_jpg_start[] asm("_binary_image_jpg_start");
extern const uint8_t image_jpg_end[]   asm("_binary_image_jpg-end");

QueueHandle_t decomp_queue;

typedef struct
{
	lv_obj_t* canvas;
	
	void* _work;
	JDEC _decoder;

	struct
	{
		uint8_t *data;
		int width, height, pos;
	} in;

	struct
	{
		lv_color16_t* data;
		int width, height;
	} out;

} jpg_dev_t;

typedef struct {
	void* data;
	int offset;
	int length;
} datareader_t;

static jpg_dev_t* s_devptr = NULL;

static UINT in_cb(JDEC *decoder, BYTE* buf, UINT len)
{
	datareader_t* dr = (datareader_t*)decoder->device;
	//ESP_LOGI(__func__, "Reading (%d +%d) of %d", dr->offset, len,  dr->length);
	if (len && buf) memcpy(buf, dr->data + dr->offset, len);
	dr->offset += len;
	return len;
}

static UINT out_cb(JDEC *decoder, void* bitmap, JRECT *r)
{
	jpg_dev_t* dev = s_devptr;
	//datareader_t* dr = (datareader*)decoder->device;

	uint8_t* ptr = (uint8_t*) bitmap;
	int w = 1+(r->right - r->left);
	int h = 1+(r->bottom - r->top);
	//ESP_LOGW(__func__, "%d,%d %dx%d", r->left, r->top,  w, h);

	if (r->right >= WIDTH) return 1;
	if (r->bottom >= HEIGHT) return 1;

	/*
	for (int row=0; row<h; row++)
	{
		memcpy(dev->out.data + (r->top+row) * WIDTH + r->left, ptr + 2 * row * w, w * 2);
	}
	*/
	for (int row=0; row<h; row++)
	{
		int y = r->top + row;
		if (y >= HEIGHT) break;
		lv_color16_t*  pdst = dev->out.data + (r->top + row) * WIDTH + r->left;
		for (int col=0; col<w; col++)
		{
			int x = r->left + col;
			if (x >= WIDTH) break;

			pdst[col].ch.red     = 0x1F & ptr[row * w * 3 + col * 3 + 0] >> 3;
			pdst[col].ch.green_h = 0x07 & ptr[row * w * 3 + col * 3 + 1] >> 5;
			pdst[col].ch.green_l = 0x07 & ptr[row * w * 3 + col * 3 + 1] >> 2;
			pdst[col].ch.blue    = 0x1F & ptr[row * w * 3 + col * 3 + 2] >> 3;
		}
	}

	return 1;
}



// make a queue aruund this 
void decoder_task(void* param)
{
	datareader_t* dr;
	while (pdTRUE == xQueueReceive(decomp_queue, &dr, portMAX_DELAY))
	{
		if (dr->data)
		{
			int err = jd_prepare(&s_devptr->_decoder, in_cb, s_devptr->in.data+8192, WORKSPACE, dr);
			if (JDR_OK != err)
			{
				heap_caps_free(dr->data);
				free(dr);
				ESP_LOGW(__func__, "Error during prepare jpeg %08X", err);
				continue;
			}
	
	
			err = jd_decomp(&s_devptr->_decoder, out_cb, 0);
			if (JDR_OK != err)
			{
				ESP_LOGW(__func__, "Error during decomp %08X", err);
				heap_caps_free(dr->data);
				free(dr);
				continue;
			}

			lv_obj_invalidate(s_devptr->canvas);
			heap_caps_free(dr->data);
			free(dr);

		}
	}
}

static int handle_receive(const radio_packet_t* p)
{
	switch (p->header.type)
	{
	case 5: //RADIO_PACKET_TYPE_CUSTOM+3:
		
		memcpy(((uint8_t*)s_devptr->in.data)+p->header.offset, p->payload, p->header.length);
		if (p->header.offset > 0 && p->header.length < RADIO_PACKET_MAX_PAYLOAD)
		{
			datareader_t* dr = malloc(sizeof(datareader_t));
			dr->data = s_devptr->in.data;
			s_devptr->in.data = heap_caps_calloc(8192+WORKSPACE, 1, MALLOC_CAP_8BIT);
			dr->offset = 0;
			dr->length = p->header.offset + p->header.length;
			xQueueSend(decomp_queue, &dr, 1);
		}

		break;
	}
	return RADIO_OK;
}
void app_main()
{
	RADIO.callbacks.on_receive=handle_receive;
	
	ESP_ERROR_CHECK( nvs_flash_init() );
	ESP_ERROR_CHECK( i2c_init() );
	ESP_ERROR_CHECK( gui_init() );
	ESP_ERROR_CHECK( ft5206_init() );
	ESP_ERROR_CHECK( axp202_init() );

	ESP_LOGW("FT5206", "VENDOR: %02x CHIP: %02x", 
			FT5206.info.ctpm_vendor_id, 
			FT5206.info.chip_vendor_id);
	gui_start(false);
	axp202_set_output(LDO_BACKLIGHT, AXP202_ON);

	static jpg_dev_t dev = {

		.in.width = STREAM_WIDTH,
		.in.height = STREAM_HEIGHT,
		.in.pos = 0,

		.out.width = WIDTH,
		.out.height = HEIGHT,
		
	};
	dev.canvas = lv_canvas_create(lv_scr_act(), NULL);
	dev.in.data = heap_caps_calloc(8192+WORKSPACE, 1, MALLOC_CAP_8BIT);
	dev.out.data = heap_caps_calloc(
			lv_img_buf_get_img_size(WIDTH, HEIGHT, TYPE), 1, 
			 MALLOC_CAP_8BIT);
	dev._work = heap_caps_calloc(WORKSPACE, 1, MALLOC_CAP_8BIT);

	for (int r=0; r<HEIGHT; r++)
	{
		for (int c = 0; c < WIDTH; c++)
		{
			dev.out.data[r * WIDTH + c].ch.red = r;
			dev.out.data[r * WIDTH + c].ch.blue = c;
		}
	}
	lv_obj_set_pos(dev.canvas, 0, 0);
	lv_obj_set_size(dev.canvas, WIDTH, HEIGHT);

#if 0
#define STREAM_WIDTH 336
#define STREAM_HEIGHT 256
	dev.in.data = image_jpg_start;
	int err = jd_prepare(&dev._decoder, in_cb, dev._work, WORKSPACE, &dev);
	if (JDR_OK != err) 
	{
		ESP_LOGE(__func__, "JD_PREPARE: %02x", err);
		ESP_ERROR_CHECK(ESP_ERR_NOT_SUPPORTED);
	}



	err = jd_decomp(&dev._decoder, out_cb, 0);
	if (JDR_OK != err) 
	{
		ESP_LOGE(__func__, "JD_DECOMPE: %02x", err);
		ESP_ERROR_CHECK(ESP_ERR_NOT_SUPPORTED);
	}
#endif

	lv_canvas_set_buffer(dev.canvas, dev.out.data, WIDTH, HEIGHT, TYPE);
	lv_obj_invalidate(dev.canvas);

	decomp_queue = xQueueCreate(20, sizeof(datareader_t*));

	xTaskCreate(decoder_task, "decoder task", 4096, NULL, 4, NULL);
	
	s_devptr = &dev;
	ESP_ERROR_CHECK( radio_init() );

	//esp_wifi_internal_set_fix_rate(ESPNOW_WIFI_IF, 1, WIFI_PHY_RATE_MCS7_SGI);
	esp_wifi_internal_set_fix_rate(ESPNOW_WIFI_IF, 1, 
			WIFI_PHY_RATE_11M_L);

	for (;;)
	{
		ft5206_read_touches();
		
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
	
		radio_broadcast(RADIO_PACKET_TYPE_IDENTITY, &RADIO.identity, sizeof(radio_identity_t));
		vTaskDelay(200);
	}
}


