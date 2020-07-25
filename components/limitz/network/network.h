#pragma once

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "tcpip_adapter.h"

//extern const uint8_t ota_pem_start[] asm("_binary_ota_pem_start");
//extern const uint8_t ota_pem_stop[]  asm("_binary_ota_pem_end");

#ifndef CONFIG_LMTZ_WIFI_SSID
#define CONFIG_LMTZ_WIFI_SSID ""
#endif

#ifndef CONFIG_LMTZ_WIFI_PASSWORD
#define CONFIG_LMTZ_WIFI_PASSWORD ""
#endif

#ifndef CONFIG_LMTZ_WIFI_RETRIES
#define CONFIG_LMTZ_WIFI_RETRIES 5
#endif

#ifndef CONFIG_WIFI_LMTZ_OTA_URL
#define CONFIG_WIFI_LMTZ_OTA_URL NULL
#endif

// load binary from file, not this
#ifndef CONFIG_WIFI_LMTZ_OTA_PEM
#define CONFIG_WIFI_LMTZ_OTA_PEM \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDuDCCAqACCQDM+/UYHLxkejANBgkqhkiG9w0BAQsFADCBnDELMAkGA1UEBhMC"\
"TkwxFTATBgNVBAgMDFp1aWQtSG9sbGFuZDESMBAGA1UEBwwJV2Fzc2VuYWFyMRcw"\
"FQYDVQQKDA5TcGxlbmRvIEhlYWx0aDELMAkGA1UECwwCTkExGzAZBgNVBAMMEm90"\
"YS5zcGxlbmRvLmhlYWx0aDEfMB0GCSqGSIb3DQEJARYQaW5mb0BzcGxlbmRvLmNv"\
"bTAgFw0yMDA2MjYxMjQ3MzZaGA8yMjk0MDQxMTEyNDczNlowgZwxCzAJBgNVBAYT"\
"Ak5MMRUwEwYDVQQIDAxadWlkLUhvbGxhbmQxEjAQBgNVBAcMCVdhc3NlbmFhcjEX"\
"MBUGA1UECgwOU3BsZW5kbyBIZWFsdGgxCzAJBgNVBAsMAk5BMRswGQYDVQQDDBJv"\
"dGEuc3BsZW5kby5oZWFsdGgxHzAdBgkqhkiG9w0BCQEWEGluZm9Ac3BsZW5kby5j"\
"b20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC3uTVOByNiVlZlQoC6"\
"z9v6W+1Y4i7BtOu92fVyDBChcQGKa8MWe+n7eAmUIZ7g/WR/DQOoyMEL39cTAzho"\
"cDpr7oNTmvsDmJezTgfJMCKEod/esIjxsfLPCkLcibCLuQYiCjGjDuoyfRO5rBzD"\
"97/buMbN12x03kP2B0xF6SWkvCso2kU9hbVemANdkh+74jItDknthXFcZw2/ZyE9"\
"twW8K1HEDqBUF3i3kDxmsRHbBdkeAO4gnMtKPAylo7Y11cnkzpyVFpvZ/3edjiUh"\
"TF7sSSyCwEhmcr58tNJcqPSniGKnQ1SflhHnzzQSbW3l3KSKCaB421pavwMi/dPP"\
"DmDrAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAJxw0IC36K8WHHLQuLA4NW098IdO"\
"4hG24O6CxPS7nBiNAdDeJaJCPMqvJu8MnkjLKs2eO5O877sIJV/jVLeFeLtIGuxi"\
"Xxyb9oL2+CL85NdLf9rUdCwznkIaVVVJpa2nEDgaVG5Y4Orj++K3Gq+Wu8fNT2Ei"\
"kycadpH6iReZXWoOk1+I1O8E5ZDatifv8fHT0kndIjZM/nFgThfS4DAbJxOX+PFE"\
"nJfvvsSfpWqdSrZsLlmkHN5ovKyYkyXfLQ8J3rwBscvkYE9bP7KI1hXr7aFCqJWW"\
"0J3zioUppnyBdghIRKeRaCvpSdiwZirhm5hKMd/Ml6FsNTN29v854CVaiZw=\n"\
"-----END CERTIFICATE-----"
#endif

typedef struct
{
    int retries;
    const char* ssid;
    const char* password;
    struct {
	const char* url;
	const char* pem;
    } ota;
} wifi_t;

typedef  int (*wifi_cb) (wifi_t* sender, int error);

#define WIFI_DEFAULT \
{ \
    .ssid = CONFIG_WIFI_SSID, \
    .password = CONFIG_WIFI_PASSWORD, \
    .retries = CONFIG_WIFI_RETRIES, \
    .ota = { \
	    .url = CONFIG_WIFI_OTA_URL,\
	    .pem = CONFIG_WIFI_OTA_PEM, \
    }, \
}

int wifi_init(wifi_t* wifi);
int wifi_connect(wifi_t* wifi);
int wifi_disconnect(wifi_t* wifi);
int wifi_ota_update(wifi_t* wifi);
int wifi_destroy(wifi_t* wifi);
