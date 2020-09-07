#pragma once

#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_camera.h>

typedef struct
{
	camera_config_t config;
} camera_t;

int _camera_init();
int _camera_capture();
int _camera_deinit();

extern camera_t CAMERA;
