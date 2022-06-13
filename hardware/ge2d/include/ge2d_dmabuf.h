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

#ifndef GE2D_DMABUF_H
#define GE2D_DMABUF_H

#if defined (__cplusplus)
extern "C" {
#endif

enum GE2DBufType {
    GE2D_BUF_INPUT1,
    GE2D_BUF_INPUT2,
    GE2D_BUF_OUTPUT,
};

int dmabuf_alloc(int ge2d_fd, int type, unsigned int len);
int dmabuf_sync_for_device(int ge2d_fd, int dma_fd);
int dmabuf_sync_for_cpu(int ge2d_fd, int dma_fd);

#if defined (__cplusplus)
}
#endif
#endif