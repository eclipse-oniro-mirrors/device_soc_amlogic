/*
 * Copyright (c) 2022 Unionman Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AMPLAYER_AUDIO_RENDER_ADAPTER
#define AMPLAYER_AUDIO_RENDER_ADAPTER

#include "audio_external_render.h"

#if defined(__cplusplus)
extern "C" {
#endif

audio_render_t *audio_render_get(void);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // AMPLAYER_AUDIO_RENDER_ADAPTER
