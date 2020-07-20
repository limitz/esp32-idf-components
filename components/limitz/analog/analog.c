#include <analog.h> 

#define TAG "ANALOG"
#define ADC_DEFAULT_VREF 	1100
#define ADC_NUM_SAMPLES 	16
#define ADC_BITDEPTH 		ADC_WIDTH_BIT_12
#define ADC_ATTENUATION 	ADC_ATTEN_DB_11

static esp_adc_cal_characteristics_t s_calibration = { 0 };
static esp_adc_cal_value_t s_calibration_value;

int analog_init()
{
	adc1_config_width(ADC_BITDEPTH);
	s_calibration_value = esp_adc_cal_characterize(
			ADC_UNIT_1,
			ADC_ATTENUATION,
			ADC_BITDEPTH,
			ADC_DEFAULT_VREF,
			&s_calibration);
	return 0;
}
int analog_input_init(analog_input_t* self)
{
	self->_window = self->window_size 
			? (int*) calloc(sizeof(int), self->window_size)
			: NULL;
	
	adc1_config_channel_atten(self->channel, ADC_ATTENUATION);
	self->voltage = -1;
	self->last_value = -1;
	self->last_voltage = -1;
	self->_window_count = 0;
	self->_window_sum = 0;
	self->_window_index = 0;
	return 0;
}

int analog_input_destroy(analog_input_t* self)
{
	if (self->_window) free(self->_window);
	return 0;
}

int analog_input_update(analog_input_t* self)
{
	uint32_t reading = 0;
	for (int i=0; i<ADC_NUM_SAMPLES; i++) 
	{
		reading += adc1_get_raw(self->channel);
	}
	reading /= ADC_NUM_SAMPLES;

	int v = esp_adc_cal_raw_to_voltage(reading, &s_calibration);
	if (self->_window) 
	{
		self->_window_sum += v - self->_window[self->_window_index];
		self->_window[self->_window_index] = v;
		if (++(self->_window_index) >= self->window_size) 
		{
			self->_window_index = 0;
		}
		if (self->_window_count < self->window_size) 
		{
			self->_window_count++;
		}
		self->voltage = self->_window_sum / self->_window_count;
	}
	else self->voltage = v;

	self->last_value = reading;
	self->last_voltage = v;

	return 0;
}
