#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include "audio.h"
#include "wave.h" // audio_table

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );

	audio_init(&AUDIO);
	audio_play(&AUDIO, audio_table, sizeof(audio_table));
	
}
