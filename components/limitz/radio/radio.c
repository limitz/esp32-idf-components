#include <radio.h>
#include <unique_id.h>

QueueHandle_t s_radio_queue = 0;

static void radio_recv_cb(const uint8_t* addr, const uint8_t* data, int len)
{
	radio_packet_t* packet = (radio_packet_t*) data;
	
	assert(addr);
	assert(data);
	assert(sizeof(radio_packet_t) == len);

	//const uint16_t crca = packet->crc;
	//packet->crc = 0;
	//const uint16_t crcb = crc16_le(UINT16_MAX, packet->data, RADIO_PACKET_SIZE);
	//if (crca != crcb) 
	//{ 
	//	ESP_LOGE(TAG, "Corrupt packet"); 
	//	return; 
	//}

	
	xQueueSendToFront(s_radio_queue, packet, 1);
}

static void radio_task(void* param)
{
	radio_t* radio = (radio_t*) param;
	radio_packet_t packet;
	
	while (pdTRUE == xQueueReceive(s_radio_queue, &packet, portMAX_DELAY))
	{
		switch (packet.type)
		{
			case RADIO_PACKET_TYPE_IDENTITY:
				if ( radio->callbacks.accept
				  && !esp_now_peer_exists(packet.identity.addr.ptr)
				  && RADIO_ACCEPT == radio->callbacks.accept(&packet.identity))
				{
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
				if (radio->callbacks.receive) radio->callbacks.receive(&packet);
				break;
		}
	}
	ESP_LOGE(__func__, "STOPPED");
}

int radio_init(radio_t* self)
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

	self->broadcast_addr = macaddr_parse("FF:FF:FF:FF:FF:FF");

	if (0 == *self->identity.name)
	{
		strcpy(self->identity.name, unique_id());
		self->identity.addr =  macaddr();
	}
	if (0 == s_radio_queue)
	{
		s_radio_queue = xQueueCreate(RADIO_QUEUE_SIZE,  RADIO_PACKET_SIZE);
		assert(s_radio_queue);
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

	xTaskCreate(radio_task, "radio task", 2048, self, 4, NULL);

return ESP_OK;
}

int radio_send(radio_t* radio, radio_packet_t* packet)
{
	macaddr_t dst, tmp = packet->addr;
	if (packet->flag & RADIO_PACKET_FLAG_BROADCAST)
	{
		dst = radio->broadcast_addr;
	}
	else 
	{
		dst = packet->addr;
	}
	packet->addr = macaddr();
	packet->seq++;
	packet->crc = 0;
	packet->crc = crc16_le(UINT16_MAX, (const void*) packet, RADIO_PACKET_SIZE);

	//ESP_LOGW(__func__, "[TYPE:%02X FLAGS:%02X SEQ:%04X CRC:%04X]",
		//	packet->type, packet->flag, packet->seq, packet->crc );
	//ESP_LOG_BUFFER_HEX(__func__, dst.ptr, 6);
	//ESP_LOG_BUFFER_HEX(__func__, &packet->payload, CONFIG_LMTZ_RADIO_PACKET_PAYLOAD_SIZE);

	esp_now_send(dst.ptr, (const void*) packet, sizeof(radio_packet_t));
	packet->addr = tmp;

	return ESP_OK;
}

int radio_destroy(radio_t* self)
{
	vSemaphoreDelete(s_radio_queue);
	s_radio_queue = 0;
	esp_now_deinit();
	return ESP_OK;
}
