#include <main.h>
// 음성인식 초기 값
int soundFlag = -1;
Ecore_Timer* file_save_timer;
char* sound_list[SOUND_INDEX] = {"crush", "siren", "baby", "door", "apart"};
char* sound_file_list[SOUND_INDEX] = {CRUSH, SIREN, BABY, DOOR, APART};
char* image_list[SOUND_INDEX];
char* icon_image_path[SOUND_INDEX];

/*
 * sound alarm array
 */
gboolean enable[4];
int vibrate_pattern[5][4] = {
							{0, 20000, 0, 0},
							{0, 20000, 0, 0},
							{1, 0, 20, 50000},
							{1, 0, 20, 50000},
							{0, 10000, 0, 0}
							};

int vibrate_pattern2[5][4] = {
							{0, 20000, 0, 0},
							{0, 20000, 0, 0},
							{0, 20000, 0, 0},
							{0, 20000, 0, 0},
							{0, 20000, 0, 0}
							};

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

/*
 * 서비스 시작하는 함수
 */
void
service_data_send(void *data, char *key, char *value) {

	appdata_s *ad = data;
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
	app_control_add_extra_data(app_control, key, value);
	app_control_set_app_id (app_control, "org.woof.app2"); // id값 잘 수정하기

	if (app_control_send_launch_request(app_control, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "Succeeded to launch a Service app.");
		//elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->img,"empty"); // ?
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
	//evas_object_hide(ad->img_layout);
	//evas_object_hide(ad->img);
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

static void
layout_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	// Let window go to hide state.
	// elm_naviframe_item_pop(ad->naviframe); // pop할떈 naviframe 인자로 넣자
	elm_win_lower(ad->win);
	if(device_power_wakeup(true) == 0)
		dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep !");
	device_power_request_lock(POWER_LOCK_DISPLAY_DIM, 1);
	device_haptic_stop(ad->handle, &ad->effect_handle);
	// dlog_print(DLOG_DEBUG, LOG_TAG, "device dim sleep 2 !");
}

/*
 * 소리 이름 배열에서 맞는 인덱스 찾아 리턴
 */
static int
get_index_sound_array(const char *name) {
		for(int i = 0; i < SOUND_INDEX; i++) {
			if(strcmp(sound_list[i], name) == 0) {
				return i;
			}
		}
		return -1;
}

// index에 맞게 알람을 화면에 출력해주는 전체적인 함수.
static void
sound_recognition_display(int index)
{
	//Evas_Object *popup;

//	if(device_power_wakeup(false) == 0)
	//	dlog_print(DLOG_DEBUG, LOG_TAG, "device wakeup !");

	dlog_print(DLOG_DEBUG, LOG_TAG, "%d is index", index);
	dlog_print(DLOG_DEBUG, LOG_TAG, "%s is path", icon_image_path[index]);
	// Image layout 객체 생성
	global_ad->img_layout = elm_layout_add(global_ad->main_layout);
	elm_layout_file_set(global_ad->img_layout, EDJ_ABSOLUTE_FILE, LAYOUT);
	//evas_object_size_hint_weight_set(global_ad->img_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_weight_set(global_ad->img_layout, 0.5, 0.5);
	elm_layout_theme_set(global_ad->img_layout, "layout", "application", "default");
	//elm_object_content_set(global_ad->naviframe, global_ad->img_layout);
	//evas_object_show(global_ad->img_layout);

	// Icon layout 객체 생성 및 대입
	dlog_print(DLOG_DEBUG, LOG_TAG, "icon layout !");
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
	dlog_print(DLOG_DEBUG, LOG_TAG, "display !");
	//elm_image_file_set(global_ad->icon, icon_image_path[index], NULL);
	elm_image_file_set(global_ad->icon, sound_file_list[index], NULL);
	//elm_bg_file_set(global_ad->img_layout, icon_image_path[index], LAYOUT);
	evas_object_smart_callback_add(global_ad->icon, "clicked", vibrate_stop, global_ad);			// click
	eext_object_event_callback_add(global_ad->icon, EEXT_CALLBACK_BACK, vibrate_stop, global_ad);	// back
	evas_object_smart_callback_add(global_ad->icon, "delete,request", vibrate_stop, global_ad);		// exit
	elm_object_content_set(global_ad->img_layout, global_ad->icon);
	elm_naviframe_item_push(global_ad->naviframe, NULL, NULL, NULL, global_ad->img_layout,"empty");
	evas_object_show(global_ad->img_layout);
	evas_object_show(global_ad->icon);
	dlog_print(DLOG_DEBUG, LOG_TAG, "img setting success !");
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

// 파일의 이름을 가져오는 함수(확장명도).
char*
get_file_all_name(const char *path) {
	// char* not_const_path = (char *)path;
	char* not_const_path = malloc(sizeof(char) * 100);    // char 100개 크기만큼 동적 메모리 할당

	strcpy(not_const_path, path);    // s1에 문자열 복사
	char* ptr = strtok(not_const_path, "\\/");    			// " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
	char *temp;										// 맨 마지막 문자
	while (ptr != NULL)              // 자른 문자열이 나오지 않을 때까지 반복
		{
			temp = ptr; // 이러면 맨 마지막 문자가 출력됨
			ptr = strtok(NULL, "\\/");
		}
	free(not_const_path);
	// temp = strtok(temp, ".");						// 파일이름을 가져와서 분류
	dlog_print(DLOG_INFO, LOG_TAG, temp);
	return temp; 									// 파일이름과 확장명 리턴
}

/*
 * 파일 읽는 함수
 * @param : file_path(const char*)
 * @return : context(char *)
 */
char*
read_file(const char* file_path) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "file reading ..");
	FILE *fp = fopen(file_path, "r");
	if(fp)
		return "not downloading";
	fseek(fp, 0, SEEK_END);
	int bufsize = ftell(fp); // 파일 크기 반환
    rewind(fp); // 파일 스트림 처음으로 되돌림
    if (bufsize < 1)
        return "setting nothing";
    char *buf = malloc(sizeof(char) * (bufsize));	// size만큼 동적 배열
    //char str[7][256];	// 255 바이트로 되나?
    char str[256];
    fgets(str, 256, fp);
    sprintf(buf, "%s", str);
    dlog_print(DLOG_ERROR, LOG_TAG, str);
    fclose(fp);
    return buf;
}

