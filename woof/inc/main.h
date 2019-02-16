#ifndef __MAIN_H__
#define __MAIN_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <glib.h>
#include <device/haptic.h>
#include <device/power.h>
#include <stt.h>
#include <efl_util.h>
#include <efl_extension_events.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "WoofLog"

#if !defined(PACKAGE)
#define PACKAGE "org.woof"
#endif
#define EDJ_ABSOLUTE_FILE "/opt/usr/apps/org.woof/res/edje/main.edj"
#define LAYOUT "woof/noti_image_layout"
#define CRUSH "/opt/usr/apps/org.woof/res/icon/crush1.png"
#define BABY "/opt/usr/apps/org.woof/res/icon/baby1.png"
#define SIREN "/opt/usr/apps/org.woof/res/icon/fire4.png"
#define DOOR "/opt/usr/apps/org.woof/res/icon/door6.png"
#define APART "/opt/usr/apps/org.woof/res/icon/siren2.png"

#define SRC_PATH "/opt/usr/media/Downloads/woof"
#define DOWNLOAD_PATH "/opt/usr/media/Downloads"

#define ON_IMG_DIR "/opt/usr/apps/org.woof/res/mainimages/ON.png"
#define OFF_IMG_DIR "/opt/usr/apps/org.woof/res/mainimages/OFF.png"
#define POPUP_PROGRESSBAR "popup_progressbar"
#define SPLASH_LAYOUT "woof/splash_layout"
#define MAIN_LAYOUT "woof/index_thumbnail"
#define MAINIMAGE_DIR "/opt/usr/apps/org.woof/res/mainimages"
#define ICON_DIR "/opt/usr/apps/org.woof/res/images"
#define SUB_LAYOUT "sub_layout"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *naviframe;
	Evas_Object *conform;
	Evas_Object *content;
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *icon;
	Evas_Object *layout;
	Evas_Object *main_layout;
	Evas_Object *main_img_layout;
	Evas_Object *img;
	Evas_Object *img_layout;
	Evas_Object* splash_layout;

	haptic_device_h handle;
	haptic_effect_h effect_handle;
	Ecore_Timer *splashtimer;
	Ecore_Timer *timer1;
	char* file_path;
	int timer_count;
	int timer_full;
	int timer_dynamic;
} appdata_s;

static appdata_s *global_ad;

// 팝업을 위한.
appdata_s *global_appdata;

/*
 * 진동 패턴 저장
 * [0] : 진동타입(0 : 반복없이 쭈욱(default), 1 : 반복)
 * [1] : 진동시간(타입이 0일 경우는 시간, 1일 경우는 0)
 * [2] : 진동횟수(타입이 0일 경우는 0, 1일 경우는 횟수)
 * [3] : 진동간격(타입이 0일 경우는 0, 1일 경우는 주기)
 * 진동 세기는 100으로 통일
 */
/*
typedef struct Pattern {


	int pattern1[4];
	int pattern2[4];
	int pattern3[4];
	int pattern4[4];
	int pattern5[4];
	int pattern6[4];
	int pattern7[4];
	int pattern8[4];
	int pattern9[4];
	int pattern10[4];

} pattern;
static pattern vibrate_pattern;
*/

#define SOUND_INDEX 5
#define RES_PATH "/opt/usr/apps/org.woof/res"
int vibrate_pattern[5][4];
char* sound_list[SOUND_INDEX];
char* sound_file_list[SOUND_INDEX];
char* image_list[SOUND_INDEX];
char* icon_image_path[SOUND_INDEX];

void _create_main_layout_start(void *data, Evas_Object *obj, void *event_info);
void _popup_small_process_loading(void *data, Evas_Object *obj, char *string, void *event_info);
void show_message_popup(Evas_Object *parent, char *string);
void _popup_message(char *string);
void show_file_req_popup();
void hide_file_req_popup();
Eina_Bool sound_recognition_func(void *data);
void vibrate_stop(void *data, Evas_Object *obj, void *event_info);
void service_data_send(void *data, char *key, char *value);
char* read_file(const char* file_path);
#endif /* __MAIN_H__ */
