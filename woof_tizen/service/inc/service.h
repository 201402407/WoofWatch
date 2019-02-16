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

void recognition_data_send(const char* string);
#endif /* __service_H__ */
