# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
list(APPEND EXTRA_COMPONENT_DIRS modules/lv_port_esp32/components/lvgl)
list(APPEND EXTRA_COMPONENT_DIRS modules/lv_port_esp32/components/lvgl_esp32_drivers)
list(APPEND EXTRA_COMPONENT_DIRS modules/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_tft)
#list(APPEND EXTRA_COMPONENT_DIRS modules/lv_port_esp32/components/lvgl_esp32_drivers/lvgl_touch)
list(APPEND EXTRA_COMPONENT_DIRS components/limitz)
project(esp32-idf-components)
