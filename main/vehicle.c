#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <driver/uart.h>
#include <apa102.h>

#define UART_BAUD_RATE 115200
#define UART_PORT 2
#define UART_PIN_TX 22
#define UART_PIN_RX 23
#define RX_BUFFER_SIZE 1024

static TaskHandle_t s_task;
static QueueHandle_t s_queue;
static uint8_t s_rx_buffer[1024];

#define uart_enable_pattern_det_baud_intr(port, ch, cnt, time, post, pre) \
	uart_enable_pattern_det_intr(port, ch, cnt, (time) << 8, (post) << 8, (pre) << 8)

#define ESC "\x1B"

#define CLS  ESC "[2J"
#define CLL  ESC "[2K"

#define CLS_UP ESC "[1J"
#define CLS_DN ESC "[0J"
#define CLL_BK ESC "[0K"
#define CLL_FW ESC "[1K"

#define INS(n) ESC "[" #n " " // or @

#define MOV_UP(n) ESC "[" #n "A"
#define MOV_DN(n) ESC "[" #n "B"
#define MOV_FW(n) ESC "[" #n "C"
#define MOV_BK(n) ESC "[" #n "D"
#define MOV_TO(r,c) ESC "[" #r ";" #c "H"

#define ATT(c) ESC "[" #c "m"


static void rx_task(void* param)
{
	uart_event_t event;
	int pos;
	for (;;)
	{
		if(xQueueReceive(s_queue, &event, (portTickType) portMAX_DELAY)) 
		{
			switch (event.type)
			{
				case UART_DATA:
					//ESP_LOGI(__func__, "[UART DATA]: %d", event.size);
#if 0
					uart_read_bytes(UART_PORT, 
							s_rx_buffer, event.size, 
							portMAX_DELAY);
					ESP_LOG_BUFFER_HEX(__func__, s_rx_buffer, event.size);
#endif
					break;
				case UART_FIFO_OVF:
					//ESP_LOGE(__func__, "UART FIFO OVF");
					uart_flush_input(UART_PORT);
					xQueueReset(s_queue);
					break;
				case UART_BUFFER_FULL:
					//ESP_LOGE(__func__, " UART_BUFFER FULL");
					uart_flush_input(UART_PORT);
					xQueueReset(s_queue);
					break;
				case UART_BREAK:
					//ESP_LOGW(__func__, "UART BREAK");
					break;
				case UART_PATTERN_DET:
					//ESP_LOGW(__func__, "Pattern detected");
					pos = uart_pattern_pop_pos(UART_PORT);
					if (pos > 0) uart_read_bytes(UART_PORT, s_rx_buffer, pos+3, portMAX_DELAY);
					//ESP_LOG_BUFFER_HEX(__func__, s_rx_buffer, event.size);
					break;

				case UART_PARITY_ERR:
				case UART_FRAME_ERR:
				default:
					ESP_LOGW(__func__, "Unhandled event %d of size %d", 
							event.type, event.size);
					break;
			}
		}
	}
}

void app_main()
{
	ESP_ERROR_CHECK( nvs_flash_init() );
	//ESP_ERROR_CHECK( APA102.init() );
	
	uart_config_t cfg = {
		.baud_rate = UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};

	ESP_ERROR_CHECK(uart_param_config(UART_PORT,&cfg));

	ESP_ERROR_CHECK(uart_set_pin(UART_PORT, 
				UART_PIN_TX, 
				UART_PIN_RX, 
				UART_PIN_NO_CHANGE,
				UART_PIN_NO_CHANGE));

	ESP_ERROR_CHECK(uart_driver_install(UART_PORT, 
				RX_BUFFER_SIZE,
				RX_BUFFER_SIZE, 
				20, &s_queue, 0));
	

	uart_disable_tx_intr(UART_PORT);
	uart_disable_rx_intr(UART_PORT);

	uart_enable_pattern_det_baud_intr(UART_PORT, '+', 3, 0xFFFF, 0, 0);


	xTaskCreate(rx_task, "uart event",2048, NULL, 12, &s_task);


	int attr = 1;
	printf(MOV_TO(0,0) CLS CLS_DN CLS_UP);
	for (;;)
	{
		/*
		for (int i=0; i<APA102.count; i++)
		{
			APA102.leds[i] = APA102_COLOR(0xAA, 0xAA, 0xFF, 0x1F);
		}
		APA102.update();
		*/
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		uart_write_bytes(UART_PORT, "hello\n", 6);
		printf(MOV_TO(0,0) ATT(%d) "LMTZ\r", attr++);
		fflush(stdout);

	}
}
