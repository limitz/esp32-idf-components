idf_component_register(SRCS "network.c"
			INCLUDE_DIRS .
			REQUIRES "esp_wifi" "esp_https_ota" "wpa_supplicant"
			)


