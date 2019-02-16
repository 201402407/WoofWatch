/*
 * mainlayout.h
 *
 *  Created on: Jan 7, 2019
 *      Author: leehaewon
 */

#ifndef MAINLAYOUT_H_
#define MAINLAYOUT_H_


#define NUM_ITEMS             3
#define NUM_INDEX             3
#define NUM_ITEMS_CIRCLE_EVEN 4
#define NUM_INDEX_CIRCLE_EVEN 4

typedef struct _item_data
{
	int index;
	Elm_Object_Item *item;
} item_data;

// 페이지 만드는데 필요한 변수들, 오브젝트들 구조체로 정의.
typedef struct _page_data page_data;
struct _page_data
{
	Evas_Object *main_layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *mapbuf[NUM_ITEMS_CIRCLE_EVEN];
	Evas_Object *index;
	Evas_Object *page_layout[NUM_ITEMS_CIRCLE_EVEN];
	Evas_Object *left_right_rect;
	int cur_page;
	int prev_page;
	Elm_Object_Item *it[NUM_ITEMS_CIRCLE_EVEN];

	Elm_Object_Item *last_it;
	Elm_Object_Item *new_it;
	int min_page, max_page;
};



#endif /* MAINLAYOUT_H_ */
