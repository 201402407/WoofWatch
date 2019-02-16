#include <main.h>

// 음성인식 초기 값
int soundFlag = -1;
/*
 * sound alarm array
 */
char* soundName[4];
char* color[4];
char* soundIcon[4] = {"CAR", "CRY", "BELL", "SIREN"};
gboolean enable[4];
int vibrate_pattern[10][4];

static void _reject_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	evas_object_del(data);
	//reject_file();
}

static void _accept_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!obj) return;
	evas_object_del(data);
	//accept_file();
}

static void
layout_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	// Let window go to hide state.
	elm_naviframe_item_pop(ad->naviframe); // pop할떈 naviframe 인자로 넣자
	//elm_win_lower(ad->win);
	if(device_power_wakeup(true) == 0)
					dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep !");
	device_power_request_lock(POWER_LOCK_DISPLAY_DIM, 1);
	device_haptic_stop(ad->handle, &ad->effect_handle);
	dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep 2 !");
}

/*
 * 서비스 시작하는 함수
 */
static void
service_data_send(void *data, char *key, char *value) {

	appdata_s *ad = data;
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_add_extra_data(app_control, key, value);
	app_control_set_app_id (app_control, "org.woof.app2"); // id값 잘 수정하기

	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "Succeeded to launch a Service app.");
		elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->img,"empty"); // ?
	}
	else
		dlog_print(DLOG_INFO, LOG_TAG, "Failed to launch a Service app.");

}

// 진동을 일정 시간에 주는 함수. 2의 배수일 때 마다 진동 발생. 그 외는 stop.
static void
dynamic_vibrate(appdata_s *ad)
{
 if( ad->effect_handle != NULL )
	 device_haptic_stop(ad->handle, &ad->effect_handle);
 if( (ad->timer_count % 2) == 0 )
	 device_haptic_vibrate(ad->handle, ad->timer_dynamic, 100, &ad->effect_handle);
}

// 타이머 callback 함수.
static Eina_Bool
timer1_cb(void *data EINA_UNUSED)
{
 appdata_s *ad = data;
 ad->timer_count++;
 dynamic_vibrate(ad);
 // 7보다 크면 완전정지.
 if(ad->timer_count > ad->timer_full)
 {
 ecore_timer_del(ad->timer1);
 // 타이머를 NULL로 만들어버린다.
 ad->timer1 = NULL;
 }
 return ECORE_CALLBACK_RENEW;
}

// 진동 멈추는 callback 함수.
void
vibrate_stop(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	// 진동 멈추는 함수.
	int error = device_haptic_stop(ad->handle, &ad->effect_handle);

	if(error == 0) {
		dlog_print(DLOG_INFO, LOG_TAG, "vibrate stop success");
	}
	if(ad->timer_full != 0) {
			ad->timer_count = ad->timer_full + 1;
		}
	/* Let window go to hide state. */
	elm_naviframe_item_pop(ad->naviframe); // pop할떈 naviframe 인자로 넣자
	//elm_win_lower(ad->win);
	if(device_power_wakeup(true) == 0)
		dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep !");
	device_power_request_lock(POWER_LOCK_DISPLAY_DIM, 1);
	dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep 2 !");
}

// 진동 발생하는 callback 함수
/*
 * 진동 발생하는 callback 함수(타입 0)
 * @param timer : 진동 지속시간
 */
static void
sound_alert_vibrate(void *data, int timer)
{
appdata_s *ad = data;
dlog_print(DLOG_INFO, LOG_TAG, "callback");
// 진동을 발생시키는 함수. 1번째 : handle. 2번째 : 진동 유지기간. 3번째: 진동 세기(0~100). 4번째: 효과주는 handle.
int error = device_haptic_vibrate(ad->handle, timer, 100, &ad->effect_handle);
	if(error == 0) {
		dlog_print(DLOG_INFO, LOG_TAG, "vibrate success");
	}
}

/*
 * 반복 진동을 위한 callback 함수(타입 1)
 * @param timer_full : 횟수
 * @param timer_dynamic : 주기
 */