/*
 * txt파일 데이터를 split하는 함수
 * [0]: 음성인식 / 파일전송 여부(T/F) [1]: bitmap 형식의 코드
 */
char*
split_context(char *context) {
	// char* not_const_path = (char *)path;
	char *isRecog = strtok(context, "_");    			// "_" 문자를 기준으로 문자열을 자름, 포인터 반환
	char *bitmap_context = strtok(NULL, "_");    			// "_" 문자를 기준으로 문자열을 자름, 포인터 반환
//	while (ptr != NULL)              // 자른 문자열이 나오지 않을 때까지 반복
//		{
//			temp = ptr; // 이러면 맨 마지막 문자가 출력됨
//			ptr = strtok(NULL, "\\/");
//		}
//	free(not_const_path);
	// temp = strtok(temp, ".");						// 파일이름을 가져와서 분류
	dlog_print(DLOG_INFO, LOG_TAG, isRecog);
	dlog_print(DLOG_INFO, LOG_TAG, bitmap_context);
	// 만약 recognition이 맞다면
	if(strstr(isRecog, "true"))
		return bitmap_context;
	// 파일전송이면
	else
		return "transfer";
}

/*
 * bytearray to bitmap
 * return: string
 */
char*
byte_to_String(char *byte_arr) {
	dlog_print(DLOG_DEBUG, LOG_TAG, byte_arr);
	size_t length = sizeof(byte_arr) / sizeof(char);//best use ptr's type -> good habit
	dlog_print(DLOG_DEBUG, LOG_TAG, "%d is length", length);
	char* bitstr = malloc(sizeof(char) * (length * 8));	// size만큼 동적 배열;
	//unsigned char mask = 1;
	char bits[8];

	for (int i = 0; *(byte_arr + i) != 0; i++) {
		int byte_size = *(byte_arr + i);
		dlog_print(DLOG_DEBUG, LOG_TAG, "byte to string5");
		dlog_print(DLOG_DEBUG, LOG_TAG, "%d is i", i);
		dlog_print(DLOG_DEBUG, LOG_TAG, "%d is byte_arr + i", byte_size);

//		char byte_char = *(byte_arr + i);
//		dlog_print(DLOG_INFO, LOG_TAG, "%s is byte char", byte_char);
//		dlog_print(DLOG_DEBUG, LOG_TAG, "byte to string6");
//		dlog_print(DLOG_DEBUG, LOG_TAG, "byte to string7");
		for (int j = 0; j < 8; j++) {
			dlog_print(DLOG_DEBUG, LOG_TAG, "byte to string6");
			// bits[j] = (byte_size & (mask << j));
//			bitstr.append(byte_size % 2);
			bits[j] = byte_size % 2;
			byte_size = byte_size / 2;
			// bitstr = (bytestr.at(i) mod 2) + bitstr; // little endian? big endian?
			dlog_print(DLOG_INFO, LOG_TAG, "%s is bits[%d]", bits[j], j);
		}
		sprintf(bitstr, "%s", bits);
//		if(i == 0)
//			sprintf(bitstr, "%s", bits);
//		else
//			sprintf(bitstr + strlen(bitstr), "%s", bits);
	}
	dlog_print(DLOG_INFO, LOG_TAG, bitstr);
	return bitstr;
}

