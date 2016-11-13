//#pragma once

/* buffer for reading from tun/tap interface, must be >= 1500 */
#define BUFSIZE 131072  
#define PORT 55555

struct net_fds {
    int dev_fd;
    int socket_fd;
};
typedef struct net_fds net_fds_t;

void do_debug(char *msg, ...);
net_fds_t net_setup(char *dev, int flags, char *address, char *netmask);
int cread(int fd, char *buf, int n);
int cwrite(int fd, char *buf, int n);
int read_n(int fd, char *buf, int n);
void my_err(char *msg, ...);
void usage(void);
