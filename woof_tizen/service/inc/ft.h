/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <glib.h>
#include "service.h"

#ifndef __FT_H__
#define __FT_H__

#define TAG "FTRceiver"

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.filetransferreceiver"
#endif

#define GRP_MAIN "main"

gboolean find_peers(void);
void     send_file(void);
void     cancel_file(void);
void     reject_file(void);
void     accept_file(void);
gboolean initialize_sap(void);

#endif /* __FT_H__ */