/*
 * 음성 인식 시작
 * @param file_name: 파일 이름
 * @param bitmap_context: 비트맵 문자열 코드
 */
void
sound_recognition_start(char *file_name, char *bitmap_context) {
	dlog_print(DLOG_DEBUG, LOG_TAG, "sound recognition start");
	byte_to_String(bitmap_context);
}

/*
 * 파일 파싱 함수
 */
void
parsing_file_text(char* file_name, const char *value) {
	char* context = NULL;
	char* result = NULL;
	context = read_file(value);	// 파일 읽어서 문자열 가져오기
	dlog_print(DLOG_DEBUG, LOG_TAG, context);
	dlog_print(DLOG_DEBUG, LOG_TAG, "file context OK");
	result = split_context(context);
	// 음성 인식이면
	if(!strstr(result, "transfer")) {
		dlog_print(DLOG_DEBUG, LOG_TAG, "this is sound recognition");
		sound_recognition_start(file_name, result);
	}
	// 인덱스 설정
	// value 자체를 파일 이름으로 하자
//	soundFlag = sound_recog_sort(value);
	// _popup_message(soundName[soundFlag]);
//	// 예외처리(존재 X)
//	if(soundFlag == -1) {
//		dlog_print(DLOG_DEBUG, LOG_TAG, "저장되지 않은 소리 알림!");
//		return;
//	}
	// sound_recognition_display(soundFlag);
}

/*
 * txt 설정파일 데이터를 split하는 함수
 * [0]: baby [1]: bitmap 형식의 코드 [2]: siren  [3]:
 */
char*
split_setting_file_context(char *context) {
	// char* not_const_path = (char *)path;
	char *isRecog = strtok(context, "_");    			// "_" 문자를 기준으로 문자열을 자름, 포인터 반환
	char *bitmap_context = strtok(NULL, "_");    			// "_" 문자를 기준으로 문자열을 자름, 포인터 반환
//	while (ptr != NULL)              // 자른 문자열이 나오지 않을 때까지 반복
//		{
//			temp = ptr; // 이러면 맨 마지막 문자가 출력됨
//			ptr = strtok(NULL, "\\/");
//		}
//	free(not_const_path);
	// temp = strtok(temp, ".");						// 파일이름을 가져와서 분류
	dlog_print(DLOG_INFO, LOG_TAG, isRecog);
	dlog_print(DLOG_INFO, LOG_TAG, bitmap_context);
	// 만약 recognition이 맞다면
	if(strstr(isRecog, "true"))
		return bitmap_context;
	// 파일전송이면
	else
		return "transfer";
}

