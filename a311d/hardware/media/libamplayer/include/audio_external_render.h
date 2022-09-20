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

#ifndef AUDIO_EXTERNAL_RENDER_H
#define AUDIO_EXTERNAL_RENDER_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct audio_render_s {
    void *priv_data;

    int (*init)(struct audio_render_s *r);
    int (*finalize)(struct audio_render_s *r);
    int (*config)(struct audio_render_s *r, uint32_t channels, uint32_t bits_per_sample, uint32_t sample_rate);
    int (*write)(struct audio_render_s *r, uint8_t *data, uint32_t size);
    int (*start)(struct audio_render_s *r);
    int (*stop)(struct audio_render_s *r);
    int (*pause)(struct audio_render_s *r);
    int (*resume)(struct audio_render_s *r);
    uint32_t (*get_latency)(struct audio_render_s *r);      /* get latency in unit of ms */
    int (*set_mute)(struct audio_render_s *r, int mute);    /* 1: enable mute ; 0: disable mute */
    int (*set_volume)(struct audio_render_s *r, float volume);
} audio_render_t;


audio_render_t *audio_render_get(void);


#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif
