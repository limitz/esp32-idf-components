#include "wifi.h"
#include "esp_https_ota.h"

static const char* const s_pem =
"-----BEGIN CERTIFICATE-----\n"
"MIIDuDCCAqACCQDM+/UYHLxkejANBgkqhkiG9w0BAQsFADCBnDELMAkGA1UEBhMC"
"TkwxFTATBgNVBAgMDFp1aWQtSG9sbGFuZDESMBAGA1UEBwwJV2Fzc2VuYWFyMRcw"
"FQYDVQQKDA5TcGxlbmRvIEhlYWx0aDELMAkGA1UECwwCTkExGzAZBgNVBAMMEm90"
"YS5zcGxlbmRvLmhlYWx0aDEfMB0GCSqGSIb3DQEJARYQaW5mb0BzcGxlbmRvLmNv"
"bTAgFw0yMDA2MjYxMjQ3MzZaGA8yMjk0MDQxMTEyNDczNlowgZwxCzAJBgNVBAYT"
"Ak5MMRUwEwYDVQQIDAxadWlkLUhvbGxhbmQxEjAQBgNVBAcMCVdhc3NlbmFhcjEX"
"MBUGA1UECgwOU3BsZW5kbyBIZWFsdGgxCzAJBgNVBAsMAk5BMRswGQYDVQQDDBJv"
"dGEuc3BsZW5kby5oZWFsdGgxHzAdBgkqhkiG9w0BCQEWEGluZm9Ac3BsZW5kby5j"
"b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC3uTVOByNiVlZlQoC6"
"z9v6W+1Y4i7BtOu92fVyDBChcQGKa8MWe+n7eAmUIZ7g/WR/DQOoyMEL39cTAzho"
"cDpr7oNTmvsDmJezTgfJMCKEod/esIjxsfLPCkLcibCLuQYiCjGjDuoyfRO5rBzD"
"97/buMbN12x03kP2B0xF6SWkvCso2kU9hbVemANdkh+74jItDknthXFcZw2/ZyE9"
"twW8K1HEDqBUF3i3kDxmsRHbBdkeAO4gnMtKPAylo7Y11cnkzpyVFpvZ/3edjiUh"
"TF7sSSyCwEhmcr58tNJcqPSniGKnQ1SflhHnzzQSbW3l3KSKCaB421pavwMi/dPP"
"DmDrAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAJxw0IC36K8WHHLQuLA4NW098IdO"
"4hG24O6CxPS7nBiNAdDeJaJCPMqvJu8MnkjLKs2eO5O877sIJV/jVLeFeLtIGuxi"
"Xxyb9oL2+CL85NdLf9rUdCwznkIaVVVJpa2nEDgaVG5Y4Orj++K3Gq+Wu8fNT2Ei"
"kycadpH6iReZXWoOk1+I1O8E5ZDatifv8fHT0kndIjZM/nFgThfS4DAbJxOX+PFE"
"nJfvvsSfpWqdSrZsLlmkHN5ovKyYkyXfLQ8J3rwBscvkYE9bP7KI1hXr7aFCqJWW"
"0J3zioUppnyBdghIRKeRaCvpSdiwZirhm5hKMd/Ml6FsNTN29v854CVaiZw=\n"
"-----END CERTIFICATE-----";

static EventGroupHandle_t s_wifi_evt_group;

#define TAG "WIFI"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DONE_BIT BIT1


static void wifi_connect(wifi_t* wifi)
{
    wifi_config_t config;
    bzero(&config, sizeof(config));
    memcpy(config.sta.ssid, wifi->ssid, sizeof(config.sta.ssid));
    memcpy(config.sta.password, wifi->password, sizeof(config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void wifi_disconnect(wifi_t* wifi, wifi_cb callback)
{
    xEventGroupClearBits(s_wifi_evt_group, WIFI_CONNECTED_BIT);
    xEventGroupSetBits(s_wifi_evt_group, WIFI_DONE_BIT);
    ESP_ERROR_CHECK(esp_wifi_disconnect());
}

static void do_ota_upgrade(wifi_t* wifi)
{
    //ESP_LOGW("WIFI", "Firmware upgrade started %s %s", wifi->ota_pem, wifi->ota_urli);
    esp_http_client_config_t config = {
        .url = wifi->ota_url,
        .cert_pem = (char *)s_pem,
    };
    ESP_ERROR_CHECK(esp_https_ota(&config));
    //wifi_disconnect();
    esp_restart();
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

    ESP_LOGI(TAG, "SSID:%s", wifi->ssid);
    ESP_LOGI(TAG, "PASSWORD:%s", wifi->password);

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

            if (wifi->ota_url && strlen(wifi->ota_url))
            {
                do_ota_upgrade(wifi);
            }
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

void wifi_init(wifi_t* wifi)
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
}

void wifi_destroy(wifi_t* wifi)
{

}
