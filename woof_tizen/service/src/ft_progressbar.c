/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include "ft_progressbar.h"
#include "ft.h"
#include <unistd.h>

static Evas_Object *cancel_btn;
static Evas_Object *progressbar = NULL;
static Evas_Object *naviframeparent;

static void _timeout(void *data, Evas_Object *obj, void *event_info)
{
	dlog_print(DLOG_INFO, TAG, "# timeout ");
	evas_object_hide(obj);
}

static void _pop_clicked_cb(Evas_Object *parent, char *string)
{
	Evas_Object *layout;
	Evas_Object *popup;

	popup = elm_popup_add(parent);
	elm_popup_timeout_set(popup, 3.0);
	evas_object_smart_callback_add(popup, "timeout", _timeout, NULL);
	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle");

	elm_object_part_text_set(layout, "elm.text", string);
	elm_object_content_set(popup, layout);

	evas_object_show(popup);
}

void set_progress_bar_value(float percentage)
{
	if (progressbar == NULL)
		return;

	elm_progressbar_value_set(progressbar, percentage);
}

static Evas_Object *create_progressbar(Evas_Object *obj)
{
	Evas_Object *progressbar;

	progressbar = elm_progressbar_add(obj);
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.5);

	return progressbar;
}

static Evas_Object *create_scroller(Evas_Object *parent)
{
	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	return scroller;
}

static Evas_Object *create_box(Evas_Object *parent, Eina_Bool hor, double align_x, double align_y, double weight_x, double weight_y)
{
	Evas_Object *box;
	box = elm_box_add(parent);
	elm_box_horizontal_set(box, hor);
	evas_object_size_hint_weight_set(box, weight_x, weight_y);
	evas_object_size_hint_align_set(box, align_x, align_y);
	evas_object_show(box);
	return box;
}

static void _clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	cancel_file();
}

Evas_Object *create_content(Evas_Object *parent)
{
	Evas_Object *main_box, *progress_box;
	Evas_Object *scroller;

	naviframeparent = parent;

	scroller = create_scroller(parent);

	main_box = create_box(scroller, EINA_TRUE, EVAS_HINT_FILL, EVAS_HINT_FILL, EVAS_HINT_EXPAND, 0.0);
	elm_box_homogeneous_set(main_box, EINA_FALSE);
	elm_object_content_set(scroller, main_box);
	elm_box_padding_set(main_box, 15 * elm_config_scale_get(), 15 * elm_config_scale_get());

	elm_box_pack_end(main_box, create_box(main_box, EINA_FALSE, 0.0, 0.0, 0.0, 0.0));

	progress_box = create_box(main_box, EINA_FALSE, EVAS_HINT_FILL, 0.0, EVAS_HINT_EXPAND, 0.0);
	elm_box_homogeneous_set(progress_box, EINA_FALSE);
	elm_box_padding_set(progress_box, 20 * elm_config_scale_get(), 20 * elm_config_scale_get());
	elm_box_pack_end(main_box, progress_box);

	elm_box_pack_end(main_box, create_box(main_box, EINA_FALSE, 0.0, 0.0, 0.0, 0.0));

	progressbar = create_progressbar(progress_box);
	elm_box_pack_end(progress_box, progressbar);

	cancel_btn = elm_button_add(main_box);
	elm_object_text_set(cancel_btn, _("<font_size=26>cancel"));

	evas_object_smart_callback_add(cancel_btn, "clicked", _clicked_cb, (void *)parent);
	evas_object_resize(cancel_btn, 220, 30);
	evas_object_move(cancel_btn, 70, 200);
	evas_object_size_hint_weight_set(cancel_btn, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return scroller;
}

void hide_progress_bar()
{
	evas_object_hide(progressbar);
	evas_object_hide(cancel_btn);
}

void show_progress_bar()
{
	evas_object_show(progressbar);
	evas_object_show(cancel_btn);
	elm_progressbar_value_set(progressbar, 0.0);
}

void progressbar_cb(void *data, Evas_Object *obj, void *event_info)
{
	//TODO:
}
