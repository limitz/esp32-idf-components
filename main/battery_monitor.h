#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"

esp_err_t battery_monitor_set_window(int window);
esp_err_t battery_monitor_get_window(int* window);

esp_err_t battery_monitor_set_interval(int ms);
esp_err_t battery_monitor_get_interval(int* ms);

esp_err_t battery_monitor_get_percentage(int* percentage);
esp_err_t battery_monitor_get_voltage(float* voltage);

void battery_monitor_task(void* param);
esp_err_t battery_monitor_step();

