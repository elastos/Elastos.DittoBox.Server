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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <crystal.h>
#include <sys/resource.h>

#include <ela_carrier.h>
#include <ela_session.h>

#include "config.h"

static const char *daemon_name = "elaoc-agentd";
static const char *daemon_version = "0.1";

static PFConfig *config;

static ElaCarrier *carrier;

typedef struct SessionEntry {
    hash_entry_t he;
    ElaSession *session;
} SessionEntry;

hashtable_t *sessions;

// Client only
static ElaSession *cli_session;
static int cli_streamid;

static char *create_secret_hello(const char *plain_hello, char *digest, size_t len)
{
    char *sha;

    if (strlen(plain_hello) >= 64) {
        vlogE("Plain hello too long");
        return NULL;
    }

    memset(digest, 0, len);
    sha = sha256a(plain_hello, strlen(plain_hello), digest, len);
    if (!sha) {
        vlogE("Sha256 for plain hello error");
        return NULL;
    }

    return sha;
}

static void session_entry_destroy(void *p)
{
    SessionEntry *entry = (SessionEntry *)p;
    if (entry && entry->session) {
        char peer[ELA_MAX_ID_LEN*2+8];

        ela_session_get_peer(entry->session, peer, sizeof(peer));
        ela_session_close(entry->session);
        vlogI("Session to %s closed", peer);
    }
}

static void add_session(ElaSession *ws)
{
    assert(ws);

    SessionEntry *entry = rc_alloc(sizeof(SessionEntry), session_entry_destroy);
    if (!entry) {
        perror("Out of memory");
        exit(-1);
    }

    entry->he.data = entry;
    entry->he.key = ws;
    entry->he.keylen = sizeof(ElaSession *);
    entry->session = ws;

    hashtable_put(sessions, &entry->he);

    deref(entry);
}

static int exist_session(ElaSession *ws)
{
    if (sessions)
        return hashtable_exist(sessions, ws, sizeof(ElaSession *));
    else
        return 0;
}

static void delete_session(ElaSession *ws)
{
    if (!sessions)
        return;

    SessionEntry *entry = hashtable_remove(sessions, ws, sizeof(ElaSession *));
    if (entry) {
        if (config->mode == MODE_CLIENT) {
            cli_session = NULL;
            cli_streamid = -1;
        }

        deref(entry);
    }
}

static void setup_portforwardings(void);

// Client only
static void peer_connection_changed(ElaConnectionStatus status)
{
    if (status == ElaConnectionStatus_Connected) {
        vlogI("Portforwarding server is online, setup portforwardings...");
        setup_portforwardings();
    } else {
        vlogI("Portforwarding server is being offline.");

        // Close current session if exist
        if (cli_session)
            delete_session(cli_session);

        vlogI("Portforwarding service will available when server peer online.");
    }
}

// Client only
static void carrier_ready(ElaCarrier *w, void *context)
{
    int rc;
    char secret_hello[128] = {0};
    const char *hello = NULL;

    vlogI("Carrier is ready!");

    if (config->mode == MODE_SERVER)
        return; // Server mode: do nothing.

    const char *friendid = config->serverid;

    if (!ela_is_friend(w, friendid)) {
        vlogI("Portforwarding server not friend yet, send friend request...");

        if (config->plain_hello)
            hello = create_secret_hello(config->plain_hello, secret_hello,
                                        sizeof(secret_hello));

        if (!hello)
            hello = "Elastos Carrier PFD/C";

        rc = ela_add_friend(w, config->server_address, hello);
        if (rc < 0) {
            vlogE("Add portforwarding server as friend failed (0x%8X)",
                  ela_get_error());
        } else {
            vlogI("Add portforwarding server as friend success!");
        }
    } else {
        ElaFriendInfo fi;
        ela_get_friend_info(w, friendid, &fi);
        peer_connection_changed(fi.status);
    }
}

static const char *connection_str[] = {
    "Connected",
    "Disconnected",
    "Unknown"
};