/*
 * 설정 배열에 설정하기
 */
void
setting_sound_icon_path() {
	char *res_path = malloc(sizeof(char) * 150);	// setting res file path
	char *transfer_path = malloc(sizeof(char) * 150);	// setting transfer file path
	char *transfer_folder_path = "/opt/usr/media/Downloads/woof";
	dlog_print(DLOG_INFO, LOG_TAG, "setting sound icon path");
	// dlog_print(DLOG_INFO, LOG_TAG, "%d is SOUND_INDEX", SOUND_INDEX);
	for(int i = 0; i < SOUND_INDEX; i++) {
		if(image_list[i] == NULL) {
			continue;
		}
		else {
			dlog_print(DLOG_INFO, LOG_TAG, "# setting index file name : %s", image_list[i]);
			// transfer
			if((strcmp(image_list[i], "crush") == 0) || (strcmp(image_list[i], "baby") == 0)
				|| (strcmp(image_list[i], "siren") == 0) || (strcmp(image_list[i], "door") == 0)
				|| (strcmp(image_list[i], "apart") == 0)) {
				sprintf(transfer_path, "%s/icon/%s.%s", transfer_folder_path, image_list[i], "png");
				icon_image_path[i] = transfer_path;
				dlog_print(DLOG_INFO, LOG_TAG, "# transfer path : %s", icon_image_path[i]);
			}
			// normal
			else {
				sprintf(res_path, "%s/icon/%s.%s", RES_PATH, image_list[i], "png");
				icon_image_path[i] = res_path;
				dlog_print(DLOG_INFO, LOG_TAG, "# res path : %s", icon_image_path[i]);

			}
		}
	}
	dlog_print(DLOG_INFO, LOG_TAG, "# setting success !!");

}

/*
 * transfer 설정하기
 */
void
setting_sound_icon_path_transfer(int i) {
	char *res_path = malloc(sizeof(char) * 150);	// setting res file path
	char *transfer_path = malloc(sizeof(char) * 150);	// setting transfer file path
	char *transfer_folder_path = "/opt/usr/media/Downloads/woof";
	dlog_print(DLOG_INFO, LOG_TAG, "setting sound icon path");
	// transfer
	if((strcmp(image_list[i], "crush") == 0) || (strcmp(image_list[i], "baby") == 0)
		|| (strcmp(image_list[i], "siren") == 0) || (strcmp(image_list[i], "door") == 0)
		|| (strcmp(image_list[i], "apart") == 0)) {
		sprintf(transfer_path, "%s/icon/%s.%s", transfer_folder_path, image_list[i], "png");
		icon_image_path[i] = transfer_path;
		dlog_print(DLOG_INFO, LOG_TAG, "# transfer path : %s", icon_image_path[i]);
	}
	// normal
	else {
		sprintf(res_path, "%s/icon/%s.%s", RES_PATH, image_list[i], "png");
		icon_image_path[i] = res_path;
		dlog_print(DLOG_INFO, LOG_TAG, "# res path : %s", icon_image_path[i]);
	}
	dlog_print(DLOG_INFO, LOG_TAG, "# setting success !!");
}

/*
 * 설정 파일 내용 분류
 */
