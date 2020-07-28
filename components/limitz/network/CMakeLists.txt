idf_component_register(SRCS 
	"server.c" 
	"packet.c"
	"network.c"
	INCLUDE_DIRS . ..
	REQUIRES "lwip" "wpa_supplicant" "esp_wifi" "esp_https_ota"
	)


