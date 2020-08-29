#include "ms12a.h"
#include "esp_system.h"
#include "esp_log.h"

// loosely based on https://github.com/Makeblock-official/Makeblock-Libraries/blob/master/src/MeSmartServo.h


inline void ms12a_store_7bit(uint8_t* data, size_t len, uint32_t v)
{
	while (len--)
	{
		*(data++) = v & 0x7F;
		v >>= 7;
	}
}

inline uint32_t ms12a_load_7bit(uint8_t* data, size_t len)
{
	uint32_t result = 0;
	while (len--)
	{
		result <<= 7;
		result |= data[len] & 0x7F;
	}
	return result;
}

static int ms12a_write_message(uint8_t dst, const uint8_t* message,int len)
{
	uint8_t data[MS12A_BUFFER_SIZE];
	uint8_t hash = 0;
	data[0] = MS12A_SYSEX_START;
	data[1] = hash = dst;
	for (int i=0; i<len; i++) hash += data[2+i] = message[i];
	data[len+2] = hash & 0x7F;
	data[len+3] = MS12A_SYSEX_STOP;
	uart_write_bytes(MS12A_UART_PORT, (const char*) data, len+4);
	return ESP_OK;
}
#define _write(dst, message) \
	ms12a_write_message(dst, (uint8_t*) message, sizeof(message))

#define _load7(message, offset, nbytes) \
	ms12a_load_7bit(((uint8_t*)message) + offset, nbytes)

#define _store7(message, offset, nbytes, value) \
	ms12a_store_7bit(((uint8_t*)message)+offset, nbytes, (uint32_t) value)



#define PLACEHOLDER_U32 0x00, 0x00, 0x00, 0x00, 0x00
#define PLACEHOLDER_U16 0x00, 0x00, 0x00
#define PLACEHOLDER_U14 0x00, 0x00
#define PLACEHOLDER_U8  0x00, 0x00
#define PLACEHOLDER_U7  0x00

#define PLACEHOLDER_I32 PLACEHOLDER_U32
#define PLACEHOLDER_I16 PLACEHOLDER_U16
#define PLACEHOLDER_I14 PLACEHOLDER_U14
#define PLACEHOLDER_I8  PLACEHOLDER_U8
#define PLACEHOLDER_I7  PLACEHOLDER_U7

int ms12a_assign_ids()
{
	uint8_t message[] = { MS12A_CTL_ASSIGN_ID, 0x00 };
	return _write(MS12A_ALL, message);
}

int ms12a_set_angle_abs(uint8_t servo, int32_t angle, uint16_t speed)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_ANGLE_ABS_LONG, 
				PLACEHOLDER_I32, // angle
				PLACEHOLDER_U14, // speed
	};

	_store7(message, 2, +5, angle);
	_store7(message, 7, +2, speed);
	return _write(servo, message);
}

int ms12a_set_angle_rel(uint8_t servo, int32_t angle, uint16_t speed)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_ANGLE_REL_LONG,
				PLACEHOLDER_I32, // angle
				PLACEHOLDER_U14, // speed
	};

	_store7(message, 2, +5, angle);
	_store7(message, 7, +2, speed);
	return _write(servo, message);
}

int ms12a_set_angle_tare(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_ANGLE_TARE };
	return _write(servo, message);
}

int ms12a_set_break(uint8_t servo, uint7_t value)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_BREAK,
				PLACEHOLDER_U7, // value
	};
	_store7(message, 2, +1, value);
	return _write(servo, message);
}

int ms12a_set_led(uint8_t servo, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_LED,
				PLACEHOLDER_U8, // red
				PLACEHOLDER_U8, // green
				PLACEHOLDER_U8, // blue
	};

	_store7(message, 2, +2, r);
	_store7(message, 4, +2, g);
	_store7(message, 6, +2, b);
	return _write(servo, message);
}

int ms12a_handshake(uint8_t servo)
{
	if (servo >= MS12A_NUM_SERVOS && servo != MS12A_ALL) return ESP_FAIL;
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_HANDSHAKE };
	return _write(servo, message);
}

int ms12a_set_pwm(uint8_t servo, uint14_t pwm)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_PWM,
				PLACEHOLDER_U16, // pwm
	};

	_store7(message, 2, +3, pwm);
	return _write(servo, message);
}

int ms12a_return_to_zero(uint8_t servo, uint7_t mode, uint16_t speed)
{
	uint8_t message[]= { MS12A_SERVO, MS12A_CMD_RETURN_TO_ZERO,
				PLACEHOLDER_U7,  // mode
				PLACEHOLDER_U14, // speed
	};

	_store7(message, 2, +1, mode);
	_store7(message, 3, +2, speed);
	return _write(servo, message);
}

int ms12a_get_angle(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_ANGLE, 0x00 };
	return _write(servo, message);
}

int ms12a_get_speed(uint8_t servo)
{
	uint8_t message[] = {MS12A_SERVO, MS12A_CMD_GET_SPEED, 0x00 };
	return _write(servo, message);
}

