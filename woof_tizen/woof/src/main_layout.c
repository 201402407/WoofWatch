/*
 * main layout
 * Author : Leehaewon
 * Team : Triangle
 * AppName : Woof
 */

#include "main.h"
#include "mainlayout.h"

static void _index_sync(void *data);

static void
_on_index_mouse_down_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_on_index_mouse_up_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and move to the page of the currently selected index item */
	if (!pd->last_it) return;
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);

	if (!pd->new_it) return;

	int idx = (int) elm_object_item_data_get(pd->new_it);
	if (idx == pd->cur_page) return;

	elm_scroller_page_bring_in(pd->scroller, idx, 0);
}

static void
_on_index_mouse_move_cb(void *data, Evas *e, Evas_Object *o, void *event_info)
{
	page_data *pd = (page_data *)data;

	/* Keep the last index item active and save the currently selected index item */
	if (!pd->last_it) return;

	int level = elm_index_item_level_get(o);
	pd->new_it = elm_index_selected_item_get(o, level);
	elm_index_item_selected_set(pd->last_it, EINA_TRUE);
}

static void
_index_refresh(void *data)
{
	int i, num_index;
	page_data *pd = (page_data *)data;

        num_index = pd->max_page + 1;

	elm_index_item_clear(pd->index);
	if (pd->cur_page < pd->min_page)
	{
		for(i = pd->cur_page ; i < pd->cur_page + num_index ; i++)
		{
			printf("Added : %d\n", i);
			elm_index_item_append(pd->index, NULL, NULL, (void *) i);
		}
		pd->min_page = pd->cur_page;
		pd->min_page = pd->cur_page + num_index - 1;
	}
	else
	{
		for(i = pd->cur_page - num_index + 1; i < pd->cur_page + 1 ; i++)
		{
			printf("Added : %d\n", i);
			elm_index_item_append(pd->index, NULL, NULL, (void *) i);
		}
		pd->min_page = pd->cur_page - num_index;
		pd->min_page = pd->cur_page - 1;
	}
	elm_index_level_go(pd->index, 0);
	_index_sync(pd);
}

static void
_index_sync(void *data)
{
	page_data *pd = (page_data *)data;
	Elm_Object_Item *it;
	it = elm_index_item_find(pd->index, (void *)pd->cur_page);
	if (it) {
		elm_index_item_selected_set(it, EINA_TRUE);
		pd->last_it = it;
		pd->new_it = it;
	}
	else
		_index_refresh(pd);
}

static void
_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	page_data *pd = (page_data *)data;
	free(pd);
}

static void
_layout_resize_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	int w1, h1, w2, h2;
	page_data *pd = (page_data *)data;

	evas_object_geometry_get(obj, NULL, NULL, &w1, &h1);
	evas_object_geometry_get(pd->main_layout, NULL, NULL, &w2, &h2);

	evas_object_size_hint_min_set(pd->left_right_rect, (w2 - w1) / 2, h2 / 2);
	elm_scroller_page_size_set(pd->scroller, w1, h1);
}

static Eina_Bool
animator_cb(void *data)
{
	int i, num_items;
	page_data *pd = (page_data *)data;

        num_items = pd->max_page + 1;

	//Since mapbuf is not perfect, Enable them after the size calculation is finished
	for (i = 0; i < num_items; ++i)
	{
		elm_mapbuf_enabled_set(pd->mapbuf[i], EINA_TRUE);
	}
	return ECORE_CALLBACK_CANCEL;
}

static void
_scroll(void *data, Evas_Object *obj, void *ei)
{
	int cur_page;
	page_data *pd = (page_data *)data;
	elm_scroller_current_page_get(pd->scroller, &cur_page, NULL);
	if (cur_page != pd->cur_page) {
		printf("scroll: %d\n", pd->cur_page);
		pd->prev_page = pd->cur_page;
		pd->cur_page = cur_page;
		if ((pd->cur_page >= NUM_ITEMS_CIRCLE_EVEN) || (pd->prev_page >= NUM_ITEMS_CIRCLE_EVEN))
			return;
		elm_object_signal_emit(pd->page_layout[pd->cur_page], "elm,state,thumbnail,select", "elm");
		elm_object_signal_emit(pd->page_layout[pd->prev_page], "elm,state,thumbnail,unselect", "elm");
		_index_sync(pd);
	}
}

static void
_main_image1_cb(void *data , Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, " # main layout image1 callback");
	appdata_s *ad = data;
	//_popup_small_process_loading(ad, NULL, "sound recognition loading ..", NULL);
	//Testing(ad);

}

static void
_main_image2_cb(void *data , Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, " # main layout image2 callback");
	appdata_s *ad = data;
	_popup_small_process_loading(ad, NULL, "watch bluetooth checking ..", NULL);
	//ad->timer1 = ecore_timer_add(1.0, bluetooth_main, ad); // 비동기적 타이머.

}
static void
_main_image3_cb(void *data , Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, " # main layout image3 callback");
}
static void
_main_image4_cb(void *data , Evas_Object *obj, void *event_info) {
	dlog_print(DLOG_INFO, LOG_TAG, " # main layout image4 callback");
}

