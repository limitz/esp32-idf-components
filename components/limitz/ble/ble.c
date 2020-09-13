#include "ble.h"
#include <esp_bt.h>
#include <esp_gap_ble_api.h>
#include <esp_gattc_api.h>
#include <esp_gatt_defs.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

#define TAG __func__
#define BDA_FORMAT "%02X:%2X:%2X:%2X:%2X:%2X"
#define BDA_ARG(a) a[0], a[1], a[2], a[3], a[4], a[5]

static struct gattc_open_evt_param s_remote;
static esp_bt_uuid_t CTS = { .len = ESP_UUID_LEN16, .uuid = 0x1805 };
static esp_bt_uuid_t CTC = { .len = ESP_UUID_LEN16, .uuid = 0x2A2B };

static void on_gap_ble_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
}

static void on_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gif, esp_ble_gattc_cb_param_t *param)
{
	int err;

	switch (event)
	{
	case ESP_GATTC_REG_EVT:

		ESP_LOGD(TAG, "ESP_GATTC_REG_EVT");
		const esp_ble_scan_params_t scan_params = 
		{
			.scan_type = BLE_SCAN_TYPE_ACTIVE,
			.own_addr_type = BLE_ADDR_TYPE_PUBLIC,
			.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
			.scan_interval = 0x50,
			.scan_window = 0x30,
			.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
		};

		err = esp_ble_gap_set_scan_params(&scan_params);
		if (ESP_OK != err) 
		{
			ESP_LOGE(TAG, "esp_ble_gap_set_scan_params. error 0x%08X", result);
		}
		break;

	case ESP_GATTC_OPEN_EVT: 
		
		ESP_LOGD(TAG, "ESP_GATTC_OPEN_EVT");
		if (ESP_GATT_OK != param->open.status)
		{
			ESP_LOGE(TAG, "Failed to connect. error 0x%08X", param->open.status);
		}
		else 
		{
			s_remote = param->open;
		}
		break;

	case ESP_GATTC_SEARCH_CMPL_EVT:
		
		ESP_LOGD(TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
		if (ESP_GATT_OK != param->search_cmpl.status)
		{
			ESP_LOGE(TAG, "Service discovery failed. error %08X", param->search_cmpl.status);
			break;
		}
#if 0
		err = esp_ble_gattc_get_attr_couni(
				gif, param->search_cmpl.conn_id, ESP_GATT_DB_CHARACTERISTIC,
				s_remote.CTS.start, s_remote.CTS.end, INVALID_HANDLE, 
				&s_remote.CTS.attr_count
		);
#endif
		err = esp_ble_gattc_get_char_by_uuid(
				gif, param->search_cmpl.conn_id, 
				s_remote.CTS.start, s_remote.CTS.end,
				CTC,
				s_remote.c
		
		// get attributes
		// get characteristics


		break;

	case ESP_GATTC_CONNECT_EVT:
	case ESP_GATTC_DISCONNECT_EVT:
	case ESP_GATTC_WRITE_CHAR_EVT:
	case ESP_GATTC_WRITE_DESCR_EVT:
	case ESP_GATTC_REG_FOR_NOTIFY_EVT:	
	case ESP_GATTC_NOTIFY_EVT:
	case ESP_GATTC_SRVC_CHG_EVT:
		break;
	}
}

static void start_scan()
{
	esp_ble_gap_start_scanning(30);
}


