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

#ifndef VPCODEC_1_0_H_
#define VPCODEC_1_0_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum vl_codec_id_e {
    CODEC_ID_NONE,
    CODEC_ID_VP8,
    CODEC_ID_H261,
    CODEC_ID_H263,
    CODEC_ID_H264, /* must support */
    CODEC_ID_H265,
} vl_codec_id_t;

typedef enum vl_img_format_e {
    IMG_FMT_NONE,
    IMG_FMT_NV12, /* must support  */
    IMG_FMT_NV21,
    IMG_FMT_YV12,
} vl_img_format_t;

typedef enum vl_frame_type_e {
    FRAME_TYPE_NONE,
    FRAME_TYPE_AUTO, /* encoder self-adaptation(default) */
    FRAME_TYPE_IDR,
    FRAME_TYPE_I,
    FRAME_TYPE_P,
} vl_frame_type_t;

typedef struct {
    int framerate;
    int bitrate;
    int gop; /* max I frame interval */
} vl_encoder_param_t;

typedef struct {
    vl_frame_type_t frame_type;
    int format;
} vl_encode_info_t;


/**
 * Getting version information
 *
 * @return : version information
 */
const char *vl_get_version(void);

/**
 * init encoder
 *
 * @param : codec_id: codec type
 * @param : width: video width
 * @param : height: video height
 * @param : vl_encoder_param_t: encoder param
 * @param : img_format: image format
 * @return : if success return encoder handle,else return <= 0
 */
long vl_video_encoder_init(vl_codec_id_t codec_id, int width, int height, vl_encoder_param_t param, \
    vl_img_format_t img_format);

/**
 * encode video
 *
 * @param : handle
 * @param : vl_encode_info_t : encode info
 * @param : in: data to be encoded
 * @param : out: data output,H.264 need header(0x00, 0x00, 0x00, 0x01),
 *          and format must be I420(apk set param out，through jni,
 *          so modify "out" in the function,don't change address point)
 * @param : idr: idr frame flag
 * @return : if success return encoded data length,else return <= 0
 */
int vl_video_encoder_encode(long handle, vl_encode_info_t info, unsigned char *in, \
    unsigned char *out, int *idr);

/**
 * destroy encoder
 *
 * @param ：handle: encoder handle
 * @return ：if success return 1,else return 0
 */
int vl_video_encoder_destory(long handle);

#ifdef __cplusplus
}
#endif

#endif /* VPCODEC_1_0_H_ */
