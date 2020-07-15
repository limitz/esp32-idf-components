#pragma once

// Makeblock MS-12A smart servo (serial control)

enum 
{
	MS12A_SET_PID = 0x10,
	MS12A_SET_POS_ABS = 0x11,
	MS12A_SET_POS_REL = 0x12,
	MS12A_SET_CONTINUOUS = 0x13,
	MS12A_SET_MOTION_COMP = 0x14,
	MS12A_CLR_MOTION_COMP = 0x15,
	MS12A_SET_BREAK = 0x16,
	MS12A_SET_RGB_LED = 0x17,
	MS12A_HANDSHAKE = 0x18,
	MS12A_SET_CMD_MODE = 0x19,

	MS12A_GET_STATUS = 0x20,
	MS12A_GET_PID = 0x21,
	MS12A_GET_POS = 0x22,
	MS12A_GET_SPEED = 0x23,
	MS12A_GET_MOTION_COMP = 0x24,
	MS12A_GET_TEMPERATURE = 0x25,
	MS12A_GET_CURRENT = 0x26,
	MS12A_GET_VOLTAGE = 0x27,

	MS12A_RESET_ANGLE = 0x30,
	MS12A_SET_ANGLE_ABS = 0x31,
	MS12A_SET_ANGLE_REL = 0x32,
	MS12A_SET_ANGLE_ABS_LONG = 0x33,
	MS12A_SET_ANGLE_REL_LONG = 0x34,
	MS12A_SET_SPEED_PWM = 0x35,
	MS12A_GET_ANGLE = 0x36,
	MS12A_INIT_ANGLE = 0x37,
	MS12A_REPORT_POS = 0x40,

} ms12a_reg_t;

enum
{
	MS12A_OK = 0x0F,
	MS12A_BUSY = 0x10,
	MS12A_ERROR = 0x11,
	MS12A_WRONG_SERVICE = 0x12,

} ms12a_error_t;

typedef struct 
{
	int pin_tx;
	int pin_rx;
	int pid;

	float angle;
	float speed;
	float voltage;
	float current;
	float temperature;
} ms12a_t;

void ms12a_init(ms12_a**);
void ms12a_destroy(ms12_a*);

void ms12a_reset_angle(ms12a_t*);
void ms12a_set_position_abs(ms12a_t*, int);
void ms12a_set_position_rel(ms12a_t*, int);
void ms12a_set_continuous(ms12a_t*, bool);
void ms12a_set_motion_comp(ms12a_t*, bool);
void ms12a_set_break(ms12a_t*, bool);
void ms12a_set_led(ms12a_t*, int rgb);
void ms12a_set_command_mode(ms12a_t*, int);

void ms12a_set_angle_abs(ms12a_t*, int);
void ms12a_set_angle_rel(ms12a_t*, int);
void ms12a_set_pwm_move(ms12a_t*, int);
void ms12a_set_angle_init(ms12a_t*, int);

void ms12a_report_at(ms12a_t*, int);

void ms12a_get_temperature(ms12a_t*);
void ms12a_get_pid(ms12a_t*);
void ms12a_

void ms12a_handshake(ms12a_t*);
void ms12a_identify(ms12a_t*);



