/**************************************************************************
* simpletun.c                                                            *
*                                                                        *
* A simplistic, simple-minded, naive tunnelling program using tun/tap    *
* interfaces and TCP. DO NOT USE THIS PROGRAM FOR SERIOUS PURPOSES.      *
*                                                                        *
* You have been warned.                                                  *
*                                                                        *
* (C) 2010 Davide Brini.                                                 *
*                                                                        *
* DISCLAIMER AND WARNING: this is all work in progress. The code is      *
* ugly, the algorithms are naive, error checking and input validation    *
* are very basic, and of course there can be bugs. If that's not enough, *
* the program has not been thoroughly tested, so it might even fail at   *
* the few simple things it should be supposed to do right.               *
* Needless to say, I take no responsibility whatsoever for what the      *
* program might do. The program has been written mostly for learning     *
* purposes, and can be used in the hope that is useful, but everything   *
* is to be taken "as is" and without any kind of warranty, implicit or   *
* explicit. See the file LICENSE for further details.                    *
*************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <unistd.h>
#include <errno.h>

#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <sys/select.h>
#include <sys/time.h>

/* buffer for reading from tun/tap interface, must be >= 1500 */
#define BUFSIZE 2000   
#define PORT 55555

struct net_fds {
    int dev_fd;
    int socket_fd;
};
typedef struct net_fds net_fds_t;

int debug;
char *progname;

/**************************************************************************
* do_debug: prints debugging stuff (doh!)                                *
**************************************************************************/
void do_debug(char *msg, ...){

    va_list argp;

    if(debug) {
        va_start(argp, msg);
        vfprintf(stderr, msg, argp);
        va_end(argp);
    }
}

/**************************************************************************
* net_setup: allocates or reconnects to a tun/tap device. The caller     *
*            must reserve enough space in *dev. If address is not empty, *
*            the device will be assigned to that address.
**************************************************************************/
net_fds_t net_setup(char *dev, int flags, char *address) {
    struct ifreq ifr;
    int fd, err, socket_fd;
    char *clonedev = "/dev/net/tun";

    if( (fd = open(clonedev , O_RDWR)) < 0 ) {
        perror("Opening /dev/net/tun");
        printf("Error1\n");
        goto ret;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    } else {
        fprintf(stderr, "ERROR! Can not create device!\n");
        exit(0);
    }

    if( (err = ioctl(fd, TUNSETIFF, &ifr)) < 0 ) {
        perror("ioctl(TUNSETIFF)");
        close(fd);
        printf("error2\n");
        goto ret;
    }

    strcpy(dev, ifr.ifr_name);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (strlen(address) != 0) {
        do_debug("Setting device IP address to %s\n", address);
        struct sockaddr_in sin = {
            .sin_family = AF_INET,
        };
        //struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
        //memset(sin, 0, sizeof(struct sockaddr_in));

        // Set IP address, netmask, and mark interface as up
        // IP
        if (inet_pton(AF_INET, address, &sin.sin_addr.s_addr) == 0) {
            fprintf(stderr, "Invalid IP Address!\n");
        }
        memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr_in));
        if ((err = ioctl(socket_fd, SIOCSIFADDR, &ifr)) < 0) {
            perror("ioctl(SIOCSIFADDR)");
        }

        // netmask
        if (inet_pton(AF_INET, "255.255.255.0", &sin.sin_addr.s_addr) == 0) {
            fprintf(stderr, "Invalid netmask!\n");
        }
        memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr_in));
        if ((err = ioctl(socket_fd, SIOCSIFNETMASK, &ifr)) < 0) {
            perror("ioctl(SIOCSIFNETMASK)");
        }

        // flags (UP, RUNNING)
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        if ((err = ioctl(socket_fd, SIOCSIFFLAGS, &ifr)) < 0) {
            perror("ioctl(SIOCSIFFLAGS)");
        }
    }

ret:
    ; // Empty statement to allow declaration after a goto
    net_fds_t retval = {
        .dev_fd = fd,
        .socket_fd = socket_fd
    };

    return retval;
}

