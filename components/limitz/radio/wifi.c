#include "wifi.h"
#include "esp_https_ota.h"

static EventGroupHandle_t s_wifi_evt_group;

#define TAG "WIFI"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DONE_BIT BIT1


int wifi_ota_update(wifi_t* wifi)
{
    //ESP_LOGW("WIFI", "Firmware upgrade started %s %s", wifi->ota_pem, wifi->ota_urli);
    esp_http_client_config_t config = {
        .url = wifi->ota.url,
        .cert_pem = (char *)wifi->ota.pem,
    };
    ESP_ERROR_CHECK(esp_https_ota(&config));
    //wifi_disconnect();
    esp_restart();
    return ESP_OK;
}


static void wifi_task(void* arg)
{
    wifi_t* wifi = (wifi_t*) arg;

    ESP_LOGW("wifi_task", "ssid: %s password: %s", wifi->ssid, wifi->password);
    EventBits_t bits;
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, wifi->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, wifi->password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    for (;;)
    {
        bits = xEventGroupWaitBits(
            s_wifi_evt_group,
            WIFI_CONNECTED_BIT | WIFI_DONE_BIT,
            true, false, portMAX_DELAY);

        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGI("WIFI", "Connected");
        }
        if (bits & WIFI_DONE_BIT)
        {
            ESP_LOGI("WIFI", "Done");
            vTaskDelete(NULL);
        }
    }
}

static void wifi_handler(void* arg, esp_event_base_t evt_base, int32_t evt_id, void* evt_data)
{
    wifi_t* wifi = (wifi_t*) arg;

    ESP_LOGW("wifi_handler", "ssid: %s password: %s", wifi->ssid, wifi->password);

    #ifndef ESP_EVT_CASE
    #define ESP_EVT_CASE(base, id) if (evt_base == base && evt_id == base ## _ ##  id)
    #endif

    ESP_EVT_CASE(WIFI_EVENT, STA_START)
    {
        xTaskCreate(wifi_task, "WIFI Task", 4096+2048, wifi, 3, NULL);
    }
    ESP_EVT_CASE(WIFI_EVENT, STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_evt_group, WIFI_CONNECTED_BIT);
    }
    ESP_EVT_CASE(IP_EVENT, STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_evt_group, WIFI_CONNECTED_BIT);
    }
}

int wifi_init(wifi_t* wifi)
{
    tcpip_adapter_init();
    s_wifi_evt_group = xEventGroupCreate();
    esp_event_loop_create_default();

    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&config));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, wifi));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, wifi));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    return 0;

}

int wifi_destroy(wifi_t* wifi)
{
	//TODO
	return 0;
}
