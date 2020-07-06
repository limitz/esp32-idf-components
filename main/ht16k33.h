#include <stdio.h>
#include <string.h>
#include "driver/i2c.h"
#include "esp_log.h"

#define ACK_CHECK_ENABLE  0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DISABLE 0x0           /*!< I2C master will not check ack from slave *//
#define ACK_VALUE         0x0                 /*!< I2C ack value */
#define NACK_VALUE        0x1                /*!< I2C nack value */

#ifndef CONFIG_HT16K33_SDA
#define CONFIG_HT16K33_SDA 21
#endif

#ifndef CONFIG_HT16K33_SCL
#define CONFIG_HT16K33_SCL 22
#endif

#ifndef CONFIG_HT16K33_FREQ
#define CONFIG_HT16K33_FREQ 100000
#endif

#ifndef CONFIG_HT16K33_PORT
#define CONFIG_HT16K33_PORT I2C_NUM_0
#endif

#define HT16K33_BLINK_CMD 0x80       ///< I2C register for BLINK setting
#define HT16K33_BLINK_DISPLAYON 0x01 ///< I2C value for steady on
#define HT16K33_BLINK_OFF 0          ///< I2C value for steady off
#define HT16K33_BLINK_2HZ 1          ///< I2C value for 2 Hz blink
#define HT16K33_BLINK_1HZ 2          ///< I2C value for 1 Hz blink
#define HT16K33_BLINK_HALFHZ 3       ///< I2C value for 0.5 Hz blink
#define HT16K33_CMD_BRIGHTNESS 0xE0 ///< I2C register for BRIGHTNESS setting

typedef struct
{
        int i2c_port;
        int i2c_freq;
        int i2c_pin_sda;
        int i2c_pin_scl;

        uint8_t address;
        uint8_t brightness;
        uint8_t blinkmode;
        int number;
        const char* text;
} ht16k33_t;

#define HT16K33_DEFAULT { \
	.i2c_port = CONFIG_HT16K33_PORT, \
	.i2c_freq = CONFIG_HT16K33_FREQ, \
	.i2c_pin_sda = CONFIG_HT16K33_SDA, \
	.i2c_pin_scl = CONFIG_HT16K33_SCL, \
	.address = 0x70, \
	.brightness = 10, \
	.blinkmode = 0, \
	.number = 0xFFFFFFFF, \
	.text = "LMTZ", \
}

esp_err_t ht16k33_init(ht16k33_t* h);
esp_err_t ht16k33_destroy(ht16k33_t* h);
esp_err_t ht16k33_display(ht16k33_t* h);