void
classify_setting_context(char *context) {
	dlog_print(DLOG_INFO, LOG_TAG, "classify setting context");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is context", context);
	// 다섯 종류의 소리
	char* crush = strtok(context, "_");    			// crush
	char* siren = strtok(NULL, "_");    			// siren
	char* baby = strtok(NULL, "_");    			// baby
	char* door = strtok(NULL, "_");    			// door
	char* apart = strtok(NULL, "_");    			// apart

	//char* crush_file_name = strtok(crush, "_");
	char* temp1 = strtok(crush, ":");
	image_list[0] = strtok(NULL, ":");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is crush.", image_list[0]);
	char* temp2 = strtok(siren, ":");
	image_list[1] = strtok(NULL, ":");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is siren.", image_list[1]);
	char* temp3 = strtok(baby, ":");
	image_list[2] = strtok(NULL, ":");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is baby.", image_list[2]);
	char* temp4 = strtok(door, ":");
	image_list[3] = strtok(NULL, ":");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is door.", image_list[3]);
	char* temp5 = strtok(apart, ":");
	image_list[4] = strtok(NULL, ":");
	dlog_print(DLOG_INFO, LOG_TAG, "%s is apart.", image_list[4]);
	// free(crush);
	// free(siren);
	// free(baby);
	// free(door);
	// free(apart);
}

/*
 * 초기 사운드 세팅 함수
 */
static char*
get_setting_context() {
	char* res_path = RES_PATH;
		char *rcv_file_path = malloc(sizeof(char) * 150);	// setting file path
		sprintf(rcv_file_path, "%s/%s", res_path, "setting.txt");
		dlog_print(DLOG_INFO, LOG_TAG, "[v] file path is %s", rcv_file_path);
		FILE *fp = fopen(rcv_file_path, "r");
			if(fp == NULL) {
				dlog_print(DLOG_INFO, LOG_TAG, "[v] file is NULL");
				return NULL;
			}

		fseek(fp, 0, SEEK_END);
		int bufsize = ftell(fp);
		rewind(fp);
		    return NULL;
		char *buf = malloc(sizeof(char) * (bufsize));
		char str[256];
		fgets(str, 256, fp);
		sprintf(buf, "%s", str);
		dlog_print(DLOG_ERROR, LOG_TAG, buf);
		dlog_print(DLOG_ERROR, LOG_TAG, str);
		fclose(fp);
		free(rcv_file_path);
		return buf;
}

/*
 * 파일 수신 시 해당 이미지 파일로 경로 변경
 */
bool
set_file_to_icon_path(int index, char *icon_name) {
	dlog_print(DLOG_INFO, LOG_TAG, "[v] index, icon name : {%d, %s}", index, icon_name);
	char* context = get_setting_context();
	dlog_print(DLOG_INFO, LOG_TAG, "[v] file context is %s", context);
	char* res_path = RES_PATH;
	char *rcv_file_path = malloc(sizeof(char) * 250);	// setting file path
	sprintf(rcv_file_path, "%s/%s", res_path, "setting.txt");
	dlog_print(DLOG_INFO, LOG_TAG, "[v] file path is %s", (const char *)rcv_file_path);

	// 권한
	if(chmod(res_path, 0777) == -1) {
				dlog_print(DLOG_INFO, LOG_TAG, strerror(errno));
			}
	if(chmod(rcv_file_path, 0777) == -1) {
		dlog_print(DLOG_INFO, LOG_TAG, strerror(errno));
	}

	FILE *fp2 = fopen((const char *)rcv_file_path, "w");
	if(!fp2) {
		dlog_print(DLOG_INFO, LOG_TAG, "[v] file is NULL");
		//return false;
	}
	if(fp2) {
		dlog_print(DLOG_INFO, LOG_TAG, "[v] file is not NULL");
	}
	// 다섯 종류의 소리
	char* sound[5];
	sound[0] = strtok(context, "_"); 	   				// crush
	dlog_print(DLOG_INFO, LOG_TAG, "[v] setting file context : 1");
	sound[1] = strtok(NULL, "_");    					// siren
	sound[2] = strtok(NULL, "_");    					// baby
	sound[3] = strtok(NULL, "_");    					// door
	sound[4] = strtok(NULL, "_");    					// apart
	dlog_print(DLOG_INFO, LOG_TAG, "[v] setting file context : 5");

	dlog_print(DLOG_INFO, LOG_TAG, "%s_%s_%s_%s_%s", sound[0], sound[1], sound[2], sound[3], sound[4]);

	char *temp = malloc(sizeof(char) * 150);		// setting file path
	char *result = malloc(sizeof(char) * 255);	// setting file path
	char *name = strtok(sound[index], ":");
	sprintf(temp, "%s:%s", name, icon_name);
	dlog_print(DLOG_INFO, LOG_TAG, "%d, %s", index, temp);
	sound[index] = temp;
	dlog_print(DLOG_INFO, LOG_TAG, "%s, %s", sound[index], temp);
	dlog_print(DLOG_INFO, LOG_TAG, "%s, %s is sound[index], temp", sound[index], temp);
	sprintf(result, "%s_%s_%s_%s_%s", sound[0], sound[1], sound[2], sound[3], sound[4]);
	dlog_print(DLOG_INFO, LOG_TAG, "[v] %s is result", result);
	rewind(fp2); // 파일 스트림 처음으로 되돌림
	fputs((const char *)result, fp2);
	fclose(fp2);
	return true;
}

