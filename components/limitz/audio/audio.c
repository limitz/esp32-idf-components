
#include "audio.h"

audio_driver_t AUDIO = {
	.num = I2S_NUM,
	.channel = I2S_CHANNEL_NUM,
	.config = {
		.mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN,
		.sample_rate = I2S_SAMPLE_RATE,
		.bits_per_sample = I2S_SAMPLE_BITS,
		.communication_format = I2S_COMM_FORMAT_I2S_MSB,
		.channel_format = I2S_FORMAT,
		.intr_alloc_flags = 0,
		.dma_buf_count = 2,
		.dma_buf_len = 1024,
		.use_apll = 1,
	},
};

static int i2s_scale(uint8_t* dst, const uint8_t* src, size_t len)
{
	for (int i=0; i<len; i++)
	{
		dst[2*i+0] = src[i];
		dst[2*i+1] = src[i];
	}
	return len * 2;
}

int audio_init(audio_driver_t* driver)
{
	int err = ESP_OK;
	
	ESP_ERROR_CHECK(err = i2s_driver_install(driver->num, &driver->config, 0, NULL));
	if (ESP_OK != err) return err;

	ESP_ERROR_CHECK(err = i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN));
	if (ESP_OK != err) return err;

	driver->buffer_out = (uint8_t*) malloc(8<<10);
	if (!driver->buffer_out) return ESP_FAIL;

	return ESP_OK;
}

static int audio_reset(audio_driver_t* driver)
{
	int err;
	//i2s_set_clk(driver->num, driver->config.sample_rate, driver->config.bits_per_sample, driver->channel);
	ESP_ERROR_CHECK(err = i2s_set_clk(driver->num, driver->config.sample_rate, driver->config.bits_per_sample, 1));
	if (ESP_OK != err) return err;

	return ESP_OK;
}

static void audio_play_task(void* param)//audio_driver_t* driver)
{
	int err;
	audio_driver_t* driver = (audio_driver_t*)param;

	ESP_ERROR_CHECK(err = audio_reset(driver));

	size_t written = 0;

	while (written < driver->wave_len)
	{
		
		size_t iav = (driver->wave_len - written);
		if (iav > (4<<10)) iav = (4<<10);
		size_t oav = i2s_scale(driver->buffer_out, driver->wave + written, iav);
		i2s_write(driver->num, driver->buffer_out, oav, &oav, portMAX_DELAY);
		ESP_LOGI(__func__, "WRITTEN %d", oav);
		written += iav;
	}
	vTaskDelay(100 / portTICK_PERIOD_MS);
	vTaskDelete(NULL);
}

int audio_play(audio_driver_t* driver, uint8_t* wave, size_t len)
{
	driver->wave = wave;
	driver->wave_len = len;
	xTaskCreate(audio_play_task, "audio playing", 2<<10, driver, 5, NULL);

	return ESP_OK;
}

int audio_deinit()
{
	return ESP_OK;
}


