#include "ht16k33.h"

ht16k33_t g_ht16k33[16] = {
	{
		#if HT16K33_B00_EN
		.device = { .bus = 0, .addr = 0x70 }, .type = HT16K33_B00_TYPE, .brightness = 8,
		#endif
	},{
		#if HT16K33_B01_EN
		.device = { .bus = 0, .addr = 0x71 }, .type = HT16K33_B01_TYPE, 
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B02_EN
		.device = { .bus = 0, .addr = 0x72 }, .type = HT16K33_B02_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B03_EN
		.device = { .bus = 0, .addr = 0x73 }, .type = HT16K33_B03_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B04_EN
		.device = { .bus = 0, .addr = 0x74 }, .type = HT16K33_B04_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B05_EN
		.device = { .bus = 0, .addr = 0x75 }, .type = HT16K33_B05_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B06_EN
		.device = { .bus = 0, .addr = 0x76 }, .type = HT16K33_B06_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B07_EN
		.device = { .bus = 0, .addr = 0x77 }, .type = HT16K33_B07_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B10_EN
		.device = { .bus = 1, .addr = 0x70 }, .type = HT16K33_B10_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B11_EN
		.device = { .bus = 1, .addr = 0x71 }, .type = HT16K33_B11_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B12_EN
		.device = { .bus = 1, .addr = 0x72 }, .type = HT16K33_B12_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B13_EN
		.device = { .bus = 1, .addr = 0x73 }, .type = HT16K33_B13_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B14_EN
		.device = { .bus = 1, .addr = 0x74 }, .type = HT16K33_B14_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B15_EN
		.device = { .bus = 1, .addr = 0x75 }, .type = HT16K33_B15_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B16_EN
		.device = { .bus = 1, .addr = 0x76 }, .type = HT16K33_B16_TYPE,
		.brightness = 8,
		#endif
	},{
		#if HT16K33_B17_EN
		.device = { .bus = 1, .addr = 0x77 }, .type = HT16K33_B17_TYPE,
		.brightness = 8,
		#endif
	},
};

static const uint16_t numfonttable[] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F, // 9

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hex
	0x40, // -
	0x63, // deg (c)
	0x00, // space
};

static const uint16_t alphafonttable[] = {
    0b0000000000000001, 0b0000000000000010, 0b0000000000000100,
    0b0000000000001000, 0b0000000000010000, 0b0000000000100000,
    0b0000000001000000, 0b0000000010000000, 0b0000000100000000,
    0b0000001000000000, 0b0000010000000000, 0b0000100000000000,
    0b0001000000000000, 0b0010000000000000, 0b0100000000000000,
    0b1000000000000000, 0b0000000000000000, 0b0000000000000000,
    0b0000000000000000, 0b0000000000000000, 0b0000000000000000,
    0b0000000000000000, 0b0000000000000000, 0b0000000000000000,
    0b0001001011001001, 0b0001010111000000, 0b0001001011111001,
    0b0000000011100011, 0b0000010100110000, 0b0001001011001000,
    0b0011101000000000, 0b0001011100000000, 0b0000000000000000,

    0b0000000000000110, // !
    0b0000001000100000, // "
    0b0001001011001110, // #
    0b0001001011101101, // $
    0b0000110000100100, // %
    0b0010001101011101, // &
    0b0000010000000000, // '
    0b0010010000000000, // (
    0b0000100100000000, // )
    0b0011111111000000, // *
    0b0001001011000000, // +
    0b0000100000000000, // ,
    0b0000000011000000, // -
    0b0000000000000000, // .
    0b0000110000000000, // /
    0b0000110000111111, // 0
    0b0000000000000110, // 1
    0b0000000011011011, // 2
    0b0000000010001111, // 3
    0b0000000011100110, // 4
    0b0010000001101001, // 5
    0b0000000011111101, // 6
    0b0000000000000111, // 7
    0b0000000011111111, // 8
    0b0000000011101111, // 9
    0b0001001000000000, // :
    0b0000101000000000, // ;
    0b0010010000000000, // <
    0b0000000011001000, // =
    0b0000100100000000, // >
    0b0001000010000011, // ?
    0b0000001010111011, // @
    0b0000000011110111, // A
    0b0001001010001111, // B
    0b0000000000111001, // C
    0b0001001000001111, // D
    0b0000000011111001, // E
    0b0000000001110001, // F
    0b0000000010111101, // G
    0b0000000011110110, // H
    0b0001001000000000, // I
    0b0000000000011110, // J
    0b0010010001110000, // K
    0b0000000000111000, // L
    0b0000010100110110, // M
    0b0010000100110110, // N
    0b0000000000111111, // O
    0b0000000011110011, // P
    0b0010000000111111, // Q
    0b0010000011110011, // R
    0b0000000011101101, // S
    0b0001001000000001, // T
    0b0000000000111110, // U
    0b0000110000110000, // V
    0b0010100000110110, // W
    0b0010110100000000, // X
    0b0001010100000000, // Y
    0b0000110000001001, // Z
    0b0000000000111001, // [
    0b0010000100000000, //
    0b0000000000001111, // ]
    0b0000110000000011, // ^
    0b0000000000001000, // _
    0b0000000100000000, // `
    0b0001000001011000, // a
    0b0010000001111000, // b
    0b0000000011011000, // c
    0b0000100010001110, // d
    0b0000100001011000, // e
    0b0000000001110001, // f
    0b0000010010001110, // g
    0b0001000001110000, // h
    0b0001000000000000, // i
    0b0000000000001110, // j
    0b0011011000000000, // k
    0b0000000000110000, // l
    0b0001000011010100, // m
    0b0001000001010000, // n
    0b0000000011011100, // o
    0b0000000101110000, // p
    0b0000010010000110, // q
    0b0000000001010000, // r
    0b0010000010001000, // s
    0b0000000001111000, // t
    0b0000000000011100, // u
    0b0010000000000100, // v
    0b0010100000010100, // w
    0b0010100011000000, // x
    0b0010000000001100, // y
    0b0000100001001000, // z
    0b0000100101001001, // {
    0b0001001000000000, // |
    0b0010010010001001, // }
    0b0000010100100000, // ~
    0b0011111111111111
};

