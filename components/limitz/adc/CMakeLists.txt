idf_component_register(SRCS "adc.c" "lipo.c"
			INCLUDE_DIRS .
			REQUIRES "nvs_flash" "esp_adc_cal")


