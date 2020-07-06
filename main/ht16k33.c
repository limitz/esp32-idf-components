#include "ht16k33.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const uint8_t numbertable[] = {
	0x3F, /* 0 */
	0x06, /* 1 */
	0x5B, /* 2 */
    	0x4F, /* 3 */
	0x66, /* 4 */
	0x6D, /* 5 */
	0x7D, /* 6 */
	0x07, /* 7 */
	0x7F, /* 8 */
	0x6F, /* 9 */
	0x77, /* a */
	0x7C, /* b */
	0x39, /* C */
	0x5E, /* d */
	0x79, /* E */
	0x71, /* F */
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

esp_err_t ht16k33_init(ht16k33_t* h)
{
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = h->i2c_pin_sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE, // needs external
        .scl_io_num = h->i2c_pin_scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE, // needs external
        .master.clk_speed = h->i2c_freq
    };

    ESP_ERROR_CHECK(i2c_param_config(h->i2c_port, &cfg));
    ESP_ERROR_CHECK(i2c_driver_install(h->i2c_port, I2C_MODE_MASTER, 0, 0, 0));
    return ESP_OK;
}

esp_err_t ht16k33_destroy(ht16k33_t* h)
{
	ESP_ERROR_CHECK(i2c_driver_delete(h->i2c_port));
	return ESP_OK;
}

esp_err_t ht16k33_display(ht16k33_t* h)
{
	if (h->address)
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
 		i2c_master_write_byte(cmd, 0x21, ACK_CHECK_ENABLE);
		i2c_master_stop(cmd);
 
		esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, 1000 / portTICK_RATE_MS);
       		i2c_cmd_link_delete(cmd);
		ESP_ERROR_CHECK(ret);
	}
	else return ESP_FAIL;

	/* Check brightness: "-b" option */
	if (h->brightness < 16) 
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
 		i2c_master_write_byte(cmd, HT16K33_CMD_BRIGHTNESS | h->brightness, ACK_CHECK_ENABLE);
		i2c_master_stop(cmd);
 
		esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, 1000 / portTICK_RATE_MS);
       		i2c_cmd_link_delete(cmd);
		ESP_ERROR_CHECK(ret);
	}
   
	if (h->blinkmode  < 4) 
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
 		i2c_master_write_byte(cmd, 
				HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (h->blinkmode << 1), 
				ACK_CHECK_ENABLE);
		i2c_master_stop(cmd);
 
		esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, 1000 / portTICK_RATE_MS);
       		i2c_cmd_link_delete(cmd);
		ESP_ERROR_CHECK(ret);
	}

	if (!h->text)
	{
		int b = h->number;	
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
 		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, numbertable[(b/1000) % 10], ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, numbertable[(b/100) % 10], ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, numbertable[(b/10) % 10], ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, numbertable[(b/1) % 10], ACK_CHECK_ENABLE);
		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		i2c_master_stop(cmd);
 
		esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, 1000 / portTICK_RATE_MS);
       		i2c_cmd_link_delete(cmd);
		ESP_ERROR_CHECK(ret);
	}

	if (h->text)
	{
		const char* b = h->text;
		
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (h->address << 1) | I2C_MASTER_WRITE, ACK_CHECK_ENABLE);
 		i2c_master_write_byte(cmd, 0x00, ACK_CHECK_ENABLE);
		
		for (int i=0; i<4; i++)
		{
			uint16_t bitmask =  strlen(b) > i ? alphafonttable[(int) b[i]] : 0x00;
			
			i2c_master_write_byte(cmd, bitmask & 0xFF, ACK_CHECK_ENABLE);
			i2c_master_write_byte(cmd, bitmask >> 8, ACK_CHECK_ENABLE);
		}
		i2c_master_stop(cmd);
 
		esp_err_t ret = i2c_master_cmd_begin(h->i2c_port, cmd, 1000 / portTICK_RATE_MS);
       		i2c_cmd_link_delete(cmd);
		ESP_ERROR_CHECK(ret);
	}
	return ESP_OK;
}

