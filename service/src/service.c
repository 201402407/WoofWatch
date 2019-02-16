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

// 파일의 이름을 가져오는 함수(확장명도).
char*
get_file_all_name(const char *path) {
	// char* not_const_path = (char *)path;
	char* not_const_path = malloc(sizeof(char) * 200);    // char 200개 크기만큼 동적 메모리 할당

	strcpy(not_const_path, path);    // s1에 문자열 복사
	char* ptr = strtok(not_const_path, "\\/");    			// " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
	char *temp;										// 맨 마지막 문자
	while (ptr != NULL)              // 자른 문자열이 나오지 않을 때까지 반복
		{
			temp = ptr; // 이러면 맨 마지막 문자가 출력됨
			ptr = strtok(NULL, "\\/");
		}
	free(not_const_path);

	dlog_print(DLOG_INFO, LOG_TAG, temp);
	return temp; 									// 파일이름과 확장명 리턴

	// 파일 이름이랑 배열 내 값이랑 비교해서 같은 결과 가진 인덱스 리턴
	// 곧 쓸 것 같다.
//	int length = sizeof(temp) / sizeof(temp[0]);
//	for(int i = 0; i < length; i++) {
//		if(strcmp(soundName[i], temp) == 0) {
//			return i;
//		}
//	}
//
}

/*
 * 파일 확장명을 추출해서 리턴
 * @param path: 파일 경로
 *
 */
char*
get_file_only_name(const char *path) {
	// char* not_const_path = (char *)path;
	char* not_const_path = malloc(sizeof(char) * 200);    // char 100개 크기만큼 동적 메모리 할당

	strcpy(not_const_path, path);    // s1에 문자열 복사
	char* ptr = strtok(not_const_path, ".");    			// " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
	// free
	return ptr;
}

/*
 * 파일 확장명을 추출해서 리턴
 * @param path: 파일 경로
 * @param menu: 1: 파일 이름,  2: 파일 확장명,  3: 파일 이름 + 확장명
 */
char*
get_file_only_extension(const char *path) {
	// char* not_const_path = (char *)path;
	char* not_const_path = malloc(sizeof(char) * 200);    // char 100개 크기만큼 동적 메모리 할당

	strcpy(not_const_path, path);    // s1에 문자열 복사
	char *file_name;
	char *file_extension;			 // 맨 마지막 문자

	file_name = strtok(not_const_path, ".");										// 파일이름
	file_extension = strtok(NULL, ".");										// 확장자명
	dlog_print(DLOG_INFO, LOG_TAG, "%s, %s" , file_extension, file_name);
	// free
	free(not_const_path);

	return file_extension;
}

/*
 * 파일 다운을 다 받으면 액티비티로 파일 전체 경로 전송
 */
void
send_activity_data(const char *key, const char *string) {
	dlog_print(DLOG_INFO, LOG_TAG, "service data send start");
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_add_extra_data(app_control, key, string); // 데이터 intent처럼 담기
	// app_control_add_extra_data_array(app_control, "recognition", string, string.length)
	app_control_set_app_id (app_control, "org.woof");
	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "file path send success.");
	}
	else
		dlog_print(DLOG_INFO, LOG_TAG, "Failed to launch a Service app.");
}

/*
 * 소리 인식 시 소리 종류 이름만 전송
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

/*
 * 피어 연결 테스트 하기 위해 띄워야 하는 팝업 메세지 전송
 */
void
popup_data_send(const char *string) {
	dlog_print(DLOG_INFO, LOG_TAG, "get popup");
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_add_extra_data(app_control, "popup", string); // 데이터 intent처럼 담기
	// app_control_add_extra_data_array(app_control, "recognition", string, string.length)
	app_control_set_app_id (app_control, "org.woof");
	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "popup success.");
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
