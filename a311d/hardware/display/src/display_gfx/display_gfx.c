/*
 * Copyright (c) 2022 Unionman Co., Ltd.
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

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "display_common.h"
#include "display_gralloc.h"
#include "securec.h"
#include "aml_ge2d.h"
#include "ge2d_dmabuf.h"
#include "display_gfx_private.h"
#include "display_gfx.h"

static aml_ge2d_t g_ge2d;

static enum GE2DBlendMode blendTypeChange(BlendType blendType)
{
    enum GE2DBlendMode ge2dBlendMode;

    switch (blendType) {
        case BLEND_SRC:
            ge2dBlendMode = GE2D_BLEND_MODE_PREMULTIPLIED;
            break;
        case BLEND_SRCOVER:
            ge2dBlendMode = GE2D_BLEND_MODE_NONE;
            break;
        case BLEND_SRCATOP:
            ge2dBlendMode = GE2D_BLEND_MODE_COVERAGE;
            break;
        default:
            ge2dBlendMode = GE2D_BLEND_MODE_INVALID;
            break;
    }

    return ge2dBlendMode;
}

static enum GE2DPixelFormat pixelFormatChange(PixelFormat fmt)
{
    enum GE2DPixelFormat ge2dPixelFmt;

    switch (fmt) {
        case PIXEL_FMT_RGBA_8888:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_RGBA_8888;
            break;
        case PIXEL_FMT_RGBX_8888:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_RGBX_8888;
            break;
        case PIXEL_FMT_RGB_888:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_RGB_888;
            break;
        case PIXEL_FMT_BGRA_8888:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_BGRA_8888;
            break;
        case PIXEL_FMT_YCRCB_420_P:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_YV12;
            break;
        case PIXEL_FMT_YCBCR_422_SP:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_YCbCr_422_SP;
            break;
        case PIXEL_FMT_YCRCB_420_SP:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_YCrCb_420_SP;
            break;
        case PIXEL_FMT_UYVY_422_PKG:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_YCbCr_422_UYVY;
            break;
        default:
            ge2dPixelFmt = GE2D_PIXEL_FORMAT_RGBA_8888;
            break;
    }

    return ge2dPixelFmt;
}

static enum GE2DRotation transformTypeChange(TransformType type)
{
    enum GE2DRotation ge2dRotation;

    switch (type) {
        case ROTATE_NONE:
            ge2dRotation = GE2D_ROTATION_0;
            break;
        case ROTATE_90:
            ge2dRotation = GE2D_ROTATION_90;
            break;
        case ROTATE_180:
            ge2dRotation = GE2D_ROTATION_180;
            break;
        case ROTATE_270:
            ge2dRotation = GE2D_ROTATION_270;
            break;
        default:
            ge2dRotation = GE2D_ROTATION_0;
            break;
    }

    return ge2dRotation;
}

static int32_t InitGfx()
{
    int32_t ret = DISPLAY_SUCCESS;

    DISPLAY_LOGI("Init Gfx\n");

    ret = aml_ge2d_init(&g_ge2d);
    if (ret < 0) {
        DISPLAY_LOGE("aml_ge2d_init failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    return DISPLAY_SUCCESS;
}

static int32_t DeinitGfx()
{
    DISPLAY_LOGI("Deinit Gfx\n");
    aml_ge2d_exit(&g_ge2d);
    return DISPLAY_SUCCESS;
}

static void FillGe2d(ISurface *surface, IRect dstRect, uint32_t color, GfxOpt *opt)
{
    g_ge2d.ge2dinfo.offset = 0;
    g_ge2d.ge2dinfo.ge2d_op = GE2D_OP_FILLRECTANGLE;
    g_ge2d.ge2dinfo.blend_mode = GE2D_BLEND_MODE_NONE;

    g_ge2d.ge2dinfo.src_info[0].memtype = GE2D_CANVAS_TYPE_INVALID;
    g_ge2d.ge2dinfo.src_info[1].memtype = GE2D_CANVAS_TYPE_INVALID;

    g_ge2d.ge2dinfo.dst_info.plane_number = 1;
    g_ge2d.ge2dinfo.dst_info.rotation = GE2D_ROTATION_0;
    g_ge2d.ge2dinfo.dst_info.mem_alloc_type = GE2D_MEM_DMABUF;
    g_ge2d.ge2dinfo.dst_info.memtype = GE2D_CANVAS_ALLOC;

    g_ge2d.ge2dinfo.dst_info.shared_fd[0] = (int32_t)surface->phyAddr;
    g_ge2d.ge2dinfo.color = color;
    g_ge2d.ge2dinfo.dst_info.plane_alpha = surface->alpha0;
    g_ge2d.ge2dinfo.dst_info.format = pixelFormatChange(surface->enColorFmt);
    g_ge2d.ge2dinfo.dst_info.canvas_w = surface->width;
    g_ge2d.ge2dinfo.dst_info.canvas_h = surface->height;
    g_ge2d.ge2dinfo.dst_info.rect.x = dstRect.x;
    g_ge2d.ge2dinfo.dst_info.rect.y = dstRect.y;
    g_ge2d.ge2dinfo.dst_info.rect.w = dstRect.w;
    g_ge2d.ge2dinfo.dst_info.rect.h = dstRect.h;

    if ((opt != NULL) && (opt->enGlobalAlpha)) {
        g_ge2d.ge2dinfo.gl_alpha = opt->globalAlpha;
        g_ge2d.ge2dinfo.src_info[0].plane_alpha = surface->alpha0;
    }

    return ;
}

static int32_t FillRect(ISurface *surface, IRect *rect, uint32_t color, GfxOpt *opt)
{
    int32_t ret = DISPLAY_SUCCESS;
    errno_t eok = EOK;
    IRect dstRect;

    if (surface == NULL) {
        DISPLAY_LOGE("surface is null and return\n");
        return DISPLAY_NULL_PTR;
    }

    eok = memset_s(&dstRect, sizeof(dstRect), 0, sizeof(dstRect));
    if (ret != EOK) {
        DISPLAY_LOGE("memset_s failed");
        return DISPLAY_FAILURE;
    }

    if (rect != NULL) {
        dstRect.x = rect->x;
        dstRect.y = rect->y;
        dstRect.w = rect->w;
        dstRect.h = rect->h;
    } else {
        dstRect.x = 0;
        dstRect.y = 0;
        dstRect.w = surface->width;
        dstRect.h = surface->height;
    }

    DISPLAY_LOGD("fd %{public}d, rect [%{public}d %{public}d %{public}d %{public}d]", \
        (int32_t)surface->phyAddr, dstRect.x, dstRect.y, dstRect.w, dstRect.h);

    eok = memset_s(&g_ge2d.ge2dinfo.src_info, sizeof(g_ge2d.ge2dinfo.src_info), \
        0, sizeof(g_ge2d.ge2dinfo.src_info));
    if (eok != EOK) {
        DISPLAY_LOGE("memset_s failed");
        return DISPLAY_FAILURE;
    }

    eok = memset_s(&g_ge2d.ge2dinfo.dst_info, sizeof(g_ge2d.ge2dinfo.dst_info), \
        0, sizeof(g_ge2d.ge2dinfo.dst_info));
    if (eok != EOK) {
        DISPLAY_LOGE("memset_s failed");
        return DISPLAY_FAILURE;
    }

    FillGe2d(surface, dstRect, color, opt);

    ret = aml_ge2d_process(&g_ge2d.ge2dinfo);
    if (ret < 0) {
        DISPLAY_LOGE("aml_ge2d_process failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    return DISPLAY_SUCCESS;
}

static void BlitGe2dOpt(enum GE2DOP opMode, GfxOpt *opt)
{
    enum GE2DBlendMode blendMode = GE2D_BLEND_MODE_NONE;
    enum GE2DRotation rotation = GE2D_ROTATION_0;
    int32_t globalAlpha = 0;
    errno_t eok = EOK;

    eok = memset_s(&g_ge2d.ge2dinfo.src_info, sizeof(g_ge2d.ge2dinfo.src_info), \
        0, sizeof(g_ge2d.ge2dinfo.src_info));
    if (eok != EOK) {
        DISPLAY_LOGW("memset_s failed");
    }

    eok = memset_s(&g_ge2d.ge2dinfo.dst_info, sizeof(g_ge2d.ge2dinfo.dst_info), \
        0, sizeof(g_ge2d.ge2dinfo.dst_info));
    if (eok != EOK) {
        DISPLAY_LOGW("memset_s failed");
    }

    if (opt != NULL) {
        globalAlpha = opt->globalAlpha;
        rotation = transformTypeChange(opt->rotateType);

        if (BLEND_NONE != opt->blendType) {
            blendMode = blendTypeChange(opt->blendType);
            if (GE2D_BLEND_MODE_INVALID == blendMode) {
                DISPLAY_LOGW("No support blend type %{public}d", opt->blendType);
            }
        }
    }

    DISPLAY_LOGD("opMode: %{public}d", opMode);
    if (blendMode != GE2D_BLEND_MODE_INVALID) {
        opMode = GE2D_OP_BLEND;
    }

    g_ge2d.ge2dinfo.ge2d_op = opMode;
    if (GE2D_OP_BLEND != opMode) {
        blendMode = GE2D_BLEND_MODE_NONE;
    }

    g_ge2d.ge2dinfo.ge2d_op = opMode;
    g_ge2d.ge2dinfo.blend_mode = blendMode;
    g_ge2d.ge2dinfo.dst_info.rotation = rotation;
    g_ge2d.ge2dinfo.gl_alpha = globalAlpha;

    return ;
}

static int32_t BlitGe2dNoAlpha(ISurface *srcSurface, IRect sRect, ISurface *dstSurface, IRect dRect)
{
    int32_t ret = DISPLAY_SUCCESS;

    g_ge2d.ge2dinfo.src_info[0].plane_number = 1;
    g_ge2d.ge2dinfo.src_info[0].plane_alpha = srcSurface->alpha0;
    g_ge2d.ge2dinfo.src_info[0].memtype = GE2D_CANVAS_ALLOC;
    g_ge2d.ge2dinfo.src_info[0].mem_alloc_type = GE2D_MEM_DMABUF;
    g_ge2d.ge2dinfo.src_info[0].layer_mode = GE2D_LAYER_MODE_NON;
    g_ge2d.ge2dinfo.src_info[0].shared_fd[0] = (int32_t)srcSurface->phyAddr;
    g_ge2d.ge2dinfo.src_info[0].canvas_w = srcSurface->width;
    g_ge2d.ge2dinfo.src_info[0].canvas_h = srcSurface->height;
    g_ge2d.ge2dinfo.src_info[0].format = pixelFormatChange(srcSurface->enColorFmt);
    g_ge2d.ge2dinfo.src_info[0].rect.x = sRect.x;
    g_ge2d.ge2dinfo.src_info[0].rect.y = sRect.y;
    g_ge2d.ge2dinfo.src_info[0].rect.w = sRect.w;
    g_ge2d.ge2dinfo.src_info[0].rect.h = sRect.h;

    g_ge2d.ge2dinfo.src_info[1].plane_number = 1;
    g_ge2d.ge2dinfo.src_info[1].plane_alpha = dstSurface->alpha0;
    g_ge2d.ge2dinfo.src_info[1].memtype = GE2D_CANVAS_ALLOC;
    g_ge2d.ge2dinfo.src_info[1].mem_alloc_type = GE2D_MEM_DMABUF;
    g_ge2d.ge2dinfo.src_info[1].layer_mode = GE2D_LAYER_MODE_NON;
    g_ge2d.ge2dinfo.src_info[1].shared_fd[0] = (int32_t)dstSurface->phyAddr;
    g_ge2d.ge2dinfo.src_info[1].canvas_w = dstSurface->width;
    g_ge2d.ge2dinfo.src_info[1].canvas_h = dstSurface->height;
    g_ge2d.ge2dinfo.src_info[1].format = pixelFormatChange(dstSurface->enColorFmt);
    g_ge2d.ge2dinfo.src_info[1].rect.x = dRect.x;
    g_ge2d.ge2dinfo.src_info[1].rect.y = dRect.y;
    g_ge2d.ge2dinfo.src_info[1].rect.w = dRect.w;
    g_ge2d.ge2dinfo.src_info[1].rect.h = dRect.h;

    ret = aml_ge2d_process(&g_ge2d.ge2dinfo);
    if (ret < 0) {
        DISPLAY_LOGE("aml_ge2d_process failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    return DISPLAY_SUCCESS;
}

static int32_t BlitGe2dAlpha(ISurface *dstSurface, IRect dRect)
{
    int32_t ret = DISPLAY_SUCCESS;

    g_ge2d.ge2dinfo.src_info[0].layer_mode = GE2D_LAYER_MODE_PREMULTIPLIED;
    g_ge2d.ge2dinfo.dst_info.mem_alloc_type = GE2D_MEM_DMABUF;
    g_ge2d.ge2dinfo.dst_info.memtype = GE2D_CANVAS_ALLOC;
    g_ge2d.ge2dinfo.dst_info.shared_fd[0] = (int32_t)dstSurface->phyAddr;
    g_ge2d.ge2dinfo.dst_info.plane_number = 1;
    g_ge2d.ge2dinfo.dst_info.plane_alpha = dstSurface->alpha0;
    g_ge2d.ge2dinfo.dst_info.canvas_w = dstSurface->width;
    g_ge2d.ge2dinfo.dst_info.canvas_h = dstSurface->height;
    g_ge2d.ge2dinfo.dst_info.format = pixelFormatChange(dstSurface->enColorFmt);
    g_ge2d.ge2dinfo.dst_info.rect.x = dRect.x;
    g_ge2d.ge2dinfo.dst_info.rect.y = dRect.y;
    g_ge2d.ge2dinfo.dst_info.rect.w = dRect.w;
    g_ge2d.ge2dinfo.dst_info.rect.h = dRect.h;

    ret = aml_ge2d_process(&g_ge2d.ge2dinfo);
    if (ret < 0) {
        DISPLAY_LOGE("aml_ge2d_process failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    return DISPLAY_SUCCESS;
}

static int32_t Blit(ISurface *srcSurface, IRect *srcRect, ISurface *dstSurface, IRect *dstRect, GfxOpt *opt)
{
    int32_t ret = DISPLAY_SUCCESS;
    IRect sRect, dRect;
    enum GE2DOP opMode = GE2D_OP_NONE;

    if ((srcSurface == NULL) || (dstSurface == NULL)) {
        DISPLAY_LOGE("srcSurface or dstSurface is null and return\n");
        return DISPLAY_NULL_PTR;
    }

    if (srcRect != NULL) {
        sRect.x = srcRect->x;
        sRect.y = srcRect->y;
        sRect.w = srcRect->w;
        sRect.h = srcRect->h;
    } else {
        sRect.x = 0;
        sRect.y = 0;
        sRect.w = srcSurface->width;
        sRect.h = srcSurface->height;
    }

    if (dstRect != NULL) {
        dRect.x = dstRect->x;
        dRect.y = dstRect->y;
        dRect.w = dstRect->w;
        dRect.h = dstRect->h;
    } else {
        dRect.x = 0;
        dRect.y = 0;
        dRect.w = dstSurface->width;
        dRect.h = dstSurface->height;
    }

    DISPLAY_LOGD("src: w %{public}d, h %{publuc}d, dst:w %{public}d, h %{publuc}d", \
        srcSurface->width, srcSurface->height, dstSurface->width, dstSurface->height);

    if ((sRect.w != dRect.w) || (sRect.h != dRect.h)) {
        opMode = GE2D_OP_STRETCHBLIT;
    } else {
        opMode = GE2D_OP_BLIT;
    }

    BlitGe2dOpt(opMode, opt);

    ret = BlitGe2dNoAlpha(srcSurface, sRect, dstSurface, dRect);
    if (ret < 0) {
        DISPLAY_LOGE("BlitGe2dNoAlpha failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    ret = BlitGe2dAlpha(dstSurface, dRect);
    if (ret < 0) {
        DISPLAY_LOGE("BlitGe2dAlpha failed. ret=%{public}d", ret);
        return DISPLAY_FAILURE;
    }

    return DISPLAY_SUCCESS;
}

static int32_t Sync(int32_t timeOut)
{
    return DISPLAY_SUCCESS;
}

int32_t GfxInitialize(GfxFuncs **funcs)
{
    DISPLAY_CHK_RETURN((funcs == NULL), DISPLAY_PARAM_ERR, DISPLAY_LOGE("info is null"));
    GfxFuncs *gfxFuncs = (GfxFuncs *)malloc(sizeof(GfxFuncs));
    if (!gfxFuncs) {
        DISPLAY_LOGE("malloc GfxFuncs failed");
        return DISPLAY_FAILURE;
    }

    errno_t eok = memset_s((void *)gfxFuncs, sizeof(GfxFuncs), 0, sizeof(GfxFuncs));
    if (eok != EOK) {
        DISPLAY_LOGE("memset_s failed");
        free(gfxFuncs);
        return DISPLAY_FAILURE;
    }

    gfxFuncs->InitGfx = InitGfx;
    gfxFuncs->DeinitGfx = DeinitGfx;
    gfxFuncs->FillRect = FillRect;
    gfxFuncs->Blit = Blit;
    gfxFuncs->Sync = Sync;
    *funcs = gfxFuncs;

    return DISPLAY_SUCCESS;
}

int32_t GfxUninitialize(GfxFuncs *funcs)
{
    if (funcs == NULL) {
        DISPLAY_LOGE("funcs is null and return\n");
        return DISPLAY_NULL_PTR;
    }
    free(funcs);
    DISPLAY_LOGI("%s: gfx uninitialize success", __func__);
    return DISPLAY_SUCCESS;
}
