/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015-2017 Treasure Data Inc.
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

#ifndef FLB_OUT_OBS
#define FLB_OUT_OBS

#include <string.h>

struct flb_out_obs_config {
    char *ak;
    char *cachepath;
    // char *sk;
    // char *endPoint;
    // char *bucketName
};


/*
 * e.g. 
 * cache_path = /tmp/cache
 * log_file = xxx.log
 * timestamp = 1491234123 (Mon Apr  3 11:42:03 EDT 2017)
 * 
 * dest_path = /tmp/cache/xxx.log.20170403110000 
 */
static void get_log_path_by_timestamp(char *dest_path, char *cache_path, char *log_file, long long timestamp)
{
    flb_info("%s", cache_path);
    flb_info("%s", log_file);
    flb_info("%d", timestamp);

    struct tm *ptm = localtime((time_t *)&timestamp);

    strcpy(dest_path, cache_path);
    strcpy(dest_path + strlen(cache_path), "/");
    strcpy(dest_path + strlen(cache_path) + 1, log_file);
    strcpy(dest_path + strlen(cache_path) + 1 + strlen(log_file), ".");
    strftime(dest_path + strlen(cache_path) + 1 + strlen(log_file) + 1, 15, "%Y%m%d%H0000", ptm);
     
    flb_info("timestamp is %d, dest_path is %s", timestamp, dest_path);
}

#endif
