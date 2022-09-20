/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __SYS_CORE_ION_H
#define __SYS_CORE_ION_H

#include <sys/types.h>
#include <linux/ion.h>

#ifdef  __cplusplus
extern "C" {
#endif

int ion_open();

int ion_close(int fd);

int ion_alloc(int fd, size_t len, size_t align, enum ion_heap_type heap_type, 
                int *buffer_fd, unsigned char **vaddr, size_t *map_len);
 
int ion_free(int fd, int buffer_fd, unsigned char *vaddr, size_t map_len);

#ifdef  __cplusplus
}
#endif

#endif /* __SYS_CORE_ION_H */
