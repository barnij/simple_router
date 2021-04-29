#include <stdbool.h>
#include <stdint.h>
#define INFINITY 20

struct entry{
    struct sockaddr_in netmask;
    struct sockaddr_in broadcast;
    struct sockaddr_in via;
    uint32_t dist;
    int activity;
    uint8_t mask;
    bool direct;
};