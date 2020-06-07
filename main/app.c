#include <battery_monitor.h>

static TaskHandle_t s_battery_monitor_task;

void app_main()
{
	xTaskCreate(
		battery_monitor_task, 
		"Battery monitor", 
		1024, NULL, tskIDLE_PRIORITY, 
		&s_battery_monitor_task);

	configASSERT(s_battery_monitor_task);
}
