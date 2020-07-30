idf_component_register(SRCS 
	"server.c" 
	"client.c"
	"packet.c"
	"wifi.c"
	"radio.c"
	INCLUDE_DIRS . ..
	REQUIRES "unique_id" "macaddr" "lwip" "wpa_supplicant" "esp_rom" "esp_wifi" "esp_https_ota"
	)


