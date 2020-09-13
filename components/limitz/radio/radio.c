#include <radio.h>
#include <unique_id.h>

radio_t RADIO =  
{
	.callbacks.on_accept = RADIO_ACCEPT_ANY,
};

/*
static radio_packet_t* next_packet()
{
	return NULL;
}*/

#define log_packet(logger, tag, to, packet) \
{ \
	logger(tag, "Dest: " MACADDR_FMT ", Type: %02x, Length: %d, Offset: %d",\
			MACADDR_ARGS(to), (packet)->type, (packet)->length, (packet)->offset);\
	if (!(packet)->type) { \
		uint8_t* f = (packet)->identity.features; \
		logger(tag, "addr: " MACADDR_FMT ", name: %s, features: %d %d %d %d %d %d %d %d",\
				MACADDR_ARGS(&(packet)->identity.addr), \
				(packet)->identity.name, \
				f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);\
	}\
}

static void radio_recv_cb(const uint8_t* addr, const uint8_t* data, int len)
{
	if (len < sizeof(8)) 
	{
		//ESP_LOGE("Unsufficient data");
		return;
	}
	if (len > RADIO_PACKET_MAX_PAYLOAD) 
	{
		//ESP_LOGE("Too much data");
		return;
	}

	radio_packet_t packet;

	//memcpy(packet.identity.addr.ptr, addr, 6);
	memcpy(&packet, data, len);
	xQueueSend(RADIO.queue, &packet, 1);
}

static void radio_task(void* param)
{
	radio_packet_t packet;
	
	while (pdTRUE == xQueueReceive(RADIO.queue, &packet, portMAX_DELAY))
	{
		log_packet(ESP_LOGW, "RECV", &RADIO.identity.addr, &packet);

		switch (packet.type)
		{
			case RADIO_PACKET_TYPE_IDENTITY:
				if (RADIO_ACCEPT_ANY == RADIO.callbacks.on_accept
				|| (RADIO.callbacks.on_accept
				    && !esp_now_peer_exists(packet.identity.addr.ptr)
				    && RADIO_ACCEPT == RADIO.callbacks.on_accept(&packet))
				){
					esp_now_peer_info_t peer = {
						.channel = CONFIG_LMTZ_RADIO_CHANNEL,
						.ifidx = ESPNOW_WIFI_IF,
						.encrypt = true,
					};
					memcpy(peer.lmk, CONFIG_LMTZ_RADIO_LMK, RADIO_KEY_SIZE);
					memcpy(peer.peer_addr, packet.identity.addr.ptr, 6);
					ESP_ERROR_CHECK(esp_now_add_peer(&peer));
				}
				break;

			case RADIO_PACKET_TYPE_DATA:
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

	// TODO remove unique id dependency
	if (0 == *RADIO.identity.name)
	{
		strcpy(RADIO.identity.name, unique_id());
		RADIO.identity.addr =  macaddr();
	}

	if (0 == RADIO.queue)
	{
		RADIO.queue = xQueueCreate(RADIO_PACKET_QUEUE_SIZE,  RADIO_PACKET_MAX_PAYLOAD);
		assert(RADIO.queue);
	}

	ESP_ERROR_CHECK( esp_now_init());
	ESP_ERROR_CHECK( esp_now_register_recv_cb(radio_recv_cb) );
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

int radio_unicast(const macaddr_t* to, int type, const void* payload, size_t len)
{
	// TODO if len > MAX, multipart using offset
	radio_packet_t packet = {
		.type = type,
		.length = len + 6,
		.offset = 0,
	};
	memcpy(packet.payload, payload, len);
	return radio_send_packet(to, &packet);
}

int radio_broadcast(int type, const void* payload, size_t len)
{
	macaddr_t broadcast = { .ptr = RADIO_BROADCAST_ADDRESS };
	return radio_unicast(&broadcast, type, payload, len);
}

int radio_send_packet(const macaddr_t* to, const radio_packet_t* packet)
{
	//packet->crc = 0;
	//packet->crc = crc16_le(UINT16_MAX, (const void*) packet, RADIO_PACKET_SIZE);

	log_packet(ESP_LOGI, "SEND", to, packet);
	esp_now_send(to->ptr, (const uint8_t*) packet, packet->length);
	return ESP_OK;
}

int radio_deinit(radio_t* self)
{
	vTaskDelete(RADIO.task);
	vSemaphoreDelete(RADIO.queue);
	RADIO.queue = 0;
	esp_now_deinit();
	return ESP_OK;
}
