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

#ifndef GE2D_PORT_H_
#define GE2D_PORT_H_

#define ge2d_fail (-1)
#define ge2d_success (0)

#define OSD0 (0)
#define OSD1 (1)

#define SRC1_GB_ALPHA_ENABLE (0x80000000)

#ifdef __DEBUG
#define D_GE2D(fmt, args...) printf(fmt, ## args)
#else
#define D_GE2D(fmt, args...)
#endif
#define E_GE2D(fmt, args...) printf(fmt, ## args)

#if defined (__cplusplus)
extern "C" {
#endif

#define GE2D_MAX_PLANE (4)

enum GE2DCanvas {
    GE2D_CANVAS_OSD0 = 0,
    GE2D_CANVAS_OSD1,
    GE2D_CANVAS_ALLOC,
    GE2D_CANVAS_TYPE_INVALID,
};

enum GE2DMemtype {
    GE2D_MEM_ION,
    GE2D_MEM_DMABUF,
    GE2D_MEM_INVALID,
};

enum GE2DLayerMode {
    GE2D_LAYER_MODE_INVALID = 0,
    GE2D_LAYER_MODE_NON = 1,
    GE2D_LAYER_MODE_PREMULTIPLIED = 2,
    GE2D_LAYER_MODE_COVERAGE = 3,
};

/* Blend modes, settable per layer */
enum GE2DBlendMode {
    GE2D_BLEND_MODE_INVALID = 0,

    /* colorOut = colorSrc */
    GE2D_BLEND_MODE_NONE = 1,

    /* colorOut = colorSrc + colorDst * (1 - alphaSrc) */
    GE2D_BLEND_MODE_PREMULTIPLIED = 2,

    /* colorOut = colorSrc * alphaSrc + colorDst * (1 - alphaSrc) */
    GE2D_BLEND_MODE_COVERAGE = 3,
};

/**
 * pixel format definitions, export from android graphics.h
 */
enum GE2DPixelFormat {
    GE2D_PIXEL_FORMAT_INVALID            = 0,
    GE2D_PIXEL_FORMAT_RGBA_8888          = 1,
    GE2D_PIXEL_FORMAT_RGBX_8888          = 2,
    GE2D_PIXEL_FORMAT_RGB_888            = 3,
    GE2D_PIXEL_FORMAT_RGB_565            = 4,
    GE2D_PIXEL_FORMAT_BGRA_8888          = 5,
    GE2D_PIXEL_FORMAT_YV12               = 0x32315659,   // YCrCb 4:2:0 Planar  YYYYUV,actually is is YU12
    GE2D_PIXEL_FORMAT_Y8                 = 0x20203859,   // YYYY
    GE2D_PIXEL_FORMAT_YCbCr_422_SP       = 0x10,         // NV16   YYYY.....UVUV
    GE2D_PIXEL_FORMAT_YCrCb_420_SP       = 0x11,         // NV21   YCrCb YYYY.....VU
    GE2D_PIXEL_FORMAT_YCbCr_422_UYVY     = 0x14,         // UYVY   U0-Y0-V0-Y1 U2-Y2-V2-Y3 U4
    GE2D_PIXEL_FORMAT_BGR_888,
    GE2D_PIXEL_FORMAT_YCbCr_420_SP_NV12,                 // NV12 YCbCr YYYY.....UV
};

/* if customized matrix is used, set this flag in format */
#define MATRIX_CUSTOM (0x80000000)

enum GE2DRotation {
    GE2D_ROTATION_0,
    GE2D_ROTATION_90,
    GE2D_ROTATION_180,
    GE2D_ROTATION_270,
};

enum GE2DOP {
    GE2D_OP_FILLRECTANGLE,
    GE2D_OP_BLEND,
    GE2D_OP_STRETCHBLIT,
    GE2D_OP_BLIT,
    GE2D_OP_NONE,
};

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rectangle_t;

typedef struct buffer_info {
    unsigned int mem_alloc_type;
    unsigned int memtype;
    char* vaddr[GE2D_MAX_PLANE];
    unsigned long offset[GE2D_MAX_PLANE];
    unsigned int canvas_w;
    unsigned int canvas_h;
    rectangle_t rect;
    int format;
    unsigned int rotation;
    int shared_fd[GE2D_MAX_PLANE];
    unsigned char plane_alpha;
    unsigned char layer_mode;
    unsigned char fill_color_en;
    unsigned int  def_color;
    int plane_number;
} buffer_info_t;

struct ge2d_matrix_s {
    unsigned int pre_offset0;
    unsigned int pre_offset1;
    unsigned int pre_offset2;
    unsigned int coef0;
    unsigned int coef1;
    unsigned int coef2;
    unsigned int coef3;
    unsigned int coef4;
    unsigned int coef5;
    unsigned int coef6;
    unsigned int coef7;
    unsigned int coef8;
    unsigned int offset0;
    unsigned int offset1;
    unsigned int offset2;
    /* input y/cb/cr saturation enable */
    unsigned char sat_in_en;
};

typedef struct aml_ge2d_info {
    int ge2d_fd;  /* ge2d_fd */
    int ion_fd;   /* ion_fd, no use */
    unsigned int offset;
    unsigned int blend_mode;
    enum GE2DOP ge2d_op;
    buffer_info_t src_info[2];
    buffer_info_t dst_info;
    unsigned int color;
    unsigned int gl_alpha;
    unsigned int const_color;
    /* means do multi ge2d op */
    unsigned int dst_op_cnt;
    int cap_attr;
    int b_src_swap;
    struct ge2d_matrix_s matrix_custom;
    unsigned int reserved;
} aml_ge2d_info_t;

int ge2d_open(void);
int ge2d_close(int fd);
int ge2d_get_cap(int fd);
int ge2d_process(int fd, aml_ge2d_info_t *pge2dinfo);
int ge2d_process_ion(int fd, aml_ge2d_info_t *pge2dinfo);

#if defined (__cplusplus)
}
#endif

#endif
