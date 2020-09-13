/////////////////////////////////////////////////////////////////
/*
MIT License

Copyright (c) 2019 lewis he

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

axp20x.cpp - Arduino library for X-Power AXP202 chip.
Created by Lewis he on April 1, 2019.
github:https://github.com/lewisxhe/AXP202X_Libraries
*/
/////////////////////////////////////////////////////////////////

#include "axp202.h"
#include <math.h>

const uint8_t startupParams[] = {
    0b00000000,
    0b01000000,
    0b10000000,
    0b11000000
};

const uint8_t longPressParams[] = {
    0b00000000,
    0b00010000,
    0b00100000,
    0b00110000
};

const uint8_t shutdownParams[] = {
    0b00000000,
    0b00000001,
    0b00000010,
    0b00000011
};

const uint8_t targetVolParams[] = {
    0b00000000,
    0b00100000,
    0b01000000,
    0b01100000
};





// Power Output Control register
static uint8_t _outputReg;
static uint8_t _chip_id;
static uint8_t _irq[64];
const  uint8_t _i2cport = 0;
const uint8_t _address = 0x35;

axp202_t AXP202 = {
	.i2c = {
		.bus = 0,
		.addr = 0x35,
	},
};

static int _readByte(uint8_t reg, uint16_t len, uint8_t* data)
{
	return i2c_read(&AXP202.i2c, reg, data, len);
}

static int _writeByte(uint8_t reg, uint16_t len, uint8_t* data)
{
	return i2c_write(&AXP202.i2c, reg, data, len);
}

static int _getRegistResult(uint8_t h8, uint8_t l4)
{
	uint8_t u8, u4;
	i2c_read8(&AXP202.i2c, h8, &u8);
	i2c_read8(&AXP202.i2c, l4, &u4);
	return (u8<<4) | (u4 & 0xF);
}
static int _getRegistH8L5(uint8_t h8, uint8_t l5)
{
	uint8_t u8, u5;
	i2c_read8(&AXP202.i2c, h8, &u8);
	i2c_read8(&AXP202.i2c, l5, &u5);
	return (u8<<5) | (u5 & 0x1F);
}
	

float axp202_get_vbus_voltage()
{
    return _getRegistResult(AXP202_VBUS_VOL_H8, AXP202_VBUS_VOL_L4) * AXP202_VBUS_VOLTAGE_STEP;
}

float axp202_get_vbus_current()
{
    return _getRegistResult(AXP202_VBUS_CUR_H8, AXP202_VBUS_CUR_L4) * AXP202_VBUS_CUR_STEP;
}

int axp202_get_temp()
{
    return _getRegistResult(AXP202_INTERNAL_TEMP_H8, AXP202_INTERNAL_TEMP_L4) * AXP202_INTERNAL_TEMP_STEP;
}

uint8_t axp202_get_adc_sampling_rate()
{
    //axp192 same axp202 aregister address 0x84
    uint8_t val;
    _readByte(AXP202_ADC_SPEED, 1, &val);
    return 25 * (int)pow(2, (val & 0xC0) >> 6);
}

int axp202_set_adc_sampling_rate(axp_adc_sampling_rate_t rate)
{
    //axp192 same axp202 aregister address 0x84
    if (rate > AXP_ADC_SAMPLING_RATE_200HZ)
        return AXP_FAIL;
    uint8_t val;
    _readByte(AXP202_ADC_SPEED, 1, &val);
    uint8_t rw = rate;
    val &= 0x3F;
    val |= (rw << 6);
    _writeByte(AXP202_ADC_SPEED, 1, &val);
    return AXP_PASS;
}

static int _axp_probe()
{
    _readByte(AXP202_IC_TYPE, 1, &_chip_id);
    AXP_DEBUG("chip id detect 0x%x\n", _chip_id);
    if (_chip_id == AXP202_CHIP_ID || _chip_id == AXP192_CHIP_ID) {
        AXP_DEBUG("Detect CHIP :%s\n", _chip_id == AXP202_CHIP_ID ? "AXP202" : "AXP192");
        _readByte(AXP202_LDO234_DC23_CTL, 1, &_outputReg);
        AXP_DEBUG("OUTPUT Register 0x%x\n", _outputReg);
        return AXP_PASS;
    }
    return AXP_FAIL;
}
int axp202_init()
{
    return _axp_probe();
}

int axp202_set_output(uint8_t ch, bool en)
{
    uint8_t data;
    uint8_t val = 0;
    
        

    _readByte(AXP202_LDO234_DC23_CTL, 1, &data);
    if (en) data |= (1 << ch);
    else data &= (~(1 << ch));

    if (_chip_id == AXP202_CHIP_ID) {
        FORCED_OPEN_DCDC3(data); //! Must be forced open in T-Watch
    }

    _writeByte(AXP202_LDO234_DC23_CTL, 1, &data);
    vTaskDelay(1);
    _readByte(AXP202_LDO234_DC23_CTL, 1, &val);
    if (data == val) {
        _outputReg = val;
        return AXP_PASS;
    }
    return AXP_FAIL;
}

/*
Note: the battery power formula:
Pbat =2* register value * Voltage LSB * Current LSB / 1000.
(Voltage LSB is 1.1mV; Current LSB is 0.5mA, and unit of calculation result is mW.)
*/
int axp202_get_batt_in_power()
{
    int rslt;
    uint8_t hv, mv, lv;
    _readByte(AXP202_BAT_POWERH8, 1, &hv);
    _readByte(AXP202_BAT_POWERM8, 1, &mv);
    _readByte(AXP202_BAT_POWERL8, 1, &lv);
    rslt = (hv << 16) | (mv << 8) | lv;
    rslt = rslt + rslt / 10;
    return rslt;
}

