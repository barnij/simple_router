#include <stdbool.h>

struct entry{
    struct sockaddr_in ip;
    struct sockaddr_in netmask;
    struct sockaddr_in broadcast;
    struct sockaddr_in via;
    bool direct;
    bool connected;
    int mask;
    int dist;
};