static void
sound_alert_dynamic_vibrate(void *data, int timer_full, int timer_dynamic)
{
 appdata_s *ad = data;
 // 초기 타이머 0으로 설정
 ad->timer_count = 0;
 ad->timer_full = timer_full;
 ad->timer_dynamic = timer_dynamic;
 dlog_print(DLOG_INFO, LOG_TAG, "dynamic timer setting success ");
 if (ad->timer1)
 ecore_timer_del(ad->timer1);
 // 0.5초마다 타이머 callback 함수 실행 설정.
 ad->timer1 = ecore_timer_add((double)(timer_dynamic / 1000), timer1_cb, ad); // double 되나?
 ad->effect_handle = NULL;
 dynamic_vibrate(ad);
}

// index에 맞게 알람을 화면에 출력해주는 전체적인 함수.
static void
sound_recognition_display(int index)
{
	//Evas_Object *popup;

	if(device_power_wakeup(false) == 0)
		dlog_print(DLOG_DEBUG, LOG_TAG, "device wakeup !");

	/*
	 * Image layout 객체 생성
	 */
	global_ad->img_layout = elm_layout_add(global_ad->naviframe);
	elm_layout_file_set(global_ad->img_layout, EDJ_ABSOLUTE_FILE, LAYOUT);
	evas_object_size_hint_weight_set(global_ad->img_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(global_ad->img_layout, "layout", "application", "default");
	//evas_object_show(global_ad->img_layout);

	/*
	 * Icon layout 객체 생성 및 대입
	 */
	global_ad->icon = elm_image_add(global_ad->img_layout);
	//elm_layout_file_set(global_ad->icon, EDJ_ABSOLUTE_FILE, LAYOUT);

	// 진동
	// 타입 확인(0인 경우)
	if(vibrate_pattern[index][0] == 0) {
		sound_alert_vibrate(global_ad, vibrate_pattern[index][1]);
	}
	// 타입 확인(1인 경우)
	if(vibrate_pattern[index][0] == 1) {
		sound_alert_dynamic_vibrate(global_ad, vibrate_pattern[index][2], vibrate_pattern[index][3]);
	}
	// display
	elm_image_file_set(global_ad->icon, soundIcon[index], NULL);
	evas_object_smart_callback_add(global_ad->icon, "clicked", vibrate_stop, global_ad);
	eext_object_event_callback_add(global_ad->icon, EEXT_CALLBACK_BACK, vibrate_stop, global_ad);
	evas_object_smart_callback_add(global_ad->icon, "delete,request", vibrate_stop, global_ad);
	elm_object_content_set(global_ad->img_layout, global_ad->icon);
	elm_naviframe_item_push(global_ad->naviframe, NULL, NULL, NULL, global_ad->img_layout,"empty");
	evas_object_show(global_ad->img_layout);
	evas_object_show(global_ad->icon);
	dlog_print(DLOG_DEBUG, LOG_TAG, "img setting success !");
	/*
	if(strcmp(path, "cry") == 0) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Cry !");
		elm_image_file_set(global_ad->icon, color, NULL);
		//elm_layout_file_set(global_ad->icon, CRY, NULL);
		sound_recog_vibrate(global_ad, 2000);
	}
	else if(strcmp(path, "car") == 0) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Car !");
		elm_image_file_set(global_ad->icon, CAR, NULL);
		sound_recog_dynamic_vibrate(global_ad, 0.5, 8);
	}
	else if(strcmp(path, "bell") == 0) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Bell !");
		elm_image_file_set(global_ad->icon, BELL, NULL);
		sound_recog_vibrate(global_ad, 5000);
	}
	else if(strcmp(path, "siren") == 0) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "Siren !");
		elm_image_file_set(global_ad->icon, SIREN, NULL);
		sound_recog_dynamic_vibrate(global_ad, 0.25, 8);
	}
	*/
}

/*
// 소리 듣는 함수.
static void
_start_recog_service(void *data , Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;
	dlog_print(DLOG_INFO, LOG_TAG, "11");

	ad->img = elm_image_add(ad->naviframe);
	//elm_layout_file_set(ad->img, EDJ_ABSOLUTE_FILE, "test/main_layout");
	elm_image_file_set(ad->img, ON_IMG_DIR, NULL);
	//elm_object_style_set(layout, "circle");
	evas_object_size_hint_weight_set(ad->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->naviframe, ad->img);
	switch_on_off(ad, 0);

	service_data_send(ad, "start", "start");

		//app_control_destroy(app_control);
}
*/

/*
 * 소리 중지 함수
 */
/*
static void
_stop_recog_service(void *data , Evas_Object *obj, void *event_info) {
	appdata_s *ad = data;

	ad->img = elm_image_add(ad->naviframe);
	//elm_layout_file_set(ad->img, EDJ_ABSOLUTE_FILE, "test/main_layout");
	elm_image_file_set(ad->img, OFF_IMG_DIR, NULL);
	//elm_object_style_set(layout, "circle");
	evas_object_size_hint_weight_set(ad->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->naviframe, ad->img);
	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->img,"empty");
	switch_on_off(ad, 1);
	evas_object_show(ad->img);

	service_data_send(ad, "stop", "stop");
}
*/

/*
 * on/off 함수
 */
/*
void
switch_on_off(void *data, gboolean on_off_click) {
	appdata_s *ad = data;
	if(on_off_click)
		evas_object_smart_callback_add(ad->img, "clicked", _start_recog_service, ad);
	else
		evas_object_smart_callback_add(ad->img, "clicked", _stop_recog_service, ad);
}
*/

// 들은 소리를 분석하는 함수.
static int
sound_recog_sort(char *path) {
	char *ptr = strtok(path, "\\");    				// " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
	char *temp;										// 맨 마지막 문자
	while (ptr == strtok(NULL, "\\"))                // 자른 문자열이 나오지 않을 때까지 반복
		{
			dlog_print(DLOG_INFO, LOG_TAG, ptr);	// 문자열 출력
			temp = ptr; // 이러면 맨 마지막 문자가 출력됨
		}
	temp = strtok(temp, ".");						// 파일이름을 가져와서 분류
	int length = sizeof(temp) / sizeof(temp[0]);
	for(int i = 0; i < length; i++) {
		if(strcmp(soundName[i], temp) == 0) {
			return i;
		}
	}
	return -1;

	/*
	if(strstr(temp, "baby")) {

	}
	if(strstr(temp, "siren")) {

	}
	if(strstr(temp, "bell")) {

	}
	if(strstr(temp, "crush")) {

	}
	*/
	/*
	else if(strstr(path, "4.txt")) {
		dlog_print(DLOG_INFO, LOG_TAG, " # 4.txt ");
		sleep(1);
		sound_recog_img_setting("siren");
	}
	else {
		dlog_print(DLOG_INFO, LOG_TAG, " # null. ");
	}
	*/
}

/*
 * 음성인식 시작
 */
void
sound_recognition(char *value) {
	// 인덱스 설정
	soundFlag = sound_recog_sort(value);
	// 예외처리(존재 X)
	if(soundFlag == -1) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "저장되지 않은 소리 알림!");
		return;
	}

}