// Main Layout 만드는 핵심 함수.
static void
_create_view_layout(void *data, char *layout_file, char *index_style, char it_style[][20], Eina_Bool even_num)
{
	dlog_print(DLOG_INFO, LOG_TAG, " # create main layout view .. ");
	Evas_Object *layout, *scroller, *box, *left_right_rect, *page_layout, *index;
	Evas_Object *img[4];
	Elm_Object_Item *nf_it;
	char img_path[PATH_MAX];
	int i, max_items;

	appdata_s *ad = data;

	// page_data 구조체 생성(calloc으로 메모리 동적 할당 = calloc은 내부의 값 초기화 / malloc은 내부 값 초기화 X)
	page_data *pd = calloc(1, sizeof(page_data));

	/* Create Layout */
	/* 우선 기본 레이아웃을 생성하고 EDJ 파일을 입힌다. */
	layout = elm_layout_add(ad->naviframe);
	elm_layout_file_set(layout, EDJ_ABSOLUTE_FILE, layout_file);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _layout_del_cb, pd);
	pd->main_layout = layout;

	/* Create Scroller */
	/* 좌,우 움직이는 화면 */
	scroller = elm_scroller_add(layout);
	elm_scroller_loop_set(scroller, EINA_FALSE, EINA_FALSE);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_scroller_page_scroll_limit_set(scroller, 1, 0);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	 // 레이아웃에 적용된 EDC 파일의 part name을 3번째 인자의 오브젝트에 입히는건가 ?
	elm_object_part_content_set(layout, "scroller", scroller);
	evas_object_show(scroller);
	// 스크롤 시 callback 실행.
	evas_object_smart_callback_add(scroller, "scroll", _scroll, pd);
	pd->scroller = scroller;

	/* Create Box */
	box = elm_box_add(scroller);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);
	evas_object_show(box);
	pd->box = box;

	/* Create Rect object for page_layout center align in Box */
	/* 먼진 모르겠지만 Box 오브젝트를 통해 해당 화면을 가운데 정렬을 해준다는거 같아 */
	left_right_rect = evas_object_rectangle_add(evas_object_evas_get(box));
	elm_box_pack_end(box, left_right_rect);

	/* Create Pages */
	if (even_num)
		max_items = NUM_INDEX_CIRCLE_EVEN;
	else
		max_items = NUM_ITEMS;
	for (i = 0; i < max_items; ++i)
	{
		// 레이아웃 박스안에 생성(가운데 정렬 완료)
		page_layout = elm_layout_add(box);
		pd->page_layout[i] = page_layout;
		evas_object_size_hint_weight_set(page_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(page_layout, 0, EVAS_HINT_FILL);
		elm_layout_theme_set(page_layout, "layout", "body_thumbnail", "default");
		evas_object_show(page_layout);

		// 레이아웃 안에 이미지 넣기.
		img[i] = elm_image_add(page_layout);
		snprintf(img_path, sizeof(img_path), "%s/mainimage_%d.png", MAINIMAGE_DIR, (i % 6) + 1);
		elm_image_file_set(img[i], img_path, NULL);
		elm_object_part_content_set(page_layout, "elm.icon", img[i]);

		elm_box_pack_end(box, page_layout);

	}

	// image click callback setting.
	dlog_print(DLOG_INFO, LOG_TAG, " # create main layout image callback ..");
	evas_object_smart_callback_add(img[0], "clicked", _main_image1_cb, ad);
	evas_object_smart_callback_add(img[1], "clicked", _main_image2_cb, ad);
	evas_object_smart_callback_add(img[2], "clicked", _main_image3_cb, ad);
	evas_object_smart_callback_add(img[3], "clicked", _main_image4_cb, ad);

	/*Add resize callback for get a actual size of layout and main layout */
	evas_object_event_callback_add(pd->page_layout[0], EVAS_CALLBACK_RESIZE, _layout_resize_cb, pd);

	elm_box_pack_end(box, left_right_rect);
	pd->left_right_rect = left_right_rect;

	/* Add animator for page transfer effect */
	ecore_animator_add(animator_cb, pd);

	/* Create Index */
	index = elm_index_add(layout);
	elm_object_style_set(index, index_style);
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	elm_object_part_content_set(layout, "controller", index);

	for (i = 0; i < max_items; ++i)
	{
		pd->it[i] = elm_index_item_append(index, NULL, NULL, (void *) i);
		if (it_style)
			elm_object_item_style_set(pd->it[i], it_style[i]);
	}

	pd->min_page = 0;
	pd->max_page = max_items - 1;
	elm_index_level_go(index, 0);
	elm_index_item_selected_set(pd->it[0], EINA_TRUE);

	pd->cur_page = 0;
	pd->index = index;
	pd->last_it = pd->it[0];

	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_DOWN, _on_index_mouse_down_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_MOVE, _on_index_mouse_move_cb, pd);
	evas_object_event_callback_add(index, EVAS_CALLBACK_MOUSE_UP, _on_index_mouse_up_cb, pd);


	nf_it = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, layout, NULL);
	elm_naviframe_item_title_enabled_set(nf_it, EINA_FALSE, EINA_FALSE);
}
/*
 * main layout 생성 시작 함수
 */
void
_create_main_layout_start(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG, " # create main layout start ");
	appdata_s *ad = (appdata_s *)data;


	// FALSE면 홀수 개 의 아이콘, TRUE면 짝수 개의 아이콘.
	_create_view_layout(ad, MAIN_LAYOUT, "thumbnail", NULL, EINA_TRUE);
}
