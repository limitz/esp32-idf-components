
#include "lipo.h"
#include <string.h>
#include <stdio.h>

const int c_lipo_p2v[] = { 
        3270, 
        3610, 3690, 3710, 3730, 3750, 
        3770, 3790, 3800, 3820, 3840, 
        3850, 3870, 3910, 3950, 3980,
        4020, 4080, 4110, 4150, 4200
};

#define P2V_LEN (sizeof(c_lipo_p2v) / sizeof(int))

int lipo_percent_to_volt( int percent)
{
        if (percent < 0)   return BATTERY_EMPTY;
	if (percent > 100) return BATTERY_CHARGING;
        return c_lipo_p2v[(percent / 5) * 5]; 
}

int lipo_volt_to_percent(int mv) 
{
        if (mv < c_lipo_p2v[0])  return BATTERY_EMPTY;
	if (mv > 4500) return BATTERY_CHARGING;
        for (int i=0; i < P2V_LEN-1; i++)
        {
		const int* v = c_lipo_p2v+i;
                if (mv < v[1]) return (5*i) + 5*(mv - v[0])/(v[1] - v[0]);
        }
        return 100;
}