/*
 * 서비스에서 소리 듣고 데이터 받는 callback 함수
 */
bool
_app_control_extra_data_cb(app_control_h app_control, const char *key, void *data)
{
int ret;
char *value;
ret = app_control_get_extra_data(app_control, key, &value);
dlog_print(DLOG_DEBUG, LOG_TAG, "%s - %s : %s", __func__, key, value);
if(ret == 0) {
	if(strcmp(key, "recognition") == 0) { // key == recognition 면
		dlog_print(DLOG_DEBUG, LOG_TAG, "sound recognition scaned !");
		sound_recognition(value); // 음성인식 알람 진행 함수 출력
	}
}
return true;
}

static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

void show_file_req_popup(void)
{
//	popup_title_text_check_button(global_ad->naviframe, NULL, NULL);
}
void hide_file_req_popup(void)
{
	elm_popup_dismiss(global_ad->popup);
}

/* naviframe의 최상단 stack item pop 시키기. */
void
naviframe_pop_cb(void *data, Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, " # naviframe pop cb ");
    Evas_Object *naviframe = data;
    elm_naviframe_item_pop(naviframe);
}

/*
 * 팝업 메세지 띄우는 함수
 */
void
_popup_message(char *string) {
	show_message_popup(global_appdata->naviframe, string);
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	elm_win_lower(ad->win);
}

/*
 * splash timeout 함수
 */
