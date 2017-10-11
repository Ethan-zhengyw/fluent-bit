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

#include <stdio.h>

#include <fluent-bit/flb_output.h>
#include <fluent-bit/flb_utils.h>
#include <fluent-bit/flb_time.h>
#include <msgpack.h>
#include <libgen.h>

#include "obs.h"

int cb_obs_init(struct flb_output_instance *ins, struct flb_config *config,
                   void *data)
{
    flb_debug("[OUT_OBS] cb_obs_init begin.");

    // (void) ins;
    (void) config;
    (void) data;

    char *tmp;
    struct flb_out_obs_config *ctx = NULL;

    ctx = flb_calloc(1, sizeof(struct flb_out_obs_config));
    if (!ctx) {
        flb_errno();
        return -1;
    }

    // load ak
    tmp = flb_output_get_property("ak", ins);
    if (tmp) {
        ctx->ak = tmp;
    } else { 
        flb_free(ctx);
        return -1;
    }

    // load cache path
    tmp = flb_output_get_property("cachepath", ins);
    if (tmp) {
        ctx->cachepath = tmp;
    } else { 
        flb_free(ctx);
        return -1;
    }

    flb_output_set_context(ins, ctx);

    flb_debug("[OUT_OBS] cb_obs_init end.");

    return 0;
}

void cb_obs_flush(void *data, size_t bytes,
                     char *tag, int tag_len,
                     struct flb_input_instance *i_ins,
                     void *out_context,
                     struct flb_config *config)
{
    flb_debug("[OUT_OBS] cb_obs_flush begin.");

    msgpack_unpacked result;
    size_t off = 0, cnt = 0;
    (void) i_ins;
    // (void) out_context;
    (void) config;
    struct flb_time tmp;
    struct flb_out_obs_config *ctx = out_context;
    msgpack_object *p;

    char *i_path = flb_input_get_property("path", i_ins);

    flb_debug("[OUT_OBS] tmp.tm.tv_sec: %d", tmp.tm.tv_sec);
    msgpack_unpacked_init(&result);
    while (msgpack_unpack_next(&result, data, bytes, &off)) {
        printf("[%zd] %s: [", cnt++, tag);
        flb_time_pop_from_msgpack(&tmp, &result, &p);
        printf("%"PRIu32".%09lu, ", (uint32_t)tmp.tm.tv_sec, tmp.tm.tv_nsec);
        msgpack_object_print(stdout, *p);
        printf("]\n");

        // i_path: /var/log/xxx.log
        // dest_path /var/log/xxx.log.20171010020000
        char *bname = basename(strdup(i_path));
        char *dest_path = (char *)malloc(sizeof(char) * (strlen(ctx->cachepath) + strlen(bname) + 16));
        get_log_path_by_timestamp(dest_path, ctx->cachepath, bname, tmp.tm.tv_sec);
	flb_debug("[OUT_OBS] dest_path is: %s", dest_path);

        FILE *fp = fopen(dest_path, "a+");
        if (fp == NULL) {
            flb_errno();
            FLB_OUTPUT_RETURN(FLB_ERROR);
        }

        fprintf(fp, "[%zd] %s: [", cnt - 1, tag);
        fprintf(fp, "%"PRIu32".%09lu, ", (uint32_t)tmp.tm.tv_sec, tmp.tm.tv_nsec);
        msgpack_object_print(fp, *p);
        fprintf(fp, "]\n");
        fclose(fp);
        free(dest_path);
    }
    
    // TODO
    // Upload "ready log file" to OBS
    // 

    flb_debug("[OUT_OBS] tmp.tm.tv_sec: %d", tmp.tm.tv_sec);
    msgpack_unpacked_destroy(&result);

    flb_debug("[OUT_OBS] ak is: %s", ctx->ak);
    flb_debug("[OUT_OBS] data is: %s", data);
    flb_debug("[OUT_OBS] bytes is: %d", bytes);
    flb_debug("[OUT_OBS] tag is: %s", tag);
    flb_debug("[OUT_OBS] tag_len is: %d", tag_len);
    flb_debug("[OUT_OBS] i_ins->name is: %s", i_ins->name);
    flb_debug("[OUT_OBS] i_ins property path is: %s", flb_input_get_property("path", i_ins));
    flb_debug("[OUT_OBS] config->flush is: %d", config->flush);

    // printf(bytes);
    // flb_debug(tag);
    // flb_debug(tag_len);

    flb_debug("[OUT_OBS] cb_obs_flush end.");

    FLB_OUTPUT_RETURN(FLB_OK);
}

int cb_obs_exit(void *data, struct flb_config *config)
{
    flb_debug("[OUT_OBS] cb_obs_exit begin.");
    
    struct flb_out_obs_config *ctx = data;
    // if (ctx->u) {
    //     flb_upstream_destroy(ctx->u);
    // }

    flb_free(ctx->ak);
    flb_free(ctx->cachepath);
    // flb_free(ctx->sk);
    // flb_free(ctx->endPoint);
    // flb_free(ctx->bucketName);
    flb_free(ctx);
    
    flb_debug("[OUT_OBS] cb_obs_exit end.");

    return 0;
}

struct flb_output_plugin out_obs_plugin = {
    .name         = "obs",
    .description  = "Output to Huawei OBS",
    .cb_init      = cb_obs_init,
    .cb_flush     = cb_obs_flush,
    .cb_exit      = cb_obs_exit,
    // .flags        = 0,
};