// Client only
static void friend_connection(ElaCarrier *w, const char *friendid,
                              ElaConnectionStatus status, void *context)
{
    if (config->mode == MODE_SERVER) {
        vlogD("Friend peer %s changed to %s", friendid, connection_str[status]);
        return; // Server mode: do nothing.
    }

    if (strcmp(friendid, config->serverid) != 0)
        return; // Ignore uninterested peer

    vlogD("Portforwarding server changed to %s", connection_str[status]);
    peer_connection_changed(status);
}

// Server and client
static void friend_request(ElaCarrier *w, const char *userid,
            const ElaUserInfo *info, const char *hello, void *context)
{
    int rc;
    int status = -1;

    if (config->mode == MODE_SERVER && (
            hashtable_exist(config->users, userid, strlen(userid)) ||
            strcmp(hello, config->secret_hello) == 0)) {
        status = 0;
    }

    vlogI("%s friend request from %s.", status == 0 ? "Accept" : "Refuse",
            info->userid);

	if (status == 0) {
        rc = ela_accept_friend(w, userid);
        if (rc < 0) {
            vlogE("Accept friend request failed(%08X).", ela_get_error());
        } else {
            vlogI("Accepted user %s to be friend.", userid);
        }
    } else {
        vlogI("Skipped unathorized friend request from %s.", userid);
    }
}

static const char *state_name[] = {
    "raw",
    "initialized",
    "transport ready",
    "connecting",
    "connected",
    "deactived",
    "closed",
    "error"
};

// Client only
static void session_request_complete(ElaSession *ws, const char *bundle, int status,
                const char *reason, const char *sdp, size_t len, void *context)
{
    ElaStreamState state;
    int rc;

    if (status != 0) {
        vlogE("Session request complete with error(%d:%s).", status, reason);
        return;
    }

    rc = ela_stream_get_state(ws, cli_streamid, &state);
    while (rc == 0 && state < ElaStreamState_transport_ready) {
        usleep(100);
        rc = ela_stream_get_state(ws, cli_streamid, &state);
    }

    if (rc < 0) {
        vlogE("Acquire stream state in session failed(%08X).", ela_get_error());
        delete_session(ws);
        return;
    }

    if (state != ElaStreamState_transport_ready) {
        vlogE("Session stream state wrong %s.", state_name[state]);
        delete_session(ws);
        return;
    }

    rc = ela_session_start(ws, sdp, len);
    if (rc < 0) {
        vlogE("Start session to portforwarding server peer failed(%08X).", ela_get_error());
        delete_session(ws);
    } else
        vlogI("Start session to portforwarding server peer success.");
}

// Server and client
static void stream_state_changed(ElaSession *ws, int stream,
                                 ElaStreamState state, void *context)
{
    int rc;
    char peer[ELA_MAX_ID_LEN*2+8];

    vlogD("Stream %d state changed to %s", stream, state_name[state]);

    ela_session_get_peer(ws, peer, sizeof(peer));

    if (state == ElaStreamState_failed
            || state == ElaStreamState_closed) {
        vlogI("Session to %s closed %s.", peer,
              state == ElaStreamState_closed ? "normally" : "on connection error");

        if (config->mode == MODE_SERVER && exist_session(ws))
            free(ela_session_get_userdata(ws));

        delete_session(ws);
        return;
    }

    if (config->mode == MODE_CLIENT) {
        if (state == ElaStreamState_initialized) {
            rc = ela_session_request(ws, NULL, session_request_complete, NULL);
            if (rc < 0) {
                vlogE("Session request to portforwarding server peer failed(%08X)", ela_get_error());
                delete_session(ws);
            } else {
                vlogI("Session request to portforwarding server success.");
            }
        } else if (state == ElaStreamState_connected) {
            PFService *svc = config->svc;
            int rc;

            rc = ela_stream_open_port_forwarding(ws, stream,
                            "owncloud", PortForwardingProtocol_TCP,
                            svc->host, svc->port);
            if (rc <= 0)
                vlogE("Open portforwarding for owncloud on %s:%s failed(%08X).",
                      svc->host, svc->port, ela_get_error());
            else
                vlogI("Open portforwarding for owncloud on %s:%s success.",
                          svc->host, svc->port);
        }
    } else {
        if (state == ElaStreamState_initialized) {
            rc = ela_session_reply_request(ws, NULL, 0, NULL);
            if (rc < 0) {
                vlogE("Session request from %s, reply failed(%08X)", peer, ela_get_error());
                free(ela_session_get_userdata(ws));
                delete_session(ws);
                return;
            }
            vlogI("Session request from %s, accepted!", peer);
        } else if (state == ElaStreamState_transport_ready) {
            char *sdp = (char *)ela_session_get_userdata(ws);

            rc = ela_session_start(ws, sdp, strlen(sdp));
            ela_session_set_userdata(ws, NULL);
            free(sdp);
            if (rc < 0) {
                vlogE("Start session to %s failed(%08X).", peer, ela_get_error());
                delete_session(ws);
            } else
                vlogI("Start session to %s success.", peer);
        }
    }
}

