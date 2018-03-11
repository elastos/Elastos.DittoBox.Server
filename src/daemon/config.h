#ifndef __ELA_PFD_CONFIG_H__
#define __ELA_PFD_CONFIG_H__

#include <ela_carrier.h>
#include <ela_session.h>
#include <linkedhashtable.h>

#define MODE_CLIENT     1
#define MODE_SERVER     2

typedef struct {
    char *host;
    char *port;
} PFService;

typedef struct {
    HashEntry he;
    char *userid;
} PFUser;

typedef struct {
    bool udp_enabled;

    size_t bootstraps_size;
    BootstrapNode **bootstraps;

    int loglevel;
    char *logfile;

    char *datadir;

    char *pidfile;

    int mode;
    int options;
    char *serverid;
    char *server_address;

    char *secret_hello;
    char *plain_hello;

    PFService *svc;
    Hashtable *users;
} PFConfig;

PFConfig *load_config(const char *config_file);

#endif /* __ELA_PFD_CONFIG_H__ */
