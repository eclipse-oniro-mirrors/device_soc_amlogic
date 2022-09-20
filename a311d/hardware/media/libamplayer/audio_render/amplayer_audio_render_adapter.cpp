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

#include <mutex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <thread>
#include <map>
#include <condition_variable>
#include "errors.h"
#include "media_log.h"
#include "audio_renderer_sink.h"

#include "audio_external_render.h"

namespace
{
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "AMPLAYER_AUDIO_RENDER"};
}

namespace OHOS
{
namespace AudioStandard
{

AudioRendererSink *g_audioRendrSinkInstance = AudioRendererSink::GetInstance();

static int audio_render_init(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(!g_audioRendrSinkInstance->rendererInited_, 0);

    AudioSinkAttr sample_attrs;
    int32_t ret;

    sample_attrs.adapterName = "primary";
    sample_attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    sample_attrs.sampleFmt = AUDIO_FORMAT_PCM_16_BIT;
    sample_attrs.sampleRate = 48000;
    sample_attrs.channel = 2;
    sample_attrs.volume = 1.0;

    ret = g_audioRendrSinkInstance->Init(sample_attrs);
    if (ret != 0) {
        MEDIA_LOGE("Error: audiorender Init() failed!");
        return -1;
    }

    return 0;
}

static int audio_render_finalize(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);

    g_audioRendrSinkInstance->Stop();
    g_audioRendrSinkInstance->DeInit();

    return 0;
}

static int audio_render_config(struct audio_render_s *r, uint32_t channels,
                               uint32_t bits_per_sample, uint32_t sample_rate)
{
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);

    MEDIA_LOGD("audio_render_config in, channels:%{public}d, sampleRate:%{public}d", channels, sample_rate);

    return 0;
}

static int audio_render_write(struct audio_render_s *r, uint8_t *data, uint32_t size)
{
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    MEDIA_LOGD("%{public}s() size=%{public}u", __func__, size);
    uint64_t writeLen = 0;
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->RenderFrame(*((char *)data), size, writeLen) == 0, -1);
    return (int)writeLen;
}

static int audio_render_start(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->Start() == 0, -1);
    return 0;
}

static int audio_render_stop(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->Stop() == 0, -1);
    return 0;
}

static int audio_render_pause(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->Pause() == 0, -1);
    return 0;
}

static int audio_render_resume(struct audio_render_s *r)
{
    MEDIA_LOGD("%{public}s() in", __func__);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->Resume() == 0, -1);
    return 0;
}

static uint32_t get_latency_from_driver(struct audio_render_s *r)
{
    uint32_t latency = 0;
    char line[256];
    FILE *fp = fopen("/proc/asound/card0/pcm1p/sub0/status", "rt");
    if (!fp) {
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        line[sizeof(line) - 1] = '\0';
        if (!strncmp(line, "delay", 5)) {
            uint32_t frames = 0;
            if (1 != sscanf(line, "%*[^:]:%u", &frames)) {
                MEDIA_LOGE("Error: parse line failed: %{public}s", line);
                break;
            }
            latency = (frames * 1000) / 48000;
            // MEDIA_LOGD("get latency: %{public}u, %{public}u", frames, latency);
            break;
        }
    }

    fclose(fp);

    return latency;
}

static uint32_t audio_render_get_latency(struct audio_render_s *r)
{
    uint32_t latency;

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    // CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->GetLatency(&latency) == 0, -1);
    latency = get_latency_from_driver(r);

    return latency;
}

static int audio_render_set_mute(struct audio_render_s *r, int mute)
{
    MEDIA_LOGD("%{public}s() in. mute=%{public}d", __func__, mute);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    return 0;
}

static int audio_render_set_volume(struct audio_render_s *r, float volume)
{
    MEDIA_LOGD("%{public}s() in. volume=%{public}f", __func__, volume);

    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->rendererInited_, -1);
    CHECK_AND_RETURN_RET(g_audioRendrSinkInstance->SetVolume(volume, volume) == 0, -1);
    return 0;
}

static audio_render_t audio_render_ops = {
    .init = audio_render_init,
    .finalize = audio_render_finalize,
    .config = audio_render_config,
    .write = audio_render_write,
    .start = audio_render_start,
    .stop = audio_render_stop,
    .pause = audio_render_pause,
    .resume = audio_render_resume,
    .get_latency = audio_render_get_latency,
    .set_mute = audio_render_set_mute,
    .set_volume = audio_render_set_volume
};

} // namespace AudioStandard
} // namespace OHOS

__attribute__((visibility("default"))) audio_render_t *audio_render_get(void)
{
    return &OHOS::AudioStandard::audio_render_ops;
}
