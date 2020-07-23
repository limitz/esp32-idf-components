#pragma once

#define BATTERY_EMPTY     -1 
#define BATTERY_CHARGING  -2

const int c_lipo_p2v[] = { 
        3270, 
        3610, 3690, 3710, 3730, 3750, 
        3770, 3790, 3800, 3820, 3840, 
        3850, 3870, 3910, 3950, 3980,
        4020, 4080, 4110, 4150, 4200
};

#define P2V_LEN (sizeof(c_lipo_p2v) / sizeof(int))

static int lipo_percent_to_volt( int percent)
{
        if (percent < 0)   return BATTERY_EMPTY;
	if (percent > 100) return BATTERY_CHARGING;
        return c_lipo_p2v[(percent / 5) * 5]; 
}

static int lipo_volt_to_percent(int mv) 
{
        if (mv < c_lipo_p2v[0])  return BATTERY_EMPTY;
	if (mv > c_lipo_p2v[P2V_LEN]) return BATTERY_CHARGING;
        for (int i=1; i < P2V_LEN; i++)
        {
		const int* v = c_lipo_p2v+i;
                if (mv < *v) return 5*i + 5*(mv - v[-1])/(*v - v[-1]);
        }
        return 100;
}

