/*
 * Copyright (C) 2014 Amlogic Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IONMEM_H
#define IONMEM_H
#include <ion/ion.h>
#if defined (__cplusplus)
extern "C" {
#endif

#define ION_IOC_MESON_PHYS_ADDR 8


struct meson_phys_data{
    int handle;
    unsigned int phys_addr;
    unsigned int size;
};

typedef struct IONMEM_AllocParams {
    int                 mIonHnd;
    size_t              size;
    unsigned char       *vaddr;
    size_t              map_len;
} IONMEM_AllocParams;


#define ION_IOC_MAGIC       'I'

#define ION_IOC_CUSTOM      _IOWR(ION_IOC_MAGIC, 6, struct ion_custom_data)

#ifdef __DEBUG
#define __D(fmt, args...) fprintf(stderr, "CMEM Debug: " fmt, ## args)
#else
#define __D(fmt, args...)
#endif

#define __E(fmt, args...) fprintf(stderr, "CMEM Error: " fmt, ## args)


int CMEM_init(void);
unsigned long CMEM_alloc(size_t size, IONMEM_AllocParams *params);
int CMEM_getPhyPtr(IONMEM_AllocParams *params, unsigned long *PhyAdrr);
/*void* CMEM_getUsrPtr(unsigned long PhyAdr, int size);*/
int CMEM_free(IONMEM_AllocParams *params);
int CMEM_exit(void);

#if defined (__cplusplus)
}
#endif

#endif

