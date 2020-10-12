#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <dmx.h>

int app_main()
{
		gpio_config_t io_conf;

		io_conf.intr_type = GPIO_INTR_DISABLE;
		io_conf.mode = GPIO_MODE_OUTPUT;
    		io_conf.pin_bit_mask = 1<<21;
		io_conf.pull_down_en = 0;
		io_conf.pull_up_en = 0;
		gpio_config(&io_conf);

	dmx_init();
	for (int i=0;i>=0; i++)
	{


    		gpio_set_level(21, 0);
		DMX.universe->frame->channel[0] = 0x55;//* ((i / 5) % 2);
		DMX.universe->frame->channel[1] = 0x55; //* ((i / 5) % 2);
		DMX.universe->frame->channel[63] = 0x55; //* ((i / 5) % 2);
		dmx_update();
    		gpio_set_level(21, 1);
		vTaskDelay(10);
	}

	dmx_deinit();
	return 0;
}
