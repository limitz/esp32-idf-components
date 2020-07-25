#pragma once

#define BATTERY_EMPTY     -1 
#define BATTERY_CHARGING  -2

int lipo_percent_to_volt( int percent);
int lipo_volt_to_percent(int mv);
