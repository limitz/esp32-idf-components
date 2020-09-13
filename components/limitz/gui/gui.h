#pragma once
#include <lv_conf.h>
#include <lvgl/lvgl.h>
#include <lvgl_helpers.h>

int gui_init();
int gui_start(bool animate);
int gui_stop(bool animate);
int gui_deinit();

lv_obj_t* gui_root();