/**************************************************************************
* cread: read routine that checks for errors and exits if an error is    *
*        returned.                                                       *
**************************************************************************/
int cread(int fd, char *buf, int n){
    int nread;

    if((nread=read(fd, buf, n)) < 0){
        perror("Reading data");
        exit(1);
    }
    return nread;
}

/**************************************************************************
* cwrite: write routine that checks for errors and exits if an error is  *
*         returned.                                                      *
**************************************************************************/
int cwrite(int fd, char *buf, int n){

    int nwrite;

    if((nwrite=write(fd, buf, n)) < 0){
        perror("Writing data");
        exit(1);
    }
    return nwrite;
}

/**************************************************************************
* read_n: ensures we read exactly n bytes, and puts them into "buf".     *
*         (unless EOF, of course)                                        *
**************************************************************************/
int read_n(int fd, char *buf, int n) {

    int nread, left = n;

    while(left > 0) {
        if ((nread = cread(fd, buf, left)) == 0){
            return 0 ;      
        } else {
            left -= nread;
            buf += nread;
        }
    }
    return n;  
}

/**************************************************************************
* my_err: prints custom error messages on stderr.                        *
**************************************************************************/
void my_err(char *msg, ...) {

    va_list argp;

    va_start(argp, msg);
    vfprintf(stderr, msg, argp);
    va_end(argp);
}

/**************************************************************************
* usage: prints usage and exits.                                         *
**************************************************************************/
void usage(void) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s -i <ifacename> [-s|-c <serverIP>] [-p <port>] [-u|-a] [-d]\n", progname);
    fprintf(stderr, "%s -h\n", progname);
    fprintf(stderr, "\n");
    fprintf(stderr, "-i <ifacename>: Name of interface to use (mandatory)\n");
    fprintf(stderr, "-s|-c <serverIP>: run in server mode (-s), or specify server address (-c <serverIP>) (mandatory)\n");
    fprintf(stderr, "-p <port>: port to listen on (if run in server mode) or to connect to (in client mode), default 55555\n");
    fprintf(stderr, "-u|-a: use TUN (-u, default) or TAP (-a)\n");
    fprintf(stderr, "-d: outputs debug information while running\n");
    fprintf(stderr, "-h: prints this help text\n");
    exit(1);
}

