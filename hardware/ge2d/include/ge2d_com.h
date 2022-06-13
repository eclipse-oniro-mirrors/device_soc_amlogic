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

#ifndef GE2D_COM_H_
#define GE2D_COM_H_

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef struct {
    unsigned int color;
    rectangle_t src1_rect;
    rectangle_t src2_rect;
    rectangle_t dst_rect;
    int op;
} ge2d_op_para_t;

typedef struct {
    unsigned int color_blending_mode;
    unsigned int color_blending_src_factor;
    unsigned int color_blending_dst_factor;
    unsigned int alpha_blending_mode;
    unsigned int alpha_blending_src_factor;
    unsigned int alpha_blending_dst_factor;
} ge2d_blend_op;

#endif
