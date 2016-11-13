#include <string.h>

void get_full_netmask(char *buf, int short_mask) {
    switch (short_mask) {
        case 30:
            strncpy(buf, "255.255.255.252", 15);
            break;
        case 29:
            strncpy(buf, "255.255.255.248", 15);
            break;
        case 28:
            strncpy(buf, "255.255.255.240", 15);
            break;
        case 27:
            strncpy(buf, "255.255.255.224", 15);
            break;
        case 26:
            strncpy(buf, "255.255.255.192", 15);
            break;
        case 25:
            strncpy(buf, "255.255.255.128", 15);
            break;
        case 24:
            strncpy(buf, "255.255.255.0", 13);
            break;
        case 23:
            strncpy(buf, "255.255.254.0", 13);
            break;
        case 22:
            strncpy(buf, "255.255.252.0", 13);
            break;
        case 21:
            strncpy(buf, "255.255.248.0", 13);
            break;
        case 20:
            strncpy(buf, "255.255.240.0", 13);
            break;
        case 19:
            strncpy(buf, "255.255.224.0", 13);
            break;
        case 18:
            strncpy(buf, "255.255.192.0", 13);
            break;
        case 17:
            strncpy(buf, "255.255.128.0", 13);
            break;
        case 16:
            strncpy(buf, "255.255.0.0", 13);
            break;

    }
}