int axp202_get_batt_voltage()
{
    return _getRegistResult(AXP202_BAT_AVERVOL_H8, AXP202_BAT_AVERVOL_L4) * AXP202_BATT_VOLTAGE_STEP;
}

int axp202_get_batt_charge_current()
{
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        return _getRegistResult(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L4) * AXP202_BATT_CHARGE_CUR_STEP;
    case AXP192_CHIP_ID:
        return _getRegistH8L5(AXP202_BAT_AVERCHGCUR_H8, AXP202_BAT_AVERCHGCUR_L5) * AXP202_BATT_CHARGE_CUR_STEP;
    default:
        return AXP_FAIL;
    }
}

float axp202_get_batt_discharge_current()
{
    return _getRegistH8L5(AXP202_BAT_AVERDISCHGCUR_H8, AXP202_BAT_AVERDISCHGCUR_L5) * AXP202_BATT_DISCHARGE_CUR_STEP;
}

float axp202_get_sys_IPSOUT_voltage()
{
    return _getRegistResult(AXP202_APS_AVERVOL_H8, AXP202_APS_AVERVOL_L4);
}

int axp202_update()
{
	uint8_t charge1;
	uint8_t percentage;
	uint8_t status1, status2;
	_readByte(AXP202_MODE_CHGSTATUS, 1, &status1);
	_readByte(AXP202_CHARGE1, 1, &charge1);
	_readByte(AXP202_BATT_PERCENTAGE, 1, &percentage);
	_readByte(AXP202_STATUS, 1, &status2);

	AXP202.settings.target_voltage = charge1 & 0x60;
  	AXP202.settings.charging_current = 300 + 100*(charge1 & 0x7);
	AXP202.settings.charging_enabled = charge1 & (1<<7);

	AXP202.battery.percentage = percentage & 0x7F;
	AXP202.battery.voltage = axp202_get_batt_voltage();
	AXP202.battery.current = axp202_get_batt_discharge_current();
	AXP202.battery.is_charging    = 0 == (status1 & (1<<6));
	AXP202.battery.is_connected = 0 == (status1 & (1<<5));

	AXP202.vbus.is_connected = 0 == (status2 & (1<<5));
	AXP202.vbus.voltage = axp202_get_vbus_voltage();
	AXP202.vbus.current = axp202_get_vbus_current();
	
	AXP202.temperature = axp202_get_temp();
	return ESP_OK;
}

float axp202_get_acin_voltage()
{
    return _getRegistResult(AXP202_ACIN_VOL_H8, AXP202_ACIN_VOL_L4) * AXP202_ACIN_VOLTAGE_STEP;
}

float axp202_get_acin_current()
{
    return _getRegistResult(AXP202_ACIN_CUR_H8, AXP202_ACIN_CUR_L4) * AXP202_ACIN_CUR_STEP;
}


