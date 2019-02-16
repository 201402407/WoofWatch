#include "service.h"

Ecore_Timer *timer1;
int timer_count = 0;

static Eina_Bool timer1_cb(void *data EINA_UNUSED) {
	timer_count ++;
	char buf[100];
	sprintf(buf, "Count - %d", timer_count);
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s - %s", __func__, buf);

//	if( timer_count > 5 )
//		service_app_exit();
	return ECORE_CALLBACK_RENEW;
}

/*
 * 앱에서 서비스 시작하도록 보내는 함수
 */
bool
_app_control_extra_data_cb(app_control_h app_control, const char *key, void *data)
{
int ret;
char *value;
ret = app_control_get_extra_data(app_control, key, &value);
dlog_print(DLOG_DEBUG, LOG_TAG, "%s - %s : %s", __func__, key, value);

/*
if(strcmp(key, "stop") == 0 && strcmp(value, "stop") == 0 )
{
dlog_print(DLOG_DEBUG, LOG_TAG, "Close message received");
switch_change(0);
service_app_exit();
}
if(strcmp(key, "start") == 0 && strcmp(value, "start") == 0 )
{
	switch_change(1);
dlog_print(DLOG_DEBUG, LOG_TAG, "Open message received");
}
*/

return true;
}

/*
 * 다시 앱으로 정보 전달하는 함수
 */
void
recognition_data_send(const char *string) {
	dlog_print(DLOG_INFO, LOG_TAG, "sound recognition");
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_add_extra_data(app_control, "recognition", string); // 데이터 intent처럼 담기
	// app_control_add_extra_data_array(app_control, "recognition", string, string.length)
	app_control_set_app_id (app_control, "org.woof");
	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "sound recognition success.");
	}
	else
		dlog_print(DLOG_INFO, LOG_TAG, "Failed to launch a Service app.");
}

bool service_app_create(void *data)
{
	 dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
	 timer_count = 0;
	 // timer1 = ecore_timer_add(1.0, timer1_cb, NULL);
	 initialize_sap(); // sap 시작
    return true;
}

void service_app_terminate(void *data)
{
	 dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
    // Todo: add your code here.
    return;
}

void service_app_control(app_control_h app_control, void *data)
{
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s", __func__);
	app_control_foreach_extra_data(app_control, _app_control_extra_data_cb, NULL);
    // Todo: add your code here.
    return;
}

static void
service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void
service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[])
{
    char ad[50] = {0,};
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	return service_app_main(argc, argv, &event_callback, ad);
}
