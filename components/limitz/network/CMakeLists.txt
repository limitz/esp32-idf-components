idf_component_register(SRCS 
	"server.c" 
	"client.c"
	"packet.c"
	"wifi.c"
	INCLUDE_DIRS . ..
	REQUIRES "lwip" "wpa_supplicant" "esp_wifi" "esp_https_ota"
	)