// Server and client
static void session_request_callback(ElaCarrier *w, const char *from, const char *bundle,
                                   const char *sdp, size_t len, void *context)
{
    ElaSession *ws;
    char userid[ELA_MAX_ID_LEN + 1];
    char *p;
    int rc;
    int options = config->options;

    ElaStreamCallbacks stream_callbacks;

    vlogI("Session request from %s", from);

    ws = ela_session_new(w, from);
    if (ws == NULL) {
        vlogE("Create session failed(%08X).", ela_get_error());
        return;
    }

    if (config->mode == MODE_CLIENT) {
        // Client mode: just refuse the request.
        vlogI("Refuse session request from %s.", from);
        ela_session_reply_request(ws, NULL, -1, "Refuse");
        ela_session_close(ws);
        return;
    }

    p = strchr(from, '@');
    if (p) {
        size_t len = p - from;
        strncpy(userid, from, len);
        userid[len] = 0;
    } else
        strcpy(userid, from);

    rc = ela_session_add_service(ws, "owncloud",
                                 PortForwardingProtocol_TCP,
                                 config->svc->host, config->svc->port);
    if (rc < 0)
        vlogE("Prepare owncloud service for %s failed(%08X).",
              userid, ela_get_error());
    else
        vlogI("Add owncloud service for %s.", userid);

    p = strdup(sdp);
    ela_session_set_userdata(ws, p);

    add_session(ws);
    memset(&stream_callbacks, 0, sizeof(stream_callbacks));
    stream_callbacks.state_changed = stream_state_changed;
    rc = ela_session_add_stream(ws, ElaStreamType_application,
                    options | ELA_STREAM_MULTIPLEXING | ELA_STREAM_PORT_FORWARDING,
                    &stream_callbacks, NULL);
    if (rc <= 0) {
        vlogE("Session request from %s, can not add stream(%08X)", from, ela_get_error());
        ela_session_reply_request(ws, NULL, -1, "Error");
        delete_session(ws);
        free(p);
    }
}

// Client only
static void setup_portforwardings(void)
{
    ElaStreamCallbacks stream_callbacks;
    int options = config->options;

    // May be previous session not closed properly.
    if (cli_session != NULL)
        delete_session(cli_session);

    cli_session = ela_session_new(carrier, config->serverid);
    if (cli_session == NULL) {
        vlogE("Create session to portforwarding server failed(%08X).", ela_get_error());
        return;
    }

    vlogI("Created session to portforwarding server.");

    add_session(cli_session);

    memset(&stream_callbacks, 0, sizeof(stream_callbacks));
    stream_callbacks.state_changed = stream_state_changed;

    cli_streamid = ela_session_add_stream(cli_session, ElaStreamType_application,
                options | ELA_STREAM_MULTIPLEXING | ELA_STREAM_PORT_FORWARDING,
                &stream_callbacks, NULL);
    if (cli_streamid <= 0) {
        vlogE("Add stream to session failed(%08X)", ela_get_error());
        delete_session(cli_session);
    } else {
        vlogI("Add stream %d to session success.", cli_streamid);
    }
}

