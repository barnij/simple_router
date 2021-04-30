// Bartosz Jaśkiewicz, 307893
#include <stdint.h>
#include <stdbool.h>
#define INFINITY 20

uint32_t get_mask(int m);
struct sockaddr_in get_broadcast(char *s, int mask);
struct sockaddr_in get_ip(char *s);
struct sockaddr_in get_netmask(char *s, int mask);

struct entry{
    struct sockaddr_in netmask;
    struct sockaddr_in broadcast;
    struct sockaddr_in via;
    uint32_t dist;
    int activity;
    uint8_t mask;
    bool direct;
};