/*
 * 아이콘 그림 파일 수신 시 진행하는 함수
 * 파일의 이름을 가지고 경로 변경
 * @param: file_name : 수신받은 파일의 이름
 */
void
setting_sound_icon_file_name(char *file_name) {
	dlog_print(DLOG_INFO, LOG_TAG, "# setting_sound_icon_file_name");
	char* sound_name = strtok(file_name, "_");    				// sound name
	char* icon_name = strtok(NULL, "_");    					// apart

	dlog_print(DLOG_INFO, LOG_TAG, "[v] sound name, icon name : {%s , %s}", sound_name, icon_name);

	int index = -1;		// sound array index
	/* get sound array index */
	index = get_index_sound_array(sound_name);
	// sound name is null
	if(index == -1) {
		_popup_message("invalid file name! not setting.");
	}
	else {
		icon_image_path[index] = icon_name;
		_popup_message("setting file write success!");
//		if(set_file_to_icon_path(index, icon_name)) {
//			dlog_print(DLOG_INFO, LOG_TAG, "[v] true");
//			_popup_message("setting file write success!");
//		}
//		else {
//			dlog_print(DLOG_INFO, LOG_TAG, "[v] false");
//			_popup_message("setting file write fail!");
//		}
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
	/* 제 1안 */
	if(strcmp(key, "path") == 0) { // key == path 면
		dlog_print(DLOG_DEBUG, LOG_TAG, "file path check in Activity !");
		// _popup_message(value);		// 테스트
		dlog_print(DLOG_DEBUG, LOG_TAG, value);
		char *file_name = get_file_all_name((const char *)value);
		parsing_file_text(file_name, (const char *)value); 	// 음성인식 알람 진행 함수 출력
	}
	/* 제 2안 */
	if(strcmp(key, "recognition") == 0) {	// 소리 인식
		dlog_print(DLOG_DEBUG, LOG_TAG, "get file name for sound recognition : %s", value);
		// _popup_message(value);		// 테스트
		int index = get_index_sound_array((const char *)value);
		sound_recognition_display(index);
		return true;

	}
	if(strcmp(key, "transfer") == 0) { // 파일 설정
		dlog_print(DLOG_DEBUG, LOG_TAG, "get file name for sound icon setting : %s", value);
		setting_sound_icon_file_name(value);
		return true;
	}
	/* 제 3안 */
	if(strcmp(key, "popup") == 0) {
		_popup_message(value);
	}
}
return false;
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
	show_message_popup(global_ad->naviframe, string);
}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	elm_win_lower(ad->win);
}

/*
 * main layout 함수
 */
