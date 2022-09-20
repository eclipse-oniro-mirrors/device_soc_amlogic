/*
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

/**************************************************
* example based on amcodec
**************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>
#include <codec.h>
#include <amvideo.h>
#include <IONmem.h>

#define READ_SIZE (64 * 1024)
#define EXTERNAL_PTS    (1)
#define SYNC_OUTSIDE    (2)
#define UNIT_FREQ       96000
#define PTS_FREQ        90000
#define AV_SYNC_THRESH    PTS_FREQ*30
#define MESON_BUFFER_SIZE 4

static codec_para_t v_codec_para;
static codec_para_t *vpcodec;
#ifdef AUDIO_ES
static codec_para_t a_codec_para;
static codec_para_t *apcodec;
#endif
static codec_para_t *pcodec;
static char *filename;
FILE* fp = NULL;
FILE* yuv = NULL;
static int axis[8] = {0};
struct amvideo_dev *amvideo;

struct out_buffer_t {
    int index;
    int size;
    bool own_by_v4l;
    void *ptr;
    IONMEM_AllocParams buffer;
} vbuffer[MESON_BUFFER_SIZE];

static int amsysfs_set_sysfs_str(const char *path, const char *val)
{
    int fd;
    int bytes;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        bytes = write(fd, val, strlen(val));
        close(fd);
        return 0;
    } else {
        printf("unable to open file %s,err: %s\n", path, strerror(errno));
    }
    return -1;
}

int osd_blank(char *path, int cmd)
{
    int fd;
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);

    if (fd >= 0) {
        sprintf(bcmd, "%d", cmd);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }

    return -1;
}

int set_tsync_enable(int enable)
{
    int fd;
    char *path = "/sys/class/tsync/enable";
    char  bcmd[16];
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        sprintf(bcmd, "%d", enable);
        write(fd, bcmd, strlen(bcmd));
        close(fd);
        return 0;
    }

    return -1;
}

int parse_para(const char *para, int para_num, int *result)
{
    char *endp;
    const char *startp = para;
    int *out = result;
    int len = 0, count = 0;

    if (!startp) {
        return 0;
    }

    len = strlen(startp);

    do {
        //filter space out
        while (startp && (isspace(*startp) || !isgraph(*startp)) && len) {
            startp++;
            len--;
        }

        if (len == 0) {
            break;
        }

        *out++ = strtol(startp, &endp, 0);

        len -= endp - startp;
        startp = endp;
        count++;

    } while ((endp) && (count < para_num) && (len > 0));

    return count;
}

int set_display_axis(int recovery)
{
    int fd;
    char *path = "/sys/class/display/axis";
    char str[128];
    int count;
    fd = open(path, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        if (!recovery) {
            read(fd, str, 128);
            printf("read axis %s, length %d\n", str, strlen(str));
            count = parse_para(str, 8, axis);
        }
        if (recovery) {
            sprintf(str, "%d %d %d %d %d %d %d %d",
                    axis[0], axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        } else {
            sprintf(str, "2048 %d %d %d %d %d %d %d",
                    axis[1], axis[2], axis[3], axis[4], axis[5], axis[6], axis[7]);
        }
        write(fd, str, strlen(str));
        close(fd);
        return 0;
    }

    return -1;
}
static void FreeBuffers()
{
    int i;
    for (i = 0; i < MESON_BUFFER_SIZE; i++) {
        if (vbuffer[i].ptr) {
            CMEM_free(&vbuffer[i].buffer);
        }
    }
}
static int AllocBuffers(int width, int height)
{
    int i, size, ret;
    CMEM_init();
    size = width * height * 3 / 2;
    for (i = 0; i < MESON_BUFFER_SIZE; i++) {
        ret = CMEM_alloc(size, &vbuffer[i].buffer);
        if (ret < 0) {
            printf("CMEM_alloc failed\n");
            FreeBuffers();
            goto fail;
        }
        vbuffer[i].index = i;
        vbuffer[i].size = size;
        vbuffer[i].ptr = vbuffer[i].buffer.vaddr;
    }
fail:
    return ret;
}
static int ionvideo_init(int width, int height)
{
    int i, ret;


    amsysfs_set_sysfs_str("/sys/class/vfm/map", "rm default");
    amsysfs_set_sysfs_str("/sys/class/vfm/map",
               "add default decoder ionvideo");

    ret = AllocBuffers(width, height);
    if (ret < 0) {
        printf("AllocBuffers failed\n");
        ret = -ENODEV;
        goto fail;
    }
    
    amvideo = new_amvideo(FLAGS_V4L_MODE);
    if (!amvideo) {
        printf("amvideo create failed\n");
        ret = -ENODEV;
        goto fail;
    }
    amvideo->display_mode = 0;
    amvideo->use_frame_mode = 0;

    ret = amvideo_init(amvideo, 0, width, height,
            V4L2_PIX_FMT_NV12, MESON_BUFFER_SIZE);
    if (ret < 0) {
        printf("amvideo_init failed\n");
        amvideo_release(amvideo);
        goto fail;
    }
    ret = amvideo_start(amvideo);
    if (ret < 0) {
        amvideo_release(amvideo);
        goto fail;
    }
    for (i = 0; i < MESON_BUFFER_SIZE; i++) {
        vframebuf_t vf;
        vf.fd = vbuffer[i].buffer.mIonHnd;
        vf.length = vbuffer[i].buffer.size;
        vf.index = vbuffer[i].index;
        ret = amlv4l_queuebuf(amvideo, &vf);
    }
fail:
    return ret;
}

static void ionvideo_close()
{
    amvideo_stop(amvideo);
    amvideo_release(amvideo);
}

static void signal_handler(int signum)
{
    printf("Get signum=%x\n", signum);
#ifdef AUDIO_ES
    codec_close(apcodec);
#endif
    codec_close(vpcodec);
    fclose(fp);
    set_display_axis(1);
    signal(signum, SIG_DFL);
    raise(signum);
    ionvideo_close();
    FreeBuffers();
}

int main(int argc, char *argv[])
{
    int ret = CODEC_ERROR_NONE;
    char buffer[READ_SIZE];

    uint32_t Readlen;
    uint32_t isize;
    struct buf_status vbuf;

    if (argc < 6) {
        printf("Corret command: ionplayer <filename> <width> <height> <fps> <format(1:mpeg4 2:h264)> [subformat for mpeg4]\n");
        return -1;
    }
#if 0
    if (osd_blank("/sys/class/graphics/fb0/blank", 1) < 0)
        osd_blank("/sys/kernel/debug/dri/0/vpu/blank", 1);
    if (osd_blank("/sys/class/graphics/fb1/blank", 0) < 0)
        osd_blank("/sys/kernel/debug/dri/64/vpu/blank", 1);
#endif
    set_display_axis(0);
#ifdef AUDIO_ES
    apcodec = &a_codec_para;
    memset(apcodec, 0, sizeof(codec_para_t));
#endif

    vpcodec = &v_codec_para;
    memset(vpcodec, 0, sizeof(codec_para_t));

    vpcodec->has_video = 1;
    vpcodec->video_type = atoi(argv[5]);
    if (vpcodec->video_type == VFORMAT_H264) {
        vpcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_H264;
        vpcodec->am_sysinfo.param = (void *)(EXTERNAL_PTS | SYNC_OUTSIDE);
    } else if (vpcodec->video_type == VFORMAT_MPEG4) {
        if (argc < 7) {
            printf("No subformat for mpeg4, take the default VIDEO_DEC_FORMAT_MPEG4_5\n");
            vpcodec->am_sysinfo.format = VIDEO_DEC_FORMAT_MPEG4_5;
        } else {
            vpcodec->am_sysinfo.format = atoi(argv[6]);
        }
    }

    vpcodec->stream_type = STREAM_TYPE_ES_VIDEO;
    vpcodec->am_sysinfo.rate = 96000 / atoi(argv[4]);
    vpcodec->am_sysinfo.height = atoi(argv[3]);
    vpcodec->am_sysinfo.width = atoi(argv[2]);
    vpcodec->has_audio = 0;
    vpcodec->noblock = 0;

#ifdef AUDIO_ES
    apcodec->audio_type = AFORMAT_MPEG;
    apcodec->stream_type = STREAM_TYPE_ES_AUDIO;
    apcodec->audio_pid = 0x1023;
    apcodec->has_audio = 1;
    apcodec->audio_channels = 2;
    apcodec->audio_samplerate = 48000;
    apcodec->noblock = 0;
    apcodec->audio_info.channels = 2;
    apcodec->audio_info.sample_rate = 48000;
#endif

    printf("\n*********CODEC PLAYER DEMO************\n\n");
    filename = argv[1];
    printf("file %s to be played\n", filename);

    if ((fp = fopen(filename, "rb")) == NULL) {
        printf("open file error!\n");
        return -1;
    }

    if ((yuv = fopen("./yuv.yuv", "wb")) == NULL) {
        printf("./yuv.yuv dump open file error!\n");
        return -1;
    }

    ionvideo_init(vpcodec->am_sysinfo.width, vpcodec->am_sysinfo.height);
#ifdef AUDIO_ES
    ret = codec_init(apcodec);
    if (ret != CODEC_ERROR_NONE) {
        printf("codec init failed, ret=-0x%x", -ret);
        return -1;
    }
#endif

    ret = codec_init(vpcodec);
    if (ret != CODEC_ERROR_NONE) {
        printf("codec init failed, ret=-0x%x", -ret);
        return -1;
    }
    printf("video codec ok!\n");

    //codec_set_cntl_avthresh(vpcodec, AV_SYNC_THRESH);
    //codec_set_cntl_syncthresh(vpcodec, 0);

    set_tsync_enable(0);

    pcodec = vpcodec;
    while (!feof(fp)) {
        Readlen = fread(buffer, 1, READ_SIZE, fp);
        printf("Readlen %d\n", Readlen);
        if (Readlen <= 0) {
            printf("read file error!\n");
            rewind(fp);
        }

        isize = 0;
        do {
            vframebuf_t vf;
            ret = amlv4l_dequeuebuf(amvideo, &vf);
            if (ret >= 0) {
                printf("vf idx%d pts 0x%llx\n", vf.index, vf.pts);
                fwrite(vbuffer[vf.index].ptr, vbuffer[vf.index].size, 1, yuv);
				fflush(yuv);
                ret = amlv4l_queuebuf(amvideo, &vf);
                if (ret < 0) {
                    printf("amlv4l_queuebuf %d\n", ret);
                }
            } else {
                printf("amlv4l_dequeuebuf %d\n", ret);
            }
            ret = codec_write(pcodec, buffer + isize, Readlen);
            if (ret < 0) {
                if (errno != EAGAIN) {
                    printf("write data failed, errno %d\n", errno);
                    goto error;
                } else {
                    continue;
                }
            } else {
                isize += ret;
            }

            printf("ret %d, isize %d\n", ret, isize);
        } while (isize < Readlen);

        signal(SIGCHLD, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGHUP, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGSEGV, signal_handler);
        signal(SIGINT, signal_handler);
        signal(SIGQUIT, signal_handler);
    }

    do {
        vframebuf_t vf;
        ret = codec_get_vbuf_state(pcodec, &vbuf);
        if (ret != 0) {
            printf("codec_get_vbuf_state error: %x\n", -ret);
            goto error;
        }

        ret = amlv4l_dequeuebuf(amvideo, &vf);
        if (ret >= 0) {
            printf("vf idx%d pts 0x%llx\n", vf.index, vf.pts);
            fwrite(vbuffer[vf.index].ptr, vbuffer[vf.index].size, 1, yuv);
			fflush(yuv);
            ret = amlv4l_queuebuf(amvideo, &vf);
            if (ret < 0) {
                //printf("amlv4l_queuebuf %d\n", ret);
            }
        } else {
            //printf("amlv4l_dequeuebuf %d\n", ret);
        }
    } while (vbuf.data_len > 0x100);

error:
#ifdef AUDIO_ES
    codec_close(apcodec);
#endif
    codec_close(vpcodec);
    fclose(fp);
    fclose(yuv);
    set_display_axis(1);
    ionvideo_close();
    FreeBuffers();
    return 0;
}

