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

#ifndef PLAYER_LOG_H
#define PLAYER_LOG_H

#define MAX_LOG_SIZE    (20*1024)

__attribute__((format(printf, 2, 3)))
void log_lprint(const int level, const char *fmt, ...);


#define AM_LOG_PANIC    0
#define AM_LOG_FATAL    8
#define AM_LOG_ERROR    16
#define AM_LOG_WARNING  24
#define AM_LOG_INFO     32
#define AM_LOG_VERBOSE  40
#define AM_LOG_DEBUG    60
#define AM_LOG_DEBUG1   70
#define AM_LOG_DEBUG2   80
#define AM_LOG_TRACE    90

#define log_print printf
#define log_error printf
#define log_warning printf
#define log_info printf

/*default global_level=5,
if the level<global_level print out
*/
#define log_debug(fmt...)   log_lprint(AM_LOG_DEBUG,##fmt)
#define log_debug1(fmt...)  log_lprint(AM_LOG_DEBUG1,##fmt)
#define log_debug2(fmt...)  log_lprint(AM_LOG_DEBUG2,##fmt)
#define log_trace(fmt...)   log_lprint(AM_LOG_TRACE,##fmt)

#define  DEBUG_PN() log_print("[%s:%d]\n", __FUNCTION__, __LINE__)

void log_close(void);
int log_open(const char *name);
int update_loglevel_setting(void);
#endif