int main(int argc, char *argv[]) {

    int net_fd, option;
    int flags = IFF_TUN;
    char if_name[IFNAMSIZ] = "";
    int maxfd;
    uint16_t nread, nwrite, plength;
    char buffer[BUFSIZE];
    struct sockaddr_in local, remote;
    char remote_ip[16] = "";            /* dotted quad IP string */
    char local_ip[16] = "";
    uint16_t port = PORT;
    int optval = 1;
    socklen_t remotelen;
    int is_server = 2;
    uint64_t tap2net = 0, net2tap = 0;

    progname = argv[0];

    /* Check command line options */
    while((option = getopt(argc, argv, "i:sc:p:uahdl:")) > 0) {
        switch(option) {
            case 'd':
            debug = 1;
            break;
            case 'h':
            usage();
            break;
            case 'i':
            strncpy(if_name,optarg, IFNAMSIZ-1);
            break;
            case 's':
            is_server = true;
            break;
            case 'c':
            is_server = false;
            strncpy(remote_ip, optarg, 15);
            break;
            case 'p':
            port = atoi(optarg);
            break;
            case 'u':
            flags = IFF_TUN;
            break;
            case 'a':
            flags = IFF_TAP;
            break;
            case 'l':
            strncpy(local_ip, optarg, 15);
            break;
            default:
            my_err("Unknown option %c\n", option);
            usage();
        }
    }

    argv += optind;
    argc -= optind;

    if(argc > 0) {
        my_err("Too many options!\n");
        usage();
    }

    if(*if_name == '\0') {
        my_err("Must specify interface name!\n");
        usage();
    } else if(is_server > 1) {
        my_err("Must specify client or server mode!\n");
        usage();
    } else if(!is_server && *remote_ip == '\0') {
        my_err("Must specify server address!\n");
        usage();
    }

    /* initialize network */
    net_fds_t fds = net_setup(if_name, flags | IFF_NO_PI, local_ip);
    if ( fds.dev_fd < 0 ) {
        my_err("Error connecting to tun/tap interface %s!\n", if_name);
        exit(1);
    }

    if ( fds.socket_fd < 0) {
        perror("socket()");
        exit(1);
    }
    // Mutable variable to access the socket file descriptor
    net_fd = fds.socket_fd;
    
    do_debug("Successfully connected to interface %s\n", if_name);
    do_debug("DEV_FD: %d\nSOC_FD: %d\n", fds.dev_fd, fds.socket_fd);


    if(!is_server) {
        /* Client, try to connect to server */

        /* assign the destination address */
        memset(&remote, 0, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_addr.s_addr = inet_addr(remote_ip);
        remote.sin_port = htons(port);

        /* connection request */
        if (connect(fds.socket_fd, (struct sockaddr*) &remote, sizeof(remote)) < 0) {
            perror("connect()");
            exit(1);
        }

        do_debug("CLIENT: Connected to server %s\n", inet_ntoa(remote.sin_addr));

    } else {
        /* Server, wait for connections */

        /* avoid EADDRINUSE error on bind() */
        if(setsockopt(fds.socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
            perror("setsockopt()");
            exit(1);
        }

        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_port = htons(port);
        if (bind(fds.socket_fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
            perror("bind()");
            exit(1);
        }

        if (listen(fds.socket_fd, 5) < 0) {
            perror("listen()");
            exit(1);
        }

        /* wait for connection request */
        remotelen = sizeof(remote);
        memset(&remote, 0, remotelen);
        if ((net_fd = accept(fds.socket_fd, (struct sockaddr*)&remote, &remotelen)) < 0) {
            perror("accept()");
            exit(1);
        }

        do_debug("SERVER: Client connected from %s\n", inet_ntoa(remote.sin_addr));
    }

    /* use select() to handle two descriptors at once */
    maxfd = (fds.dev_fd > net_fd)?fds.dev_fd:net_fd;

    while(1) {
        int ret;
        fd_set rd_set;

        FD_ZERO(&rd_set);
        FD_SET(fds.dev_fd, &rd_set); FD_SET(net_fd, &rd_set);

        ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

        if (ret < 0 && errno == EINTR){
            continue;
        }

        if (ret < 0) {
            perror("select()");
            exit(1);
        }

        if(FD_ISSET(fds.dev_fd, &rd_set)) {
            /* data from tun/tap: just read it and write it to the network */

            nread = cread(fds.dev_fd, buffer, BUFSIZE);

            tap2net++;
            do_debug("TAP2NET %lu: Read %d bytes from the tap interface\n", tap2net, nread);

            /* write length + packet */
            plength = htons(nread);
            nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength));
            nwrite = cwrite(net_fd, buffer, nread);

            do_debug("TAP2NET %lu: Written %d bytes to the network\n", tap2net, nwrite);
        }

        if(FD_ISSET(net_fd, &rd_set)) {
            /* data from the network: read it, and write it to the tun/tap interface. 
             * We need to read the length first, and then the packet */

            /* Read length */      
            nread = read_n(net_fd, (char *)&plength, sizeof(plength));
            if(nread == 0) {
            /* ctrl-c at the other end */
                break;
            }

            net2tap++;

            /* read packet */
            nread = read_n(net_fd, buffer, ntohs(plength));
            do_debug("NET2TAP %lu: Read %d bytes from the network\n", net2tap, nread);

            /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */ 
            nwrite = cwrite(fds.dev_fd, buffer, nread);
            do_debug("NET2TAP %lu: Written %d bytes to the tap interface\n", net2tap, nwrite);
        }
    }

    return(0);
}
