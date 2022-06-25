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

#ifndef AML_GE2D_H_
#define AML_GE2D_H_

#include "ge2d_port.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct aml_ge2d {
    aml_ge2d_info_t ge2dinfo;
    char *src_data[GE2D_MAX_PLANE];
    char *src2_data[GE2D_MAX_PLANE];
    char *dst_data[GE2D_MAX_PLANE];
    unsigned int src_size[GE2D_MAX_PLANE];
    unsigned int src2_size[GE2D_MAX_PLANE];
    unsigned int dst_size[GE2D_MAX_PLANE];
    void *cmemParm_src[GE2D_MAX_PLANE];
    void *cmemParm_src2[GE2D_MAX_PLANE];
    void *cmemParm_dst[GE2D_MAX_PLANE];
} aml_ge2d_t;

int aml_ge2d_init(aml_ge2d_t *pge2d);
void aml_ge2d_exit(aml_ge2d_t *pge2d);

int aml_ge2d_mem_alloc_ion(aml_ge2d_t *pge2d);
void aml_ge2d_mem_free_ion(aml_ge2d_t *pge2d);

/* support dma_buf */
int aml_ge2d_mem_alloc(aml_ge2d_t *pge2d);
void aml_ge2d_mem_free(aml_ge2d_t *pge2d);
int aml_ge2d_process(aml_ge2d_info_t *pge2dinfo);
int aml_ge2d_process_ion(aml_ge2d_info_t *pge2dinfo);
int aml_ge2d_invalid_cache(aml_ge2d_info_t *pge2dinfo);

#if defined (__cplusplus)
}
#endif

#endif