static void session_close(void)
{
    hashtable_t *ss = sessions;

    sessions = NULL;
    if (ss)
        deref(ss);

    if (carrier) {
        ela_session_cleanup(carrier);
        ela_kill(carrier);
        carrier = NULL;
    }

    if (config) {
        deref(config);
        config = NULL;
    }
}

static void daemonize(const char *pid_file_path)
{
   // Check if the PID file exists
    FILE *pid_file;

    if ((pid_file = fopen(pid_file_path, "r"))) {
        vlogW("Another instance of the daemon is already running, PID file %s exists.\n", pid_file_path);
        fclose(pid_file);
    }

    // Open the PID file for writing
    pid_file = fopen(pid_file_path, "w+");
    if (pid_file == NULL) {
        vlogE("Couldn't open the PID file for writing: %s. Exiting.\n", pid_file_path);
        exit(1);
    }

    const pid_t pid = fork();

    if (pid > 0) {
        fprintf(pid_file, "%d", pid);
        fclose(pid_file);
        vlogI("Forked successfully: PID: %d.\n", pid);
        exit(0);
    } else {
        fclose(pid_file);
    }

    if (pid < 0) {
        vlogE("Forked failed. Exiting");
        exit(1);
    }

    // Create a new SID for the child process
    if (setsid() < 0) {
        vlogE("SID creation failure. Exiting.\n");
        exit(1);
    }
}

static void signal_handler(int signum)
{
    session_close();
}

