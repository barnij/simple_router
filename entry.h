#include <stdbool.h>
#include <stdint.h>
#define INFINITY UINT32_MAX

struct entry{
    struct sockaddr_in ip;
    struct sockaddr_in netmask;
    struct sockaddr_in broadcast;
    struct sockaddr_in via;
    bool direct;
    bool connected;
    uint8_t mask;
    uint32_t dist;
    int activity;
};