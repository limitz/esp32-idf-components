#pragma once

#include <stdint.h>
#include <i2cbase.h>

#define TRACKBALL_VALID_MAGIC 0x3a6fb67c

// TODO put somewhere else, like general math lib
typedef struct
{
	int x, y;
} point_t;

enum
{
	LED_F1,      LED_MODE_FLASH_1 = 0x00, 
	LED_F2,      LED_MODE_FLASH_2 = 0x01,
	LED_FSW,     LED_MODE_FLASH_TOGGLE = 0x02,
	LED_FALL,    LED_MODE_FLASH_ALL = 0x03,
	LED_ON,      LED_MODE_ALWAYS_ON = 0x04,
	LED_OFF,     LED_MODE_ALWAYS_OFF = 0x05,
	LED_B1,      LED_MODE_BREATHING_1 = 0x06,
	LED_B2,      LED_MODE_BREATHING_2 = 0x07,
	LED_BALL,    LED_MODE_BREATHING_ALL = 0x08,
	LED_MOVE,    LED_MODE_MOVE_FLASH = 0x09,

};

enum
{
	R_UP,		TRACKBALL_REG_UP 		= 0x00, 
	R_DOWN, 	TRACKBALL_REG_DOWN 		= 0x01,
	R_LEFT, 	TRACKBALL_REG_LEFT 		= 0x02,
	R_RIGHT,	TRACKBALL_REG_RIGHT 		= 0x03,
	R_CLICK,	TRACKBALL_REG_CLICK 		= 0x04,
	R_VALID, 	TRACKBALL_REG_VALID 		= 0x05, 
		        TRACKBALL_REG_VALID_LWHB	= 0x06, 
			TRACKBALL_REG_VALID_HWLB	= 0x07,
			TRACKBALL_REG_VALID_HWHB	= 0x08,

	R_ADDR,		TRACKBALL_REG_ADDR 		= 0x09,
	R_SPEED,	TRACKBALL_REG_SPEED 		= 0x0A,
	R_SPEED,	TRACKBALL_REG_SPEED_HB 		= 0x0B,
	R_LED,		TRACKBALL_REG_LEDMODE 		= 0x0C,
	R_TFLASH,	TRACKBALL_REG_TIME_FLASH 	= 0x0D,
	R_TFLASH,	TRACKBALL_REG_TIME_FLASH_HB 	= 0x0E,
	R_TCLEAR,	TRACKBALL_REG_TIME_CLEAR 	= 0x0F,
			TRACKBALL_REG_TIME_CLEAR_HB	= 0x10,
	R_TREAD,	TRACKBALL_REG_TIME_READ 	= 0x11,
			TRACKBALL_REG_TIME_READ_HB	= 0x12,
	R_SIZEOF
};

typedef struct {

	union {
		uint8_t  u8;
		uint16_t u16;
		uint32_t u32;

		int8_t  s8;
		int16_t s16;
		int32_t s32;
	};
	uint8_t addr;
	uint8_t size;
	uint8_t mode;

} reg_ptr_t;

typedef struct
{
	i2cendpoint_t endpoint;
	
	union 
	{
		point_t position;
		point_t pos;
	};

	uint8_t _mem[R_SIZEOF];
	inline reg_ptr_t reg(int r) { return (reg_ptr_t) _mem[r]; }

} trackball_t;


extern trackball_t TRACKBALL;
