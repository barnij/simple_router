#include <stdint.h>

uint32_t get_mask(int m);
struct sockaddr_in get_broadcast(char *s, int mask);
struct sockaddr_in get_ip(char *s);
struct sockaddr_in get_netmask(char *s, int mask);