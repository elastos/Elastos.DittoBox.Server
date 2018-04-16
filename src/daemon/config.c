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

#include <confuse.h>
#include <rc_mem.h>

#include <ela_carrier.h>

#include "config.h"

#define DEFAULT_PID_FILE "/var/run/elaoc-agentd/elaoc-agentd.pid"

static void config_error(cfg_t *cfg, const char *fmt, va_list ap)
{
    fprintf(stderr, "Config file error, line %d: ", cfg->line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
}

static int mode_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    const char *stropt = cfg_opt_getnstr(opt, cfg_opt_size(opt) - 1);
    if (stropt == NULL) {
        cfg_error(cfg, "Missing mode option");
        return -1;
    }

    if (strcmp(stropt, "client") != 0 && strcmp(stropt, "server") != 0) {
        cfg_error(cfg, "Unknown mode '%s'", stropt);
        return -1;
    }

    return 0;
}

static int service_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    cfg_t *sec;

    sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

    if (cfg_getstr(sec, "port") == NULL) {
        cfg_error(cfg, "port option missing");
        return -1;
    }

    return 0;
}

static int not_null_validator(cfg_t *cfg, cfg_opt_t *opt)
{
    if (cfg_getstr(cfg, opt->name) == NULL) {
        cfg_error(cfg, "option '%s' missing.", opt->name);
        return -1;
    }

    return 0;
}

static void config_destroy(void *p)
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

static void service_destroy(void *p)
{
    PFService *svc = (PFService *)p;

    if (!svc)
        return;

    if (svc->host)
        free(svc->host);

    if (svc->port)
        free(svc->port);
}

static void user_destroy(void *p)
{
    PFUser *user = (PFUser *)p;

    if (!user)
        return;

    if (user->userid)
        free(user->userid);
}