static Eina_Bool
_timeout(void *data EINA_UNUSED)
{
	appdata_s *ad = (appdata_s *)data;
	evas_object_hide(ad->splash_layout);
	elm_naviframe_item_pop(ad->splash_layout);
	dlog_print(DLOG_INFO, LOG_TAG, "# timeout ");
	/* Main Layout */
	_create_main_layout_start(ad, NULL, NULL);

	// 서비스 시작(planB)
	service_data_send(ad, "start", "start");
	ad->splashtimer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

/*
 * splash 화면 시작 함수
 */
static void start_splash(appdata_s *ad)
{

	dlog_print(DLOG_INFO, LOG_TAG, "# start splash ");

	ad->splash_layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(ad->splash_layout, EDJ_ABSOLUTE_FILE, SPLASH_LAYOUT);
	// elm_object_style_set(layout, "circle");
	evas_object_size_hint_weight_set(ad->splash_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->naviframe, ad->splash_layout);
	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->splash_layout,"empty");
	// elm_object_content_set(popup, layout);

	evas_object_show(ad->splash_layout);

	ad->splashtimer = ecore_timer_add(2.0, _timeout, ad);
	/*
	if (timer1)
	  {
	     ecore_timer_del(timer1);
	     timer1 = NULL;
	     dlog_print(DLOG_INFO, LOG_TAG, "# splash timer delete ");
	  }
	dlog_print(DLOG_INFO, LOG_TAG, "# splash end ");
	*/
}

static void create_base_gui(appdata_s *ad)
{

	dlog_print(DLOG_INFO, LOG_TAG, "1");
	/* Window */
		ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
		elm_win_conformant_set(ad->win, EINA_TRUE);
		elm_win_autodel_set(ad->win, EINA_TRUE);

		if (elm_win_wm_rotation_supported_get(ad->win)) {
			int rots[4] = { 0, 90, 180, 270 };
			elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
		}

		evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, ad);
		dlog_print(DLOG_INFO, LOG_TAG, "# window setting success. ");
	/* Conformant */
		ad->conform = elm_conformant_add(ad->win);
		evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(ad->win, ad->conform);
		evas_object_show(ad->conform);
		dlog_print(DLOG_INFO, LOG_TAG, "# conformant setting success. ");
	/* Base Layout */
		ad->layout = elm_layout_add(ad->conform);
		elm_layout_file_set(ad->layout, EDJ_ABSOLUTE_FILE, LAYOUT);
		evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_theme_set(ad->layout, "layout", "application", "default");
		eext_object_event_callback_add(ad->layout, EEXT_CALLBACK_BACK, layout_back_cb, ad);
		evas_object_show(ad->layout);

		elm_object_content_set(ad->conform, ad->layout);
		dlog_print(DLOG_INFO, LOG_TAG, "# base layout setting success. ");

	/* naviframe */
	ad->naviframe = elm_naviframe_add(ad->layout);
	evas_object_size_hint_weight_set(ad->naviframe, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_object_part_content_set(ad->layout, "elm.swallow.content", ad->naviframe);

	//evas_object_show(ad->naviframe);
	dlog_print(DLOG_INFO, LOG_TAG, "# naviframe setting success. ");

	/*
	 * 이미지 화면 출력 함수 -> 나중에 쓸 수 있을듯
	ad->img = elm_image_add(ad->naviframe);
	elm_image_file_set(ad->img, OFF_IMG_DIR, NULL);
	//elm_object_style_set(layout, "circle");
	evas_object_size_hint_weight_set(ad->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->naviframe, ad->img);
	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->img,"empty");

	evas_object_show(ad->img);
	*/

	// 팝업 설정
	global_appdata = ad;

	/* Splash */
	start_splash(ad);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
	device_haptic_open(0, &ad->handle);
	global_ad = ad;
	//switch_on_off(global_ad, 1);
}

static bool app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	        Initialize UI resources and application's data
	        If this function returns true, the main loop of application starts
	        If this function returns false, the application is terminated */
	appdata_s *ad = data;
	create_base_gui(ad);

	return true;
}

static void app_control(app_control_h app_control, void *data)
{
	app_control_foreach_extra_data(app_control, _app_control_extra_data_cb, NULL);
	/* Handle the launch request. */
}

static void app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void app_terminate(void *data)
{
	/* Release all resources. */
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[])
{
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
