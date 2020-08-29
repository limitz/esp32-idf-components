#pragma once

// loosely based on https://github.com/Makeblock-official/Makeblock-Libraries/blob/master/src/MeSmartServo.h

struct ms12a_driver_decl;
struct ms12a_servo_decl;
struct ms12a_message_decl;

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"

typedef uint16_t uint14_t;
typedef uint8_t uint7_t;
enum
{
	MS12A_CTL_ASSIGN_ID = 0x10,
	MS12A_CTL_SYSTEM_RESET = 0x11,
	MS12A_CTL_READ_VERSION = 0x12,
	MS12A_CTL_SET_BAUD_DATE = 0x13,
	MS12A_CTL_TEST = 0x14,
	MS12A_CTL_ERROR = 0x15,
};


enum
{
	MS12A_CMD_SET_PID = 0x10,
	MS12A_CMD_SET_POSITION_ABS = 0x11,
	MS12A_CMD_SET_POSITION_REL = 0x12,
	MS12A_CMD_SET_CONTINUOUS = 0x13,
	MS12A_CMD_SET_COMPENSATION = 0x14,
	MS12A_CMD_CLR_COMPENSATION = 0x15,
	MS12A_CMD_SET_BREAK = 0x16,
	MS12A_CMD_SET_LED = 0x17,
	MS12A_CMD_HANDSHAKE = 0x18,
	MS12A_CMD_SET_MODE = 0x19,

	MS12A_CMD_GET_STATUS = 0x20,
	MS12A_CMD_GET_PID = 0x21,
	MS12A_CMD_GET_POSITION = 0x22,
	MS12A_CMD_GET_SPEED = 0x23,
	MS12A_CMD_GET_COMPENSATION = 0x24,
	MS12A_CMD_GET_TEMPERATURE = 0x25,
	MS12A_CMD_GET_CURRENT = 0x26,
	MS12A_CMD_GET_VOLTAGE = 0x27,

	MS12A_CMD_SET_ANGLE_TARE = 0x30,
	MS12A_CMD_SET_ANGLE_ABS = 0x31,
	MS12A_CMD_SET_ANGLE_REL = 0x32,
	MS12A_CMD_SET_ANGLE_ABS_LONG = 0x33,
	MS12A_CMD_SET_ANGLE_REL_LONG = 0x34,
	MS12A_CMD_SET_PWM = 0x35,
	MS12A_CMD_GET_ANGLE = 0x36,
	MS12A_CMD_RETURN_TO_ZERO = 0x37,
	
	MS12A_CMD_REPORT = 0x40,
};

#define MS12A_ALL 0xFF
#define MS12A_CUSTOM 0x00
#define MS12A_SERVO  0x60
#define MS12A_SYSEX_START 0xF0
#define MS12A_SYSEX_STOP  0xF7

#define MS12A_BUFFER_SIZE 128
#define MS12A_RX_BUFFER_SIZE 2048
#define MS12A_NUM_SERVOS 3
#define MS12A_UART_PORT 1
#define MS12A_UART_PIN_TX 17
#define MS12A_UART_PIN_RX 15
#define MS12A_UART_BAUD_RATE 115200

enum 
{
	MS12A_OK = 0x0F,
	MS12A_BUSY = 0x10,
	MS12A_ERROR = 0x11,
	MS12A_WRONG_SERVICE = 0x12,
};

// MESSAGES
typedef struct
ms12a_message_decl
{
	union {
		uint8_t data[MS12A_BUFFER_SIZE];
		struct 
		{
			uint8_t device_id;
			uint8_t servo_id;
			uint8_t body[MS12A_BUFFER_SIZE-2];
		};
	};

} ms12a_message_t;

// SERVO

typedef struct 
ms12a_servo_decl
{
	uint8_t id;
	struct ms12a_driver_decl* driver;
} ms12a_servo_t;


// DRIVER 
typedef struct
ms12a_driver_decl
{
	ms12a_servo_t servo[MS12A_NUM_SERVOS];
	uint8_t rx_buffer[MS12A_BUFFER_SIZE];
	TaskHandle_t task;
	QueueHandle_t queue;
} ms12a_t;


// API
int ms12a_init(ms12a_t* self);
int ms12a_deinit(ms12a_t* self);
int ms12a_handshake(uint8_t servo);
int ms12a_set_angle_tare(uint8_t servo);
int ms12a_set_angle_abs(uint8_t servo, int32_t angle, uint14_t speed);
int ms12a_set_angle_rel(uint8_t servo, int32_t angle, uint14_t speed);
int ms12a_get_angle(uint8_t servo);
int ms12a_set_mode(uint8_t servo, uint7_t mode);
int ms12a_set_position_abs(uint8_t servo, int32_t pos, uint14_t speed);
int ms12a_set_position_rel(uint8_t servo, int32_t pos, uint14_t speed);
int ms12a_return_to_zero(uint8_t servo, uint7_t mode, uint14_t speed);

int ms12a_set_led(uint8_t servo, uint8_t red, uint8_t green, uint8_t blue);
int ms12a_get_voltage(uint8_t servo);

#define ms12a_set_angle ms12a_set_angle_abs
// Internal use
int ms12a_test_7bit();