/*
Coulomb calculation formula:
C= 65536 * current LSB *（charge coulomb counter value - discharge coulomb counter value） /
3600 / ADC sample rate. Refer to REG84H setting for ADC sample rate；the current LSB is
0.5mA；unit of the calculation result is mAh. ）
*/
uint32_t axp202_get_batt_charge_coulomb()
{
    uint8_t buffer[4];
    _readByte(0xB0, 4, buffer);
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

uint32_t axp202_get_batt_discharge_coulomb()
{
    uint8_t buffer[4];
    _readByte(0xB4, 4, buffer);
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
}

float axp202_get_coulomb_data()
{
    uint32_t charge = axp202_get_batt_charge_coulomb(), discharge = axp202_get_batt_discharge_coulomb();
    uint8_t rate = axp202_get_adc_sampling_rate();
    float result = 65536.0 * 0.5 * (charge - discharge) / 3600.0 / rate;
    return result;
}


//-------------------------------------------------------
// New Coulomb functions  by MrFlexi
//-------------------------------------------------------

uint8_t axp202_get_coulomb_register()
{
    uint8_t buffer;
    _readByte(AXP202_COULOMB_CTL, 1, &buffer);
    return buffer;
}


int axp202_set_coulomb_register(uint8_t val)
{
    _writeByte(AXP202_COULOMB_CTL, 1, &val);
    return AXP_PASS;
}


int axp202_enable_coulomb_counter(void)
{
   
     uint8_t val = 0x80;    
    _writeByte(AXP202_COULOMB_CTL, 1, &val);
    return AXP_PASS;    
}

int axp202_disable_coulomb_counter(void)
{
   
     uint8_t val = 0x00;    
    _writeByte(AXP202_COULOMB_CTL, 1, &val);
    return AXP_PASS;    
}

int axp202_stop_coulomb_counter(void)
{
   
     uint8_t val = 0xB8;    
    _writeByte(AXP202_COULOMB_CTL, 1, &val);
    return AXP_PASS;    
}


int axp202_clear_coulomb_counter(void)
{
     uint8_t val = 0xA0;    
    _writeByte(AXP202_COULOMB_CTL, 1, &val);
    return AXP_PASS;    
}

//-------------------------------------------------------
// END 
//-------------------------------------------------------

int axp202_adc1_enable(uint16_t params, bool en)
{
    
        
    uint8_t val;
    _readByte(AXP202_ADC_EN1, 1, &val);
    if (en)
        val |= params;
    else
        val &= ~(params);
    _writeByte(AXP202_ADC_EN1, 1, &val);
    return AXP_PASS;
}

int axp202_adc2_enable(uint16_t params, bool en)
{
    uint8_t val;
    _readByte(AXP202_ADC_EN2, 1, &val);
    if (en)
        val |= params;
    else
        val &= ~(params);
    _writeByte(AXP202_ADC_EN2, 1, &val);
    return AXP_PASS;
}


int axp202_set_TS_function(axp_ts_pin_function_t func)
{
    //axp192 same axp202 aregister address 0x84
    if (func > AXP_TS_PIN_FUNCTION_ADC)
        return AXP_FAIL;
    uint8_t val;
    _readByte(AXP202_ADC_SPEED, 1, &val);
    uint8_t rw = func;
    val &= 0xFA;
    val |= (rw << 2);
    _writeByte(AXP202_ADC_SPEED, 1, &val);
    return AXP_PASS;
}

int axp202_set_TS_current(axp_ts_pin_current_t current)
{
    //axp192 same axp202 aregister address 0x84
    if (current > AXP_TS_PIN_CURRENT_80UA)
        return AXP_FAIL;
    uint8_t val;
    _readByte(AXP202_ADC_SPEED, 1, &val);
    uint8_t rw = current;
    val &= 0xCF;
    val |= (rw << 4);
    _writeByte(AXP202_ADC_SPEED, 1, &val);
    return AXP_PASS;
}

int axp202_set_TS_mode(axp_ts_pin_mode_t mode)
{
    //axp192 same axp202 aregister address 0x84

    if (mode > AXP_TS_PIN_MODE_ENABLE)
        return AXP_FAIL;
    uint8_t val;
    _readByte(AXP202_ADC_SPEED, 1, &val);
    uint8_t rw = mode;
    val &= 0xFC;
    val |= rw;
    _writeByte(AXP202_ADC_SPEED, 1, &val);

    // TS pin ADC function enable/disable
    if (mode == AXP_TS_PIN_MODE_DISABLE)
        axp202_adc1_enable(AXP202_TS_PIN_ADC1, false);
    else
        axp202_adc1_enable(AXP202_TS_PIN_ADC1, true);
    return AXP_PASS;
}

int axp202_enable_IRQ(uint64_t params, bool en)
{
    uint8_t val, val1;
    if (params & 0xFF) {
        val1 = params & 0xFF;
        _readByte(AXP202_INTEN1, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN1, val);
        _writeByte(AXP202_INTEN1, 1, &val);
    }
    if (params & 0xFF00) {
        val1 = params >> 8;
        _readByte(AXP202_INTEN2, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN2, val);
        _writeByte(AXP202_INTEN2, 1, &val);
    }

    if (params & 0xFF0000) {
        val1 = params >> 16;
        _readByte(AXP202_INTEN3, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN3, val);
        _writeByte(AXP202_INTEN3, 1, &val);
    }

    if (params & 0xFF000000) {
        val1 = params >> 24;
        _readByte(AXP202_INTEN4, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN4, val);
        _writeByte(AXP202_INTEN4, 1, &val);
    }

    if (params & 0xFF00000000) {
        val1 = params >> 32;
        _readByte(AXP202_INTEN5, 1, &val);
        if (en)
            val |= val1;
        else
            val &= ~(val1);
        AXP_DEBUG("%s [0x%x]val:0x%x\n", en ? "enable" : "disable", AXP202_INTEN5, val);
        _writeByte(AXP202_INTEN5, 1, &val);
    }
    return AXP_PASS;
}

int axp202_read_IRQ()
{
    switch (_chip_id) {
    case AXP192_CHIP_ID:
        for (int i = 0; i < 4; ++i) {
            _readByte(AXP192_INTSTS1 + i, 1, &_irq[i]);
        }
        _readByte(AXP192_INTSTS5, 1, &_irq[4]);
        return AXP_PASS;

    case AXP202_CHIP_ID:
        for (int i = 0; i < 5; ++i) {
            _readByte(AXP202_INTSTS1 + i, 1, &_irq[i]);
        }
        return AXP_PASS;
    default:
        return AXP_FAIL;
    }
}

void axp_clear_IRQ()
{
    uint8_t val = 0xFF;
    switch (_chip_id) {
    case AXP192_CHIP_ID:
        for (int i = 0; i < 3; i++) {
            _writeByte(AXP192_INTSTS1 + i, 1, &val);
        }
        _writeByte(AXP192_INTSTS5, 1, &val);
        break;
    case AXP202_CHIP_ID:
        for (int i = 0; i < 5; i++) {
            _writeByte(AXP202_INTSTS1 + i, 1, &val);
        }
        break;
    default:
        break;
    }
    memset(_irq, 0, sizeof(_irq));
}

/*
bool axp202_is_acin_OverVoltageIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(7));
}

bool isAcinPlugInIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(6));
}

bool isAcinRemoveIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(5));
}

bool isVbusOverVoltageIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(4));
}

bool isVbusPlugInIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(3));
}

bool isVbusRemoveIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(2));
}

bool isVbusLowVHOLDIRQ()
{
    return (bool)(_irq[0] & BIT_MASK(1));
}

bool isBattPlugInIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(7));
}
bool isBattRemoveIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(6));
}
bool isBattEnterActivateIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(5));
}
bool isBattExitActivateIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(4));
}
bool isChargingIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(3));
}
bool isChargingDoneIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(2));
}
bool isBattTempLowIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(1));
}
bool isBattTempHighIRQ()
{
    return (bool)(_irq[1] & BIT_MASK(0));
}

bool isPEKShortPressIRQ()
{
    return (bool)(_irq[2] & BIT_MASK(1));
}

bool isPEKLongtPressIRQ()
{
    return (bool)(_irq[2] & BIT_MASK(0));
}

bool isTimerTimeoutIRQ()
{
    return (bool)(_irq[4] & BIT_MASK(7));
}
*/

/*
int setDCDC2Voltage(uint16_t mv)
{
    
        
    if (mv < 700) {
        AXP_DEBUG("DCDC2:Below settable voltage:700mV~2275mV");
        mv = 700;
    }
    if (mv > 2275) {
        AXP_DEBUG("DCDC2:Above settable voltage:700mV~2275mV");
        mv = 2275;
    }
    uint8_t val = (mv - 700) / 25;
    //! axp173/192/202 same register
    _writeByte(AXP202_DC2OUT_VOL, 1, &val);
    return AXP_PASS;
}

uint16_t getDCDC2Voltage()
{
    uint8_t val = 0;
    //! axp173/192/202 same register
    _readByte(AXP202_DC2OUT_VOL, 1, &val);
    return val * 25 + 700;
}

uint16_t getDCDC3Voltage()
{
    
        return 0;
    if (_chip_id == AXP173_CHIP_ID)return AXP_NOT_SUPPORT;
    uint8_t val = 0;
    _readByte(AXP202_DC3OUT_VOL, 1, &val);
    return val * 25 + 700;
}

int setDCDC3Voltage(uint16_t mv)
{
    
        
    if (_chip_id == AXP173_CHIP_ID)return AXP_NOT_SUPPORT;
    if (mv < 700) {
        AXP_DEBUG("DCDC3:Below settable voltage:700mV~3500mV");
        mv = 700;
    }
    if (mv > 3500) {
        AXP_DEBUG("DCDC3:Above settable voltage:700mV~3500mV");
        mv = 3500;
    }
    uint8_t val = (mv - 700) / 25;
    _writeByte(AXP202_DC3OUT_VOL, 1, &val);
    return AXP_PASS;
}

int setLDO2Voltage(uint16_t mv)
{
    uint8_t rVal, wVal;
    
        
    if (mv < 1800) {
        AXP_DEBUG("LDO2:Below settable voltage:1800mV~3300mV");
        mv = 1800;
    }
    if (mv > 3300) {
        AXP_DEBUG("LDO2:Above settable voltage:1800mV~3300mV");
        mv = 3300;
    }
    wVal = (mv - 1800) / 100;
    if (_chip_id == AXP202_CHIP_ID) {
        _readByte(AXP202_LDO24OUT_VOL, 1, &rVal);
        rVal &= 0x0F;
        rVal |= (wVal << 4);
        _writeByte(AXP202_LDO24OUT_VOL, 1, &rVal);
        return AXP_PASS;
    } else if (_chip_id == AXP192_CHIP_ID || _chip_id == AXP173_CHIP_ID) {
        _readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        rVal &= 0x0F;
        rVal |= (wVal << 4);
        _writeByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        return AXP_PASS;
    }
    return AXP_FAIL;
}

uint16_t getLDO2Voltage()
{
    uint8_t rVal;
    if (_chip_id == AXP202_CHIP_ID) {
        _readByte(AXP202_LDO24OUT_VOL, 1, &rVal);
        rVal &= 0xF0;
        rVal >>= 4;
        return rVal * 100 + 1800;
    } else if (_chip_id == AXP192_CHIP_ID || _chip_id == AXP173_CHIP_ID ) {
        _readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        AXP_DEBUG("get result:%x\n", rVal);
        rVal &= 0xF0;
        rVal >>= 4;
        return rVal * 100 + 1800;
    }
    return 0;
}

int setLDO3Voltage(uint16_t mv)
{
    uint8_t rVal;
    
        
    if (_chip_id == AXP202_CHIP_ID && mv < 700) {
        AXP_DEBUG("LDO3:Below settable voltage:700mV~3500mV");
        mv = 700;
    } else if (_chip_id == AXP192_CHIP_ID && mv < 1800) {
        AXP_DEBUG("LDO3:Below settable voltage:1800mV~3300mV");
        mv = 1800;
    }

    if (_chip_id == AXP202_CHIP_ID && mv > 3500) {
        AXP_DEBUG("LDO3:Above settable voltage:700mV~3500mV");
        mv = 3500;
    } else if (_chip_id == AXP192_CHIP_ID && mv > 3300) {
        AXP_DEBUG("LDO3:Above settable voltage:1800mV~3300mV");
        mv = 3300;
    }

    if (_chip_id == AXP202_CHIP_ID) {
        _readByte(AXP202_LDO3OUT_VOL, 1, &rVal);
        rVal &= 0x80;
        rVal |= ((mv - 700) / 25);
        _writeByte(AXP202_LDO3OUT_VOL, 1, &rVal);
        return AXP_PASS;
    } else if (_chip_id == AXP192_CHIP_ID || _chip_id == AXP173_CHIP_ID) {
        _readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        rVal &= 0xF0;
        rVal |= ((mv - 1800) / 100);
        _writeByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        return AXP_PASS;
    }
    return AXP_FAIL;
}

uint16_t getLDO3Voltage()
{
    uint8_t rVal;
    
        

    if (_chip_id == AXP202_CHIP_ID) {
        _readByte(AXP202_LDO3OUT_VOL, 1, &rVal);
        if (rVal & 0x80) {
            //! According to the hardware N_VBUSEN Pin selection
            return getVbusVoltage() * 1000;
        } else {
            return (rVal & 0x7F) * 25 + 700;
        }
    } else if (_chip_id == AXP192_CHIP_ID || _chip_id == AXP173_CHIP_ID) {
        _readByte(AXP192_LDO23OUT_VOL, 1, &rVal);
        rVal &= 0x0F;
        return rVal * 100 + 1800;
    }
    return 0;
}

//! Only axp173 support
int setLDO4Voltage(uint16_t mv)
{
    
        
    if (_chip_id != AXP173_CHIP_ID)
        return AXP_FAIL;

    if (mv < 700) {
        AXP_DEBUG("LDO4:Below settable voltage:700mV~3500mV");
        mv = 700;
    }
    if (mv > 3500) {
        AXP_DEBUG("LDO4:Above settable voltage:700mV~3500mV");
        mv = 3500;
    }
    uint8_t val = (mv - 700) / 25;
    _writeByte(AXP173_LDO4_VLOTAGE, 1, &val);
    return AXP_PASS;
}

uint16_t getLDO4Voltage()
{
    const uint16_t ldo4_table[] = {1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2500, 2700, 2800, 3000, 3100, 3200, 3300};
    
        return 0;
    uint8_t val = 0;
    switch (_chip_id) {
    case AXP173_CHIP_ID:
        _readByte(AXP173_LDO4_VLOTAGE, 1, &val);
        return val * 25 + 700;
    case AXP202_CHIP_ID:
        _readByte(AXP202_LDO24OUT_VOL, 1, &val);
        val &= 0xF;
        return ldo4_table[val];
        break;
    case AXP192_CHIP_ID:
    default:
        break;
    }
    return 0;
}


//! Only axp202 support
int setLDO4Voltage(axp_ldo4_table_t param)
{
    
        
    if (_chip_id == AXP202_CHIP_ID) {
        if (param >= AXP202_LDO4_MAX)
            return AXP_INVALID;
        uint8_t val;
        _readByte(AXP202_LDO24OUT_VOL, 1, &val);
        val &= 0xF0;
        val |= param;
        _writeByte(AXP202_LDO24OUT_VOL, 1, &val);
        return AXP_PASS;
    }
    return AXP_FAIL;
}

//! Only AXP202 support
// 0 : LDO  1 : DCIN
int setLDO3Mode(uint8_t mode)
{
    uint8_t val;
    if (_chip_id != AXP202_CHIP_ID)
        return AXP_FAIL;
    _readByte(AXP202_LDO3OUT_VOL, 1, &val);
    if (mode) {
        val |= BIT_MASK(7);
    } else {
        val &= (~BIT_MASK(7));
    }
    _writeByte(AXP202_LDO3OUT_VOL, 1, &val);
    return AXP_PASS;
}

int setStartupTime(uint8_t param)
{
    uint8_t val;
    
        
    if (param > sizeof(startupParams) / sizeof(startupParams[0]))
        return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b11000000);
    val |= startupParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
    return AXP_PASS;
}

int setlongPressTime(uint8_t param)
{
    uint8_t val;
    
        
    if (param > sizeof(longPressParams) / sizeof(longPressParams[0]))
        return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b00110000);
    val |= longPressParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
    return AXP_PASS;
}
*/
int axp202_set_shutdown_time(uint8_t param)
{
    uint8_t val;
    
        
    if (param > sizeof(shutdownParams) / sizeof(shutdownParams[0]))
        return AXP_INVALID;
    _readByte(AXP202_POK_SET, 1, &val);
    val &= (~0b00000011);
    val |= shutdownParams[param];
    _writeByte(AXP202_POK_SET, 1, &val);
    return AXP_PASS;
}
/*
int setTimeOutShutdown(bool en)
{
    uint8_t val;
    
        
    _readByte(AXP202_POK_SET, 1, &val);
    if (en)
        val |= (1 << 3);
    else
        val &= (~(1 << 3));
    _writeByte(AXP202_POK_SET, 1, &val);
    return AXP_PASS;
}

int shutdown()
{
    uint8_t val;
    
        
    _readByte(AXP202_OFF_CTL, 1, &val);
    val |= (1 << 7);
    _writeByte(AXP202_OFF_CTL, 1, &val);
    return AXP_PASS;
}
bool isChargeingEnable()
{
    uint8_t val;
    
        return false;
    _readByte(AXP202_CHARGE1, 1, &val);
    if (val & (1 << 7)) {
        AXP_DEBUG("Charging enable is enable\n");
        val = true;
    } else {
        AXP_DEBUG("Charging enable is disable\n");
        val = false;
    }
    return val;
}

int enableChargeing(bool en)
{
    uint8_t val;
    
        
    _readByte(AXP202_CHARGE1, 1, &val);
    val |= (1 << 7);
    _writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
}

int setChargingTargetVoltage(axp_chargeing_vol_t param)
{
    uint8_t val;
    
        
    if (param > sizeof(targetVolParams) / sizeof(targetVolParams[0]))
        return AXP_INVALID;
    _readByte(AXP202_CHARGE1, 1, &val);
    val &= ~(0b01100000);
    val |= targetVolParams[param];
    _writeByte(AXP202_CHARGE1, 1, &val);
    return AXP_PASS;
}

int axp202_get_batt_percentage()
{
    if (_chip_id != AXP202_CHIP_ID)
        return AXP_NOT_SUPPORT;
    uint8_t val;
    if (!isBatteryConnect())
        return 0;
    _readByte(AXP202_BATT_PERCENTAGE, 1, &val);
    if (!(val & BIT_MASK(7))) {
        return val & (~BIT_MASK(7));
    }
    return 0;
}

int setChgLEDMode(axp_chgled_mode_t mode)
{
    uint8_t val;
    _readByte(AXP202_OFF_CTL, 1, &val);
    val &= 0b11001111;
    val |= BIT_MASK(3);
    switch (mode) {
    case AXP20X_LED_OFF:
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_BLINK_1HZ:
        val |= 0b00010000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_BLINK_4HZ:
        val |= 0b00100000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    case AXP20X_LED_LOW_LEVEL:
        val |= 0b00110000;
        _writeByte(AXP202_OFF_CTL, 1, &val);
        break;
    default:
        return AXP_FAIL;
    }
    return AXP_PASS;
}

int debugCharging()
{
    uint8_t val;
    _readByte(AXP202_CHARGE1, 1, &val);
    AXP_DEBUG("SRC REG:0x%x\n", val);
    if (val & (1 << 7)) {
        AXP_DEBUG("Charging enable is enable\n");
    } else {
        AXP_DEBUG("Charging enable is disable\n");
    }
    AXP_DEBUG("Charging target-voltage : 0x%x\n", ((val & 0b01100000) >> 5) & 0b11);
    if (val & (1 << 4)) {
        AXP_DEBUG("end when the charge current is lower than 15%% of the set value\n");
    } else {
        AXP_DEBUG(" end when the charge current is lower than 10%% of the set value\n");
    }
    val &= 0b00000111;
    AXP_DEBUG("Charge current : %.2f mA\n", 300.0 + val * 100.0);
    return AXP_PASS;
}

int debugStatus()
{
    
        
    uint8_t val, val1, val2;
    _readByte(AXP202_STATUS, 1, &val);
    _readByte(AXP202_MODE_CHGSTATUS, 1, &val1);
    _readByte(AXP202_IPS_SET, 1, &val2);
    AXP_DEBUG("AXP202_STATUS:   AXP202_MODE_CHGSTATUS   AXP202_IPS_SET\n");
    AXP_DEBUG("0x%x\t\t\t 0x%x\t\t\t 0x%x\n", val, val1, val2);
    return AXP_PASS;
}

int limitingOff()
{
    
        
    uint8_t val;
    _readByte(AXP202_IPS_SET, 1, &val);
    if (_chip_id == AXP202_CHIP_ID) {
        val |= 0x03;
    } else {
        val &= ~(1 << 1);
    }
    _writeByte(AXP202_IPS_SET, 1, &val);
    return AXP_PASS;
}

// Only AXP129 chip and AXP173
int setDCDC1Voltage(uint16_t mv)
{
    
        
    if (_chip_id != AXP192_CHIP_ID && _chip_id != AXP173_CHIP_ID)
        return AXP_FAIL;
    if (mv < 700) {
        AXP_DEBUG("DCDC1:Below settable voltage:700mV~3500mV");
        mv = 700;
    }
    if (mv > 3500) {
        AXP_DEBUG("DCDC1:Above settable voltage:700mV~3500mV");
        mv = 3500;
    }
    uint8_t val = (mv - 700) / 25;
    //! axp192 and axp173 dc1 control register same
    _writeByte(AXP192_DC1_VLOTAGE, 1, &val);
    return AXP_PASS;
}

// Only AXP129 chip and AXP173
uint16_t getDCDC1Voltage()
{
    if (_chip_id != AXP192_CHIP_ID && _chip_id != AXP173_CHIP_ID)
        return AXP_FAIL;
    uint8_t val = 0;
    //! axp192 and axp173 dc1 control register same
    _readByte(AXP192_DC1_VLOTAGE, 1, &val);
    return val * 25 + 700;
}

*/
int axp202_set_timer(uint8_t minutes)
{
    if (_chip_id == AXP202_CHIP_ID) {
        if (minutes > 63) {
            return AXP_ARG_INVALID;
        }
        _writeByte(AXP202_TIMER_CTL, 1, &minutes);
        return AXP_PASS;
    }
    return AXP_NOT_SUPPORT;
}

int axp202_unset_timer()
{
    if (_chip_id == AXP202_CHIP_ID) {
        uint8_t minutes = 0x80;
        _writeByte(AXP202_TIMER_CTL, 1, &minutes);
        return AXP_PASS;
    }
    return AXP_NOT_SUPPORT;
}

int axp202_clear_timer()
{
    if (_chip_id == AXP202_CHIP_ID) {
        uint8_t val;
        _readByte(AXP202_TIMER_CTL, 1, &val);
        val |= 0x80;
        _writeByte(AXP202_TIMER_CTL, 1, &val);
        return AXP_PASS;
    }
    return AXP_NOT_SUPPORT;
}

/***********************************************
 *              !!! GPIO FUNCTION !!!
 * *********************************************/

int _axp192_gpio_0_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
        return 0b101;
    case AXP_IO_INPUT_MODE:
        return 0b001;
    case AXP_IO_LDO_MODE:
        return 0b010;
    case AXP_IO_ADC_MODE:
        return 0b100;
    case AXP_IO_FLOATING_MODE:
        return 0b111;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
        return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
    case AXP_IO_PWM_OUTPUT_MODE:
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp192_gpio_1_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
        return 0b101;
    case AXP_IO_INPUT_MODE:
        return 0b001;
    case AXP_IO_ADC_MODE:
        return 0b100;
    case AXP_IO_FLOATING_MODE:
        return 0b111;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
        return 0;
    case AXP_IO_PWM_OUTPUT_MODE:
        return 0b010;
    case AXP_IO_OUTPUT_HIGH_MODE:
    case AXP_IO_LDO_MODE:
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


int _axp192_gpio_3_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_EXTERN_CHARGING_CTRL_MODE:
        return 0;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
        return 1;
    case AXP_IO_INPUT_MODE:
        return 2;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp192_gpio_4_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_EXTERN_CHARGING_CTRL_MODE:
        return 0;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
        return 1;
    case AXP_IO_INPUT_MODE:
        return 2;
    case AXP_IO_ADC_MODE:
        return 3;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


int _axp192_gpio_set(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
    int rslt;
    uint8_t val;
    switch (gpio) {
    case AXP_GPIO_0: {
        rslt = _axp192_gpio_0_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP192_GPIO0_CTL, 1, &val);
        val &= 0xF8;
        val |= (uint8_t)rslt;
        _writeByte(AXP192_GPIO0_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_1: {
        rslt = _axp192_gpio_1_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP192_GPIO1_CTL, 1, &val);
        val &= 0xF8;
        val |= (uint8_t)rslt;
        _writeByte(AXP192_GPIO1_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_2: {
        rslt = _axp192_gpio_1_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP192_GPIO2_CTL, 1, &val);
        val &= 0xF8;
        val |= (uint8_t)rslt;
        _writeByte(AXP192_GPIO2_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_3: {
        rslt = _axp192_gpio_3_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP192_GPIO34_CTL, 1, &val);
        val &= 0xFC;
        val |= (uint8_t)rslt;
        _writeByte(AXP192_GPIO34_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_4: {
        rslt = _axp192_gpio_4_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP192_GPIO34_CTL, 1, &val);
        val &= 0xF3;
        val |= (uint8_t)rslt;
        _writeByte(AXP192_GPIO34_CTL, 1, &val);
        return AXP_PASS;
    }
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp202_gpio_0_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
        return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
        return 1;
    case AXP_IO_INPUT_MODE:
        return 2;
    case AXP_IO_LDO_MODE:
        return 3;
    case AXP_IO_ADC_MODE:
        return 4;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp202_gpio_1_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
        return 0;
    case AXP_IO_OUTPUT_HIGH_MODE:
        return 1;
    case AXP_IO_INPUT_MODE:
        return 2;
    case AXP_IO_ADC_MODE:
        return 4;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp202_gpio_2_select( axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_OUTPUT_LOW_MODE:
        return 0;
    case AXP_IO_INPUT_MODE:
        return 2;
    case AXP_IO_FLOATING_MODE:
        return 1;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


int _axp202_gpio_3_select(axp_gpio_mode_t mode)
{
    switch (mode) {
    case AXP_IO_INPUT_MODE:
        return 1;
    case AXP_IO_OPEN_DRAIN_OUTPUT_MODE:
        return 0;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp202_gpio_set(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
    uint8_t val;
    int rslt;
    switch (gpio) {
    case AXP_GPIO_0: {
        rslt = _axp202_gpio_0_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP202_GPIO0_CTL, 1, &val);
        val &= 0b11111000;
        val |= (uint8_t)rslt;
        _writeByte(AXP202_GPIO0_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_1: {
        rslt = _axp202_gpio_1_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP202_GPIO1_CTL, 1, &val);
        val &= 0b11111000;
        val |= (uint8_t)rslt;
        _writeByte(AXP202_GPIO1_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_2: {
        rslt = _axp202_gpio_2_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP202_GPIO2_CTL, 1, &val);
        val &= 0b11111000;
        val |= (uint8_t)rslt;
        _writeByte(AXP202_GPIO2_CTL, 1, &val);
        return AXP_PASS;
    }
    case AXP_GPIO_3: {
        rslt = _axp202_gpio_3_select(mode);
        if (rslt < 0)return rslt;
        _readByte(AXP202_GPIO3_CTL, 1, &val);
        val = rslt ? (val | BIT_MASK(2)) : (val & (~BIT_MASK(2)));
        _writeByte(AXP202_GPIO3_CTL, 1, &val);
        return AXP_PASS;
    }
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


int axp202_set_gpio_mode(axp_gpio_t gpio, axp_gpio_mode_t mode)
{
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        return _axp202_gpio_set(gpio, mode);
        break;
    case AXP192_CHIP_ID:
        return _axp192_gpio_set(gpio, mode);
        break;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


int _axp_irq_mask(axp_gpio_irq_t irq)
{
    switch (irq) {
    case AXP_IRQ_NONE:
        return 0;
    case AXP_IRQ_RISING:
        return BIT_MASK(7);
    case AXP_IRQ_FALLING:
        return BIT_MASK(6);
    case AXP_IRQ_DOUBLE_EDGE:
        return 0b1100000;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int _axp202_gpio_irq_set(axp_gpio_t gpio, axp_gpio_irq_t irq)
{
    uint8_t reg;
    uint8_t val;
    int mask;
    mask = _axp_irq_mask(irq);

    if (mask < 0)return mask;
    switch (gpio) {
    case AXP_GPIO_0:
        reg = AXP202_GPIO0_CTL;
        break;
    case AXP_GPIO_1:
        reg = AXP202_GPIO1_CTL;
        break;
    case AXP_GPIO_2:
        reg = AXP202_GPIO2_CTL;
        break;
    case AXP_GPIO_3:
        reg = AXP202_GPIO3_CTL;
        break;
    default:
        return AXP_NOT_SUPPORT;
    }
    _readByte(reg, 1, &val);
    val = mask == 0 ? (val & 0b00111111) : (val | mask);
    _writeByte(reg, 1, &val);
    return AXP_PASS;
}


int axp202_set_gpio_IRQ(axp_gpio_t gpio, axp_gpio_irq_t irq)
{
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        return _axp202_gpio_irq_set(gpio, irq);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
        return AXP_NOT_SUPPORT;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int axp202_set_LDO5_voltage(axp_ldo5_table_t vol)
{
    const uint8_t params[] = {
        0b11111000, //1.8V
        0b11111001, //2.5V
        0b11111010, //2.8V
        0b11111011, //3.0V
        0b11111100, //3.1V
        0b11111101, //3.3V
        0b11111110, //3.4V
        0b11111111, //3.5V
    };
    
        
    if (_chip_id != AXP202_CHIP_ID)
        return AXP_NOT_SUPPORT;
    if (vol > sizeof(params) / sizeof(params[0]))
        return AXP_ARG_INVALID;
    uint8_t val = 0;
    _readByte(AXP202_GPIO0_VOL, 1, &val);
    val &= 0b11111000;
    val |= params[vol];
    _writeByte(AXP202_GPIO0_VOL, 1, &val);
    return AXP_PASS;
}


int _axp202_gpio_write(axp_gpio_t gpio, uint8_t val)
{
    uint8_t reg;
    uint8_t wVal = 0;
    switch (gpio) {
    case AXP_GPIO_0:
        reg = AXP202_GPIO0_CTL;
        break;
    case AXP_GPIO_1:
        reg = AXP202_GPIO1_CTL;
        break;
    case AXP_GPIO_2:
        reg = AXP202_GPIO2_CTL;
        if (val) {
            return AXP_NOT_SUPPORT;
        }
        break;
    case AXP_GPIO_3:
        if (val) {
            return AXP_NOT_SUPPORT;
        }
        _readByte(AXP202_GPIO3_CTL, 1, &wVal);
        wVal &= 0b11111101;
        _writeByte(AXP202_GPIO3_CTL, 1, &wVal);
        return AXP_PASS;
    default:
        return AXP_NOT_SUPPORT;
    }
    _readByte(reg, 1, &wVal);
    wVal = val ? (wVal | 1) : (wVal & 0b11111000);
    _writeByte(reg, 1, &wVal);
    return AXP_PASS;
}

int _axp202_gpio_read(axp_gpio_t gpio)
{
    uint8_t val;
    uint8_t reg = AXP202_GPIO012_SIGNAL;
    uint8_t offset;
    switch (gpio) {
    case AXP_GPIO_0:
        offset = 4;
        break;
    case AXP_GPIO_1:
        offset = 5;
        break;
    case AXP_GPIO_2:
        offset = 6;
        break;
    case AXP_GPIO_3:
        reg = AXP202_GPIO3_CTL;
        offset = 0;
        break;
    default:
        return AXP_NOT_SUPPORT;
    }
    _readByte(reg, 1, &val);
    return val & BIT_MASK(offset) ? 1 : 0;
}

int axp202_gpio_write(axp_gpio_t gpio, uint8_t val)
{
    
        
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        return _axp202_gpio_write(gpio, val);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
        return AXP_NOT_SUPPORT;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int axp202_gpio_read(axp_gpio_t gpio)
{
    
        
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        return _axp202_gpio_read(gpio);
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
        return AXP_NOT_SUPPORT;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}



int axp202_get_charge_control_cur()
{
    int cur;
    uint8_t val;
    
        
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        _readByte(AXP202_CHARGE1, 1, &val);
        val &= 0x0F;
        cur =  val * 100 + 300;
        if (cur > 1800 || cur < 300)return 0;
        return cur;
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
        _readByte(AXP202_CHARGE1, 1, &val);
        return val & 0x0F;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}

int axp202_set_charge_control_cur(uint16_t mA)
{
    uint8_t val;
   
        
    switch (_chip_id) {
    case AXP202_CHIP_ID:
        _readByte(AXP202_CHARGE1, 1, &val);
        val &= 0b11110000;
        mA -= 300;
        val |= (mA / 100);
        _writeByte(AXP202_CHARGE1, 1, &val);
        return AXP_PASS;
    case AXP192_CHIP_ID:
    case AXP173_CHIP_ID:
        _readByte(AXP202_CHARGE1, 1, &val);
        val &= 0b11110000;
        if(mA > AXP1XX_CHARGE_CUR_1320MA)
            mA = AXP1XX_CHARGE_CUR_1320MA;
        val |= mA;
        _writeByte(AXP202_CHARGE1, 1, &val);
        return AXP_PASS;
    default:
        break;
    }
    return AXP_NOT_SUPPORT;
}