static void bootstrap_destroy(void *p)
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
    cfg_t *cfg, *svc, *sec;
    cfg_t *bootstraps;
    const char *stropt;
    int nsecs;
    int i;
    int rc;
    bool is_plain;

    cfg_opt_t service_opts[] = {
        CFG_STR("host", "127.0.0.1", CFGF_NONE),
        CFG_STR("port", NULL, CFGF_NODEFAULT),
        CFG_END()
    };

    cfg_opt_t bootstrap_opts[] = {
        CFG_STR("ipv4", NULL, CFGF_NONE),
        CFG_STR("ipv6", NULL, CFGF_NONE),
        CFG_STR("port", "33445", CFGF_NONE),
        CFG_STR("public_key", NULL, CFGF_NONE),
        CFG_END()
    };

    cfg_opt_t bootstraps_opts[] = {
        CFG_SEC("bootstrap", bootstrap_opts, CFGF_MULTI | CFGF_NO_TITLE_DUPES),
    };

    cfg_opt_t cfg_opts[] = {
        CFG_BOOL("udp_enabled", true, CFGF_NONE),
        CFG_SEC("bootstraps", bootstraps_opts, CFGF_NONE),
        CFG_INT("loglevel", 3, CFGF_NONE),
        CFG_STR("logfile", NULL, CFGF_NONE),
        CFG_STR("datadir", NULL, CFGF_NONE),
        CFG_STR("pidfile", NULL, CFGF_NONE),
        CFG_STR("mode", NULL, CFGF_NODEFAULT),
        CFG_BOOL("plain", 0, CFGF_NONE),
        CFG_STR("server", NULL, CFGF_NONE),
        CFG_STR("server_address", NULL, CFGF_NONE),
        CFG_STR("secret_hello", NULL, CFGF_NONE),
        CFG_STR("plain_hello", NULL, CFGF_NONE),
        CFG_SEC("service", service_opts, CFGF_NONE ),
        CFG_STR_LIST("allowed_users", NULL, CFGF_NONE),
        CFG_END()
    };

    cfg = cfg_init(cfg_opts, CFGF_NONE);
    cfg_set_error_function(cfg, config_error);
    cfg_set_validate_func(cfg, NULL, not_null_validator);
    cfg_set_validate_func(cfg, "mode", mode_validator);
    cfg_set_validate_func(cfg, "service", service_validator);

    rc = cfg_parse(cfg, config_file);
    if (rc != CFG_SUCCESS) {
        cfg_error(cfg, "can not parse config file: %s.", config_file);
        cfg_free(cfg);
        return NULL;
    }

    config = (PFConfig *)rc_zalloc(sizeof(PFConfig), config_destroy);
    if (!config) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        return NULL;
    }

    config->udp_enabled = cfg_getbool(cfg, "udp_enabled");

    bootstraps = cfg_getsec(cfg, "bootstraps");
    if (!bootstraps) {
        cfg_error(cfg, "missing services section.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    nsecs = cfg_size(bootstraps, "bootstrap");

    config->bootstraps_size = nsecs;
    config->bootstraps = (BootstrapNode **)calloc(1, config->bootstraps_size *
                                                  sizeof(BootstrapNode *));
    if (!config->bootstraps) {
        cfg_error(cfg, "out of memory.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    for (i = 0; i < nsecs; i++) {
        BootstrapNode *node;

        node = rc_zalloc(sizeof(BootstrapNode), bootstrap_destroy);
        if (!node) {
            cfg_error(cfg, "out of memory.");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        sec = cfg_getnsec(bootstraps, "bootstrap", i);

        stropt = cfg_getstr(sec, "ipv4");
        if (stropt)
            node->ipv4 = (const char *)strdup(stropt);
        else
            node->ipv4 = NULL;

        stropt = cfg_getstr(sec, "ipv6");
        if (stropt)
            node->ipv6 = (const char *)strdup(stropt);
        else
            node->ipv6 = NULL;

        stropt = cfg_getstr(sec, "port");
        if (stropt)
            node->port = (const char *)strdup(stropt);
        else
            node->port = NULL;

        stropt = cfg_getstr(sec, "public_key");
        if (stropt)
            node->public_key = (const char *)strdup(stropt);
        else
            node->public_key = NULL;

        config->bootstraps[i] = node;
    }

    config->loglevel = (int)cfg_getint(cfg, "loglevel");

    stropt = cfg_getstr(cfg, "logfile");
    if (stropt)
        config->logfile = strdup(stropt);

    stropt = cfg_getstr(cfg, "datadir");
    if (!stropt) {
        char datadir[PATH_MAX];

        sprintf(datadir, "%s/%s/%s", getenv("HOME"), ".elaoc",
                config->mode == MODE_CLIENT ? "client" : "server");
        config->datadir = strdup(datadir);
    } else
        config->datadir = strdup(stropt);

    stropt = cfg_getstr(cfg, "pidfile");
    if (stropt) {
        config->pidfile = strdup(stropt);
    } else {
        config->pidfile = strdup(DEFAULT_PID_FILE);
    }

    stropt = cfg_getstr(cfg, "mode");
    if (!stropt) {
        cfg_error(cfg, "missing mode option.");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    is_plain = cfg_getbool(cfg, "plain");
    if (is_plain)
        config->options |= ELA_STREAM_PLAIN;
    config->options |= ELA_STREAM_RELIABLE;

    if (strcmp(stropt, "client") == 0) {
        config->mode = MODE_CLIENT;
    } else if (strcmp(stropt, "server") == 0) {
        config->mode = MODE_SERVER;
    } else {
        cfg_error(cfg, "unknown mode '%s'", stropt);
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    stropt = cfg_getstr(cfg, "server");
    if (!stropt) {
        if (config->mode == MODE_CLIENT) {
            cfg_error(cfg, "missing server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }
    } else {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "undesired server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->serverid = strdup(stropt);
    }

    stropt = cfg_getstr(cfg, "server_address");
    if (!stropt) {
        if (config->mode == MODE_CLIENT) {
            cfg_error(cfg, "missing server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }
    } else {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "undesired server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->server_address = strdup(stropt);
    }

    stropt = cfg_getstr(cfg, "secret_hello");
    if (!stropt) {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "missing secret_hello");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }
    } else {
        if (config->mode == MODE_CLIENT) {
            cfg_error(cfg, "undesired server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->secret_hello = strdup(stropt);
    }

    stropt = cfg_getstr(cfg, "plain_hello");
    if (stropt) {
        if (config->mode == MODE_SERVER) {
            cfg_error(cfg, "undesired server option");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        config->plain_hello = strdup(stropt);
    }

    svc = cfg_getsec(cfg, "service");
    if (!svc) {
        cfg_error(cfg, "Missing service options");
        cfg_free(cfg);
        deref(config);
        return NULL;
    }

    service = rc_zalloc(sizeof(PFService), service_destroy);
    if (!service) {
        cfg_error(cfg, "Out of memory");
        cfg_free(cfg);
        deref(cfg);
        return NULL;
    }

    stropt = cfg_getstr(svc, "host");
    if (!stropt)
        stropt = "127.0.0.1";
    service->host = strdup(stropt);

    stropt = cfg_getstr(svc, "port");
    service->port = strdup(stropt);
    config->svc = service;

    if (config->mode == MODE_SERVER) {
        int n;
        int i;

        n = cfg_size(cfg, "allowed_users");
        config->users = hashtable_create(n * 2, 0, NULL, NULL);
        if (!config->users) {
            cfg_error(cfg, "out of memory");
            cfg_free(cfg);
            deref(config);
            return NULL;
        }

        for (i = 0; i < n; i++) {
            PFUser *user;

            user = rc_zalloc(sizeof(PFUser), user_destroy);
            if (!user) {
                cfg_error(cfg, "Out of memory");
                cfg_free(cfg);
                deref(config);
                return NULL;
            }

            stropt = cfg_getnstr(cfg, "allowed_users", i);
            if (!ela_id_is_valid(stropt)) {
                cfg_error(cfg, "User id '%s' is invalid", stropt);
                cfg_free(cfg);
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