void
main_layout(void *data EINA_UNUSED) {
	appdata_s *ad = (appdata_s *)data;
	/* Main Layout */
	// Image layout 객체 생성
	global_ad->main_layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(ad->main_layout, EDJ_ABSOLUTE_FILE, LAYOUT);
	evas_object_size_hint_weight_set(ad->main_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(ad->main_layout, "layout", "application", "default");

	// Icon layout 객체 생성 및 대입
	dlog_print(DLOG_DEBUG, LOG_TAG, "icon layout !");
	ad->main_img_layout = elm_image_add(ad->main_layout);
	//elm_layout_file_set(global_ad->icon, EDJ_ABSOLUTE_FILE, LAYOUT);

	// display
	dlog_print(DLOG_DEBUG, LOG_TAG, "display !");
	elm_image_file_set(ad->main_img_layout, ON_IMG_DIR, NULL);
	//elm_bg_file_set(global_ad->img_layout, icon_image_path[index], LAYOUT);
	//evas_object_smart_callback_add(ad->main_img_layout, "clicked", vibrate_stop, ad);			// click
	eext_object_event_callback_add(ad->main_img_layout, EEXT_CALLBACK_BACK, layout_back_cb, ad);	// back
	evas_object_smart_callback_add(ad->main_img_layout, "delete,request", layout_back_cb, ad);		// exit
	elm_object_content_set(ad->main_layout, ad->main_img_layout);
	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->main_layout,"empty");
	evas_object_show(ad->main_layout);
	evas_object_show(ad->main_img_layout);

}
/*
 * splash timeout 함수
 */
static Eina_Bool
_timeout(void *data EINA_UNUSED)
{
	appdata_s *ad = (appdata_s *)data;
	// evas_object_hide(ad->splash_layout);
	// elm_naviframe_item_pop(ad->splash_layout);
	evas_object_del(ad->splash_layout);
	dlog_print(DLOG_INFO, LOG_TAG, "# timeout ");
	/* Main Layout */
	service_data_send(ad, "start", "start");
	main_layout(ad);

	//_create_main_layout_start(ad, NULL, NULL);

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
	evas_object_size_hint_weight_set(ad->splash_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->naviframe, ad->splash_layout);
	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->splash_layout,"empty");

	evas_object_show(ad->splash_layout);

	// sound icon path setting
	char *context = get_setting_context();
	dlog_print(DLOG_INFO, LOG_TAG, "# context %s", context);
		if(context != NULL) {
			classify_setting_context(context);
			dlog_print(DLOG_INFO, LOG_TAG, "### setting sound icon path");
			setting_sound_icon_path();
		}
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


//
//	// * 이미지 화면 출력 함수 -> 나중에 쓸 수 있을듯
//	ad->img = elm_image_add(ad->naviframe);
//	elm_image_file_set(ad->img, OFF_IMG_DIR, NULL);
//	// elm_image_memfile_set(obj, img, size, format, key)
//	//elm_object_style_set(layout, "circle");
//	evas_object_size_hint_weight_set(ad->img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
//	elm_object_content_set(ad->naviframe, ad->img);
//	elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, ad->img,"empty");

	evas_object_show(ad->img);


	// 팝업 설정
	// global_appdata = ad;
	// global_ad = ad;
	/* Show window after base gui is set up */
	evas_object_show(ad->win);
	device_haptic_open(0, &ad->handle);
	global_ad = ad;
	/* Splash */
	start_splash(global_ad); // 여차하면 다시 ad로

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
	bool result = app_control_foreach_extra_data(app_control, _app_control_extra_data_cb, NULL);
	dlog_print(DLOG_INFO, LOG_TAG, "# result is %s. ", result);
	if(result)
		dlog_print(DLOG_INFO, LOG_TAG, "# app_control_foreach_extra_data result is OK. ");
	else
		dlog_print(DLOG_INFO, LOG_TAG, "# app_control_foreach_extra_data result is NOT OK. ");
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