int sys_coredump_set(bool enable)
{
    const struct rlimit rlim = {
        enable ? RLIM_INFINITY : 0,
        enable ? RLIM_INFINITY : 0
    };

    return setrlimit(RLIMIT_CORE, &rlim);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

static uint32_t session_hash_code(const void *key, size_t len)
{
    return (uint32_t)key;
}

#pragma GCC diagnostic pop

static int session_hash_compare(const void *key1, size_t len1,
                                const void *key2, size_t len2)
{
    if (key1 == key2)
        return 0;
    else if (key1 < key2)
        return -1;
    else
        return 1;
}

#define ADDRESS_FILE    "address"

static int announce_address(ElaCarrier *carrier, const char *datadir, const char *announce_cmd)
{
    char *address_file;
    char uid[ELA_MAX_ID_LEN+1];
    char address[ELA_MAX_ADDRESS_LEN+1];
    char *cmd;
    int fd;

    assert(carrier);
    assert(datadir);

    if (!ela_get_userid(carrier, uid, sizeof(uid)))
        return -1;

    if (!ela_get_address(carrier, address, sizeof(address)))
        return -1;

    vlogI("User ID: %s", uid);
    vlogI("Address: %s", address);

    address_file = (char *)alloca(strlen(datadir) + strlen(ADDRESS_FILE) + 16);
    sprintf(address_file, "%s/%s", datadir, ADDRESS_FILE);

    fd = open(address_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
        return -1;

    write(fd, address, strlen(address));
    close(fd);

    if (announce_cmd) {
        cmd = (char *)alloca(strlen(datadir) + strlen(announce_cmd) + 128);
        sprintf(cmd, "DATADIR=%s ADDRESS=%s sh -c '%s'", datadir, address, announce_cmd);
        int rc = system(cmd);
        if (rc == 0)
            vlogI("Announce the node address success.");
        else
            vlogW("Announce the node address failed, exit code: %d", rc);
    }

    return 0;
}

static void show_version(void)
{
    printf("%s version: %s\n\n", daemon_name, daemon_version);
}

static void show_usage(void)
{
    printf(
        "Usage: %s [OPTION]...\n"
        "\n"
        "Options:\n"
        "  --config=FILE                Specify path to the config file.\n"
        "  --foreground                 Run the daemon in foreground.\n"
        "  --version                    Print version information.\n"
        "  --help                       Print this help message.\n",
        daemon_name);
}

int main(int argc, char *argv[])
{
    ElaOptions opts;
    ElaCallbacks callbacks;
    char buffer[2048] = { 0 };
    int wait_for_attach = 0;
    int run_in_foreground = 0;
    int rc;
    int opt;
    int idx;
    int i;

    sys_coredump_set(true);

    // Uncatchable: signal(SIGKILL, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);

    struct option options[] = {
        { "config",         required_argument,  NULL, 'c' },
        { "foreground",     no_argument,        NULL, 'f' },
        { "debug",          no_argument,        NULL,  1  },
        { "help",           no_argument,        NULL, 'h' },
        { "version",        no_argument,        NULL, 'v' },
        { NULL,             0,                  NULL,  0  }
    };

    while ((opt = getopt_long(argc, argv, "c:h?", options, &idx)) != -1) {
        switch (opt) {
        case 'c':
            strcpy(buffer, optarg);
            break;
        case 'f':
            run_in_foreground = 1;
            break;
        case 1:
            wait_for_attach = 1;
            break;
        case 'v':
            show_version();
            exit(0);
            break;
        case 'h':
        case '?':
        default:
            show_usage();
            exit(-1);
        }
    }

    if (wait_for_attach) {
        printf("Wait for debugger attaching, process id is: %d.\n", getpid());
        printf("After debugger attached, press any key to continue......");
        getchar();
    }

    if (!*buffer) {
        realpath(argv[0], buffer);
        strcat(buffer, ".conf");
    }

    config = load_config(buffer);
    if (!config) {
        return -1;
    }

    if (!run_in_foreground)
        daemonize(config->pidfile);

    // Initialize carrier options.
    memset(&opts, 0, sizeof(opts));

    opts.udp_enabled = config->udp_enabled;
    opts.persistent_location = config->datadir;
    opts.bootstraps_size = config->bootstraps_size;
    opts.bootstraps = (BootstrapNode *)calloc(1, sizeof(BootstrapNode) * opts.bootstraps_size);
    if (!opts.bootstraps) {
        fprintf(stderr, "out of memory.");
        deref(config);
        return -1;
    }

    for (i = 0 ; i < config->bootstraps_size; i++) {
        BootstrapNode *b = &opts.bootstraps[i];
        BootstrapNode *node = config->bootstraps[i];

        b->ipv4 = node->ipv4;
        b->ipv6 = node->ipv6;
        b->port = node->port;
        b->public_key = node->public_key;
    }

    sessions = hashtable_create(16, 1, session_hash_code, session_hash_compare);
    if (!sessions) {
        deref(config);
        return -1;
    }

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.ready = carrier_ready;
    callbacks.friend_connection = friend_connection;
    callbacks.friend_request = friend_request;

    ela_log_init(config->loglevel, config->logfile, NULL);

    carrier = ela_new(&opts, &callbacks, config);
    free(opts.bootstraps);

    if (!carrier) {
        fprintf(stderr, "Can not create Carrier instance (%08X).\n",
                ela_get_error());
        session_close();
        return -1;
    }

    announce_address(carrier, config->datadir, config->announce_address);

    rc = ela_session_init(carrier);
    if (rc < 0) {
        fprintf(stderr, "Can not initialize Carrier session extension (%08X).",
                ela_get_error());
        session_close();
        return -1;
    }

    rc = ela_session_set_callback(carrier, NULL, session_request_callback, NULL);
    if (rc < 0) {
        fprintf(stderr, "Can not initialize Carrier session extension (%08X).",
                ela_get_error());
        session_close();
        return -1;
    }

    rc = ela_run(carrier, 500);
    if (rc < 0)
        fprintf(stderr, "Can not start Carrier.\n");

    session_close();
    return rc;
}
