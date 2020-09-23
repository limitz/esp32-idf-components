#include <radio.h>

radio_t RADIO =  
{
	.identity = {
		.name = { 0 },
		.addr = { 0 },
		.features = { 0 },
		.connected = 0
	},
	.callbacks.on_accept = RADIO_ACCEPT_ANY,
};

#define MACADDR_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MACADDR_ARG(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4],(a)[5] 
#define LOGDATA8_FMT "%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x"
#define LOGDATA16_FMT LOGDATA8_FMT "'" LOGDATA8_FMT
#define LOGDATA32_FMT LOGDATA16_FMT "\"" LOGDATA16_FMT
#define LOGDATA4_ARG(a,n) (a)[(n)+0], (a)[(n)+1], (a)[(n)+2], (a)[(n)+3]
#define LOGDATA8_ARG(a,n) LOGDATA4_ARG(a,(n)),LOGDATA4_ARG(a,(n)+4)
#define LOGDATA16_ARG(a,n) LOGDATA8_ARG(a,(n)),LOGDATA8_ARG(a,(n)+8)
#define LOGDATA32_ARG(a,n) LOGDATA16_ARG(a,(n)),LOGDATA16_ARG(a,(n)+16)
#define log_packet(logger, packet) printf(".");
#define log_packet2(logger, packet) \
{ \
	logger(__func__, "src: " MACADDR_FMT ", dst: " MACADDR_FMT ", Type: %02x, Length: %d, Offset: %d",\
			MACADDR_ARG((packet)->header.addr_src), \
			MACADDR_ARG((packet)->header.addr_dst), \
			(packet)->header.type, (packet)->header.length, (packet)->header.offset);\
	logger(__func__, "[" LOGDATA32_FMT "]\n" ,\
			LOGDATA32_ARG((packet)->payload, 0));\
}


static void radio_send_cb(const uint8_t* addr, esp_now_send_status_t status)
{
	xQueueSend(RADIO.send_queue, &status, 1);
}

static void radio_recv_cb(const uint8_t* addr, const uint8_t* data, int len)
{
	if (len < sizeof(radio_packet_header_t)) 
	{
		ESP_LOGE(__func__, "Unsufficient data");
		return;
	}
	if (len > RADIO_PACKET_MTU) 
	{
		ESP_LOGE(__func__, "Too much data");
		return;
	}

	radio_packet_t packet;
	memcpy(&packet, data, len);
	xQueueSend(RADIO.queue, &packet, 1);
}

static void radio_task(void* param)
{
	radio_packet_t packet;
	
	while (pdTRUE == xQueueReceive(RADIO.queue, &packet, portMAX_DELAY))
	{
		log_packet(ESP_LOGW, &packet);

		switch (packet.header.type)
		{
			case RADIO_PACKET_TYPE_IDENTITY:
				if ( !esp_now_peer_exists(packet.header.addr_src) 
					&& ( RADIO_ACCEPT_ANY == RADIO.callbacks.on_accept 
						|| ( RADIO.callbacks.on_accept 
							&& RADIO_ACCEPT == RADIO.callbacks.on_accept(&packet))
				)){
					esp_now_peer_info_t peer = {
						.channel = CONFIG_LMTZ_RADIO_CHANNEL,
						.ifidx = ESPNOW_WIFI_IF,
						.encrypt = true,
					};
					memcpy(peer.lmk, CONFIG_LMTZ_RADIO_LMK, RADIO_KEY_SIZE);
					memcpy(peer.peer_addr, packet.header.addr_src, 6);
					ESP_ERROR_CHECK(esp_now_add_peer(&peer));
				}
				break;

			default:
				if (RADIO.callbacks.on_receive) 
					RADIO.callbacks.on_receive(&packet);
				break;
		}
	}
	ESP_LOGE(__func__, "STOPPED");
}

int radio_init()
{
	//ESP_ERROR_CHECK(esp_netif_init());
	tcpip_adapter_init();

	wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&config) );
	ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE));

	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
	ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );
	ESP_ERROR_CHECK( esp_wifi_start());

#if CONFIG_LMTZ_RADIO_LONGRANGE	
	ESP_ERROR_CHECK( 
			esp_wifi_set_protocol(
				ESPNOW_WIFI_IF, 
				  WIFI_PROTOCOL_11B
				| WIFI_PROTOCOL_11G
				| WIFI_PROTOCOL_11N
				| WIFI_PROTOCOL_LR) 
		);
	}
#endif

	esp_efuse_mac_get_default(RADIO.identity.addr);

	if (0 == RADIO.queue)
	{
		RADIO.queue = xQueueCreate(RADIO_PACKET_QUEUE_LEN,  RADIO_PACKET_MTU);
		assert(RADIO.queue);
	}

	if (0 == RADIO.send_queue)
	{
		RADIO.send_queue = xQueueCreate(3,sizeof(esp_now_send_status_t));
		assert(RADIO.send_queue);
		esp_now_send_status_t first = 0;
		//xQueueSend(RADIO.send_queue, &first, 1);
	}

	ESP_ERROR_CHECK( esp_now_init());
	ESP_ERROR_CHECK( esp_now_register_recv_cb(radio_recv_cb) );
	ESP_ERROR_CHECK( esp_now_register_send_cb(radio_send_cb) );
	ESP_ERROR_CHECK( esp_now_set_pmk((const uint8_t*)CONFIG_LMTZ_RADIO_PMK) );
	
	esp_now_peer_info_t broadcast = {
		.ifidx = ESPNOW_WIFI_IF,
		.channel = CONFIG_LMTZ_RADIO_CHANNEL,
		.encrypt = false,
		.peer_addr = RADIO_BROADCAST_ADDRESS,
	};
	ESP_ERROR_CHECK( esp_now_add_peer(&broadcast) );

	xTaskCreate(radio_task, "radio task", 4096, NULL, 4, &RADIO.task);
	return ESP_OK;
}

int radio_unicast(const uint8_t* addr_dst, int type, const void* payload, size_t len)
{
	int err;
	radio_packet_t packet;
	memcpy(packet.header.addr_dst, addr_dst, 6);
	memcpy(packet.header.addr_src, RADIO.identity.addr, 6);
	packet.header.type = type;
	
	for (int offset=0; offset < len; offset+= RADIO_PACKET_MAX_PAYLOAD)
	{
		int bytes = len - offset;
		if (bytes > RADIO_PACKET_MAX_PAYLOAD) bytes = RADIO_PACKET_MAX_PAYLOAD;
		packet.header.length =  bytes;
		packet.header.offset = offset;
		memcpy(packet.payload, ((const uint8_t*)payload)+offset, packet.header.length);
		
		err = radio_send_packet(addr_dst, &packet);
		if (err) return err;
	}
	return ESP_OK;
}

int radio_broadcast(int type, const void* payload, size_t len)
{
	uint8_t broadcast[6]  = RADIO_BROADCAST_ADDRESS;
	return radio_unicast(broadcast, type, payload, len);
}

int radio_send_packet(const uint8_t* addr_dst, const radio_packet_t* packet)
{
	esp_now_send_status_t status;
	do 
	{
		esp_now_send(addr_dst, (const uint8_t*) packet, packet->header.length + sizeof(radio_packet_header_t));
	}
	while (pdTRUE == xQueueReceive(RADIO.send_queue, &status, 60000) && status);

	log_packet(ESP_LOGI, packet);
	return status;
}

int radio_deinit(radio_t* self)
{
	vTaskDelete(RADIO.task);
	vSemaphoreDelete(RADIO.queue);
	RADIO.queue = 0;
	esp_now_deinit();
	return ESP_OK;
}
