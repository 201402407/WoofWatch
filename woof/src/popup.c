#include "main.h"

double POPUP_TIMER = 1.5;

static void
_popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if(!obj) return;
	elm_popup_dismiss(obj);
}

static void
_popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	if(!obj) return;
	evas_object_del(obj);
}

// 팝업 메세지 시간 다되면
static void
_message_popup_timeout(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG, " # message popup timeout ! ");
	evas_object_hide(obj);
}

// 팝업 메세지 띄우는 함수
void
show_message_popup(Evas_Object *parent, char *string)
{
	dlog_print(DLOG_INFO, LOG_TAG, " # show message popup ! ");
	Evas_Object *layout;
	Evas_Object *popup;

	popup = elm_popup_add(parent);
	// 팝업 시간 설정.
	elm_popup_timeout_set(popup, POPUP_TIMER);
	evas_object_smart_callback_add(popup, "timeout", _message_popup_timeout, NULL);
	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle");

	elm_object_part_text_set(layout, "elm.text", string);
	elm_object_content_set(popup, layout);

	evas_object_show(popup);
}


/*
 * 작은 원 로딩 팝업 출력
 */
void
_popup_small_process_loading(void *data, Evas_Object *obj, char *string, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG, " # popup progress loading ");
	Evas_Object *layout, *progressbar;
	appdata_s *ad = data;

	ad->popup = elm_popup_add(ad->naviframe);

	elm_object_style_set(ad->popup, "circle");

	eext_object_event_callback_add(ad->popup, EEXT_CALLBACK_BACK, _popup_hide_cb, NULL);

	evas_object_smart_callback_add(ad->popup, "dismissed", _popup_hide_finished_cb, NULL);


	layout = elm_layout_add(ad->popup);
	elm_layout_file_set(layout, EDJ_ABSOLUTE_FILE, POPUP_PROGRESSBAR);
	elm_object_content_set(ad->popup, layout);
	elm_object_part_text_set(layout, "elm.text", string);
	const char *temp = elm_object_part_text_get(layout, "elm.text");
	dlog_print(DLOG_INFO, LOG_TAG, temp);
	evas_object_show(layout);


	progressbar = elm_progressbar_add(layout);
	elm_object_style_set(progressbar, "process/popup/small");
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	evas_object_show(progressbar);

	evas_object_show(ad->popup);
	// 시간 초과 시 프로세스 로딩 팝업 창 닫는 명령줄.
	/*
	elm_popup_timeout_set(ad->popup, 10.0);
	evas_object_smart_callback_add(ad->popup, "timeout", _message_popup_timeout, NULL);
	*/
}
