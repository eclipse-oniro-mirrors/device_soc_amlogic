/*
 * include/linux/amlogic/unifykey/key_manage.h
 *
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

#ifndef KEYMANAGE1
#define KEYMANAGE1

typedef int32_t (*store_key_ops)(uint8_t *buf, uint32_t len, uint32_t *actual_length);

void storage_ops_read(store_key_ops read);
void storage_ops_write(store_key_ops write);

void *get_ukdev(void);

#endif /* KEYMANAGE1 */
