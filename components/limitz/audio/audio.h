#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/i2s.h>

#define I2S_NUM 		(0)
#define I2S_SAMPLE_RATE 	(16000)
#define I2S_SAMPLE_BITS 	(16)
#define I2S_BUFFER_SIZE 	(16 << 10)
#define I2S_FORMAT 		(I2S_CHANNEL_FMT_RIGHT_LEFT)
#define I2S_CHANNEL_NUM 	(I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT ? 2 : 1)
#define I2S_FLASH_SECTOR_SIZE 	(0x1000)
#define I2S_FLASH_ADDRESS 	(0x200000)

typedef struct
{
	int num;
	int channel;
	i2s_config_t config;
	uint8_t* wave;
	size_t wave_len;
	uint8_t* buffer_out;
}
audio_driver_t;

extern audio_driver_t AUDIO;

int audio_init(audio_driver_t* driver);
int audio_play(audio_driver_t* driver, uint8_t* wave, size_t len);
int audio_deinit();