int ms12a_get_voltage(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_VOLTAGE, 0x00 };
	return _write(servo, message);
}

int ms12a_get_current(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_CURRENT, 0x00 };
	return _write(servo, message);
}

int ms12_get_temperature(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_TEMPERATURE, 0x00 };
	return _write(servo, message);
}

int ms12a_get_pid(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_PID, 0x00 };
	return _write(servo, message);
}

int ms12a_get_compensation(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_COMPENSATION, 0x00 };
	return _write(servo, message);
}

int ms12a_get_position(uint8_t servo)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_GET_POSITION, 0x00 };
	return _write(servo, message);
}

int ms12a_set_mode(uint8_t servo, uint7_t mode)
{
	return ESP_FAIL;
}

int ms12a_report_when_position(uint8_t servo, int32_t position)
{
	return ESP_FAIL;
}

int ms12a_set_position_abs(uint8_t servo, int32_t position, uint14_t speed)
{
	return ESP_FAIL;
}

int ms12a_set_position_rel(uint8_t servo, int32_t position, uint14_t speed)
{
	uint8_t message[] = { MS12A_SERVO, MS12A_CMD_SET_POSITION_REL,
				PLACEHOLDER_I32, // angle
				PLACEHOLDER_U16, // speed
	};

	_store7(message, 2, +5, position);
	_store7(message, 7, +3, speed);
	return _write(servo, message);
	//return ESP_FAIL;
}


static void ms12a_rx_task(void* param)
{
	ms12a_t* self = (ms12a_t*) param;
	uart_event_t event;

	for (;;)
	{
		ESP_LOGE(__func__, "waiting for event");
		if(xQueueReceive(self->queue, &event, (portTickType) portMAX_DELAY)) 
		{
			ESP_LOGI(__func__, "got event");
			switch (event.type)
			{
				case UART_DATA:
					ESP_LOGI(__func__, "[UART DATA]: %d", event.size);
				        uart_read_bytes(MS12A_UART_PORT, 
							self->rx_buffer, event.size, 
							portMAX_DELAY);
					ESP_LOG_BUFFER_HEX(__func__, self->rx_buffer, event.size);
					break;
				case UART_FIFO_OVF:
					ESP_LOGE(__func__, "UART FIFO OVF");
					uart_flush_input(MS12A_UART_PORT);
					xQueueReset(self->queue);
					break;
				case UART_BUFFER_FULL:
					ESP_LOGE(__func__, " UART_BUFFER FULL");
					uart_flush_input(MS12A_UART_PORT);
					xQueueReset(self->queue);
					break;
				case UART_BREAK:
					ESP_LOGW(__func__, "UART BREAK");
					break;
				case UART_PARITY_ERR:
				case UART_FRAME_ERR:
				case UART_PATTERN_DET:
				default:
					ESP_LOGW(__func__, "Unhandled event %d of size %d", 
							event.type, event.size);
					break;
			}
		}
	}
}

int ms12a_init(ms12a_t* self)
{
	uart_config_t cfg = {
		.baud_rate = MS12A_UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};

	ESP_ERROR_CHECK(uart_param_config(MS12A_UART_PORT,&cfg));

	ESP_ERROR_CHECK(uart_set_pin(MS12A_UART_PORT, 
				MS12A_UART_PIN_TX, 
				MS12A_UART_PIN_RX, 
				UART_PIN_NO_CHANGE,
				UART_PIN_NO_CHANGE));

	ESP_ERROR_CHECK(uart_driver_install(MS12A_UART_PORT, 
				MS12A_RX_BUFFER_SIZE,
				0, //MS12A_RX_BUFFER_SIZE, 
				20, &self->queue, 0));
	
	xTaskCreate(ms12a_rx_task, "ms12a uart event",2048, self, 12, &self->task);
	
	vTaskDelay(50 / portTICK_PERIOD_MS);
	ms12a_assign_ids();
	vTaskDelay(500 / portTICK_PERIOD_MS);
	

	return ESP_OK;
}

int ms12a_test_7bit()
{
	uint8_t  u08   = 0xAA;
	uint16_t u14   = 0xBBB;
	uint16_t u16   = 0xCCCC;
	uint32_t u32   = 0xEEEEEEEE;

	uint8_t stream[16] = {0};

	_store7(stream, 0, +2, u08);
	_store7(stream, 2, +2, u14);
	_store7(stream, 4, +3, u16);
	_store7(stream, 7, +5, u32);

	ESP_LOG_BUFFER_HEX(__func__, stream, 16);

	ESP_ERROR_CHECK(u08 - _load7(stream, 0, +2));
	ESP_ERROR_CHECK(u14 - _load7(stream, 2, +2));
	ESP_ERROR_CHECK(u16 - _load7(stream, 4, +3));
	ESP_ERROR_CHECK(u32 - _load7(stream, 7, +5));

	return ESP_OK;
}

int ms12a_deinit(ms12a_t* self)
{
	vTaskDelete(self->task);
	uart_driver_delete(MS12A_UART_PORT);
	return ESP_OK;
}
