#include "camera.h"

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
		.fb_count = 1,
	},
};

int _camera_init()
{
	int err;

	ESP_ERROR_CHECK(err = esp_camera_init(&CAMERA.config));
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

	esp_camera_fb_return(fb);
	return ESP_OK;
}


void app_main()
{
	ESP_ERROR_CHECK(_camera_init());

	for (;;)
	{
		ESP_ERROR_CHECK(_camera_capture());
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
