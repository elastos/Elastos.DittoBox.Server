/*
 * Copyright (c) 2018 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <unistd.h>

#include <libconfig.h>
#include <crystal.h>

#include <ela_carrier.h>

#include "config.h"

#define DEFAULT_PID_FILE "/var/run/elaoc-agentd/elaoc-agentd.pid"

static void config_destructor(void *p)
{
    PFConfig *config = (PFConfig *)p;

    if (!config)
        return;

    if (config->bootstraps) {
        int i;

        for (i = 0; i < config->bootstraps_size; i++)
            deref(config->bootstraps[i]);

        free(config->bootstraps);
    }

    if (config->logfile)
        free(config->logfile);

    if (config->datadir)
        free(config->datadir);

    if (config->pidfile)
        free(config->pidfile);

    if (config->announce_address)
        free(config->announce_address);

    if (config->serverid)
        free(config->serverid);

    if (config->server_address)
        free(config->server_address);

    if (config->secret_hello)
        free(config->secret_hello);

    if (config->plain_hello)
        free(config->plain_hello);

    if (config->svc)
        deref(config->svc);

    if (config->users)
        deref(config->users);
}

static void service_destructor(void *p)
{
    PFService *svc = (PFService *)p;

    if (!svc)
        return;

    if (svc->host)
        free(svc->host);

    if (svc->port)
        free(svc->port);
}

static void user_destructor(void *p)
{
    PFUser *user = (PFUser *)p;

    if (!user)
        return;

    if (user->userid)
        free(user->userid);
}

static void bootstrap_destructor(void *p)
{
    BootstrapNode *node = (BootstrapNode *)p;

    if (!node)
        return;

    if (node->ipv4)
        free((void*)node->ipv4);

    if (node->ipv6)
        free((void*)node->ipv6);

    if (node->port)
        free((void*)node->port);

    if (node->public_key)
        free((void*)node->public_key);
}

PFConfig *load_config(const char *config_file)
{
    PFConfig *config;
    PFService *service;
    config_t cfg;
    config_setting_t *setting;
    const char *stropt;
    int intopt;
    int entries;
    int i;
    int rc;

    config_init(&cfg);

    rc = config_read_file(&cfg, config_file);
    if (!rc) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return NULL;
    }

    config = (PFConfig *)rc_zalloc(sizeof(PFConfig), config_destructor);
    if (!config) {
        fprintf(stderr, "Load configuration failed, out of memory.\n");
        config_destroy(&cfg);
        return NULL;
    }

    rc = config_lookup_bool(&cfg, "udp_enabled", &intopt);
    if (rc && intopt) {
        config->udp_enabled = true;
    }

    setting = config_lookup(&cfg, "bootstraps");
    if (!setting) {
        fprintf(stderr, "Missing bootstraps section.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    entries = config_setting_length(setting);
    if (entries <= 0) {
        fprintf(stderr, "Empty bootstraps option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    config->bootstraps_size = entries;
    config->bootstraps = (BootstrapNode **)calloc(1, config->bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!config->bootstraps) {
        fprintf(stderr, "Out of memory.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < entries; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destructor);
        if (!node) {
            fprintf(stderr, "Out of memory.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        config_setting_t *bs = config_setting_get_elem(setting, i);

        rc = config_setting_lookup_string(bs, "ipv4", &stropt);
        if (rc && *stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        rc = config_setting_lookup_string(bs, "ipv6", &stropt);
        if (rc && *stropt)
            node->ipv6 = (const char *)strdup(stropt);
        else
            node->ipv6 = NULL;

        rc = config_setting_lookup_int(bs, "port", &intopt);
        if (rc && intopt) {
            char number[64];
            sprintf(number, "%d", intopt);
            node->port = (const char *)strdup(number);
        } else
            node->port = NULL;

        rc = config_setting_lookup_string(bs, "public_key", &stropt);
        if (rc && *stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;

        config->bootstraps[i] = node;
    }

    config->loglevel = 3;
    config_lookup_int(&cfg, "loglevel", &config->loglevel);

    rc = config_lookup_string(&cfg, "logfile", &stropt);
    if (rc && *stropt) {
        config->logfile = strdup(stropt);
    }

    rc = config_lookup_string(&cfg, "datadir", &stropt);
    if (!rc || !*stropt) {
        stropt= "data"; // Default datadir
    }
    config->datadir = strdup(stropt);

    rc = config_lookup_string(&cfg, "pidfile", &stropt);
    if (rc && *stropt) {
        config->pidfile = strdup(stropt);
    } else {
        config->pidfile = strdup(DEFAULT_PID_FILE);
    }

    rc = config_lookup_string(&cfg, "announce_address", &stropt);
    if (rc && *stropt) {
        config->announce_address = strdup(stropt);
    } else {
        config->announce_address = NULL;
    }

    rc = config_lookup_string(&cfg, "mode", &stropt);
    if (!rc || !*stropt) {
        fprintf(stderr, "Missing mode option.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    if (strcmp(stropt, "client") == 0) {
        config->mode = MODE_CLIENT;
    } else if (strcmp(stropt, "server") == 0) {
        config->mode = MODE_SERVER;
    } else {
        fprintf(stderr, "Unknown mode '%s'.\n", stropt);
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    if (config->mode == MODE_CLIENT) {
        rc = config_lookup_string(&cfg, "server", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
        config->serverid = strdup(stropt);

        rc = config_lookup_string(&cfg, "server_address", &stropt);
        if (!rc || !*stropt) {
            fprintf(stderr, "Missing server_address option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
        config->server_address = strdup(stropt);
    }

    rc = config_lookup_bool(&cfg, "plain", &intopt);
    if (rc && intopt)
        config->options |= ELA_STREAM_PLAIN;
    config->options |= ELA_STREAM_RELIABLE;

    rc = config_lookup_string(&cfg, "secret_hello", &stropt);
    if (!rc || !*stropt) {
        if (config->mode == MODE_SERVER) {
            fprintf(stderr, "Missing secret_hello option.\n");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }
    } else {
        config->secret_hello = strdup(stropt);
    }

    rc = config_lookup_string(&cfg, "plain_hello", &stropt);
    if (rc && *stropt)
        config->plain_hello = strdup(stropt);

    setting = config_lookup(&cfg, "service");
    if (!setting) {
        fprintf(stderr, "Missing service section.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    service = rc_zalloc(sizeof(PFService), service_destructor);
    if (!service) {
        fprintf(stderr, "Out of memory");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }

    rc = config_setting_lookup_string(setting, "host", &stropt);
    if (!rc || !*stropt)
        stropt = "127.0.0.1";
    service->host = strdup(stropt);

    rc = config_setting_lookup_int(setting, "port", &intopt);
    if (rc && intopt) {
        char number[64];
        sprintf(number, "%d", intopt);
        service->port = strdup(number);
    } else {
        fprintf(stderr, "Missing services port.\n");
        config_destroy(&cfg);
        deref(config);
        return NULL;
    }
    config->svc = service;

    if (config->mode == MODE_SERVER) {
        config->users = hashtable_create(16, 0, NULL, NULL);
        if (!config->users) {
            fprintf(stderr, "Out of memory");
            config_destroy(&cfg);
            deref(config);
            return NULL;
        }

        setting = config_lookup(&cfg, "allowed_users");
        if (!setting)
            return config;

        entries = config_setting_length(setting);
        for (i = 0; i < entries; i++) {
            PFUser *user;

            user = rc_zalloc(sizeof(PFUser), user_destructor);
            if (!user) {
                fprintf(stderr, "Out of memory");
                config_destroy(&cfg);
                deref(config);
                return NULL;
            }

            stropt = config_setting_get_string_elem(setting, i);
            if (!stropt || !*stropt)
                continue;

            if (!ela_id_is_valid(stropt)) {
                fprintf(stderr, "User id '%s' is invalid", stropt);
                config_destroy(&cfg);
                deref(user);
                deref(config);
                return NULL;
            }

            user->userid = strdup(stropt);
            user->he.key = user->userid;
            user->he.keylen = strlen(user->userid);
            user->he.data = user;

            hashtable_put(config->users, &user->he);
            deref(user);
        }
    }

    return config;
}
