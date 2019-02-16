#ifndef __service_H__
#define __service_H__

#include <dlog.h>
#include <tizen.h>
#include <Ecore.h>
#include <service_app.h>
#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <glib.h>
#include "ft.h"
#include "ft_progressbar.h"

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif

#define LOG_TAG "WoofService"
#define SRC_PATH "/opt/usr/media/Downloads/woof"
#define DOWNLOAD_PATH "/opt/usr/media/Downloads"

void send_activity_data(const char* key, const char* string);
char* get_file_all_name(const char* path);
char* get_file_only_name(const char* path);
char* get_file_only_extension(const char* path);
#endif /* __service_H__ */
