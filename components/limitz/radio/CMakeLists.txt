idf_component_register(SRCS 
	"radio.c"
	INCLUDE_DIRS . ..
	REQUIRES "unique_id" "macaddr" "lwip" "wpa_supplicant" "esp_rom" "esp_wifi"
	)