int ht16k33_init(ht16k33_t* self)
{
	return ESP_OK; 
}

int ht16k33_deinit(ht16k33_t* self)
{
	return ESP_OK;
}

int ht16k33_update(ht16k33_t* self)
{
	int err, ofs;
	char str[18] = {0};

	if (0x70 > self->device.addr || 0x78 <= self->device.addr)
	{
		return ESP_FAIL;
	}

	ESP_LOGE(__func__, "writing to %d, %x", self->device.bus, self->device.addr);

	err = i2c_send_cmd(&self->dev, 0x21);
	if (ESP_OK != err) return err;

	err = i2c_send_cmd(&self->dev, HT16K33_CMD_BRIGHTNESS | (self->brightness & 0x0F));
	if (ESP_OK != err) return err;

	err = i2c_send_cmd(&self->dev, HT16K33_BLINK_CMD|HT16K33_DISPLAYON|(((self->blinkmode & 0x03) << 1)));
	if (ESP_OK != err) return err;


	switch (self->content)
	{
	case HT16K33_CONTENT_INT:	snprintf(str, 18, "%4d", self->intval % 10000); break;
	case HT16K33_CONTENT_INT_HEX:	snprintf(str, 18, "%04X", self->intval & 0xFFFF); break;
	//TODO: case HT16K33_CONTENT_TIME:	snprintf(str, 18, "%04X", self->intval & 0xFFFF); break;
	case HT16K33_CONTENT_DEGREES:	snprintf(str, 18, "%3dc", self->intval % 1000); break;

	case HT16K33_CONTENT_FLOAT:	snprintf(str, 18, "%f", self->floatval); break;
	case HT16K33_CONTENT_STRING:	snprintf(str, 18, "%s", self->stringval); break;
	default: break;
	}

	switch (self->type)
	{
	case HT16K33_TYPE_NUMERIC_X4:
	case HT16K33_TYPE_NUMERIC_X4L:
		ofs = -1;
		for (int i=4; i>=0; i--)
		{
			if (i == 2) str[i] = ofs = 0x00;
			else
			{
				int key = (int)str[i + ofs];
				switch (key)
				{
					case '-': key = 16; break;
					case 'c': key = 17; break;
					case ' ': key = 18; break;
					default: key = key - '0'; break;
				}
				uint16_t mask = numfonttable[key];
				str[(i<<1)+1] = mask & 0xFF;
				str[(i<<1)+2] = mask >> 8;
			}
		}
		ofs = 11;
		break;

	case HT16K33_TYPE_ALPHANUMERIC_X4:
		ofs = 0;
		for (int i=3; i>=0; i--)
		{
			uint16_t mask = alphafonttable[(int)str[i + ofs]];
			str[(i<<1)+1] = mask & 0xFF;
			str[(i<<1)+2] = mask >> 8;
		}
		ofs = 9;
		break;	
	default:
		ESP_LOGE(__func__, "Not implemented");
		return ESP_FAIL;
	}
	
	str[0] = 0x00;
	ESP_LOG_BUFFER_HEX(__func__, str, ofs);
	i2c_send_data(&self->device, -1, str, ofs);
	
	return ESP_OK;
}
