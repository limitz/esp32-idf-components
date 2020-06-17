#include <battery_monitor.h>
#include <unique_id.h>

static TaskHandle_t s_battery_monitor_task;

void app_main()
{

	ESP_LOGW("APP", "unique id = %s", unique_id());
	ESP_LOGW("APP", "unique id = %s", unique_id());
	
	xTaskCreate(
		battery_monitor_task, 
		"Battery monitor", 
		2048, NULL, tskIDLE_PRIORITY, 
		&s_battery_monitor_task);

	configASSERT(s_battery_monitor_task);
}
