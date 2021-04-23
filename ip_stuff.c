#include <arpa/inet.h>
#include <netinet/ip.h>

uint32_t get_mask(int m){
    uint32_t t = 0;
    m = 32 - m;
    while(m--){
        t<<=1;
        t|=1;
    }
    return ~t;
}

struct sockaddr_in get_broadcast(char *s, int mask){
    struct sockaddr_in sa;
    inet_pton(AF_INET, s, &(sa.sin_addr));
    uint32_t broadcast = ntohl(sa.sin_addr.s_addr);
    uint32_t fullmask = get_mask(mask);
    broadcast |= (~fullmask);

    sa.sin_addr.s_addr = htonl(broadcast);
    return sa;
}

struct sockaddr_in get_ip(char *s){
    struct sockaddr_in sa;
    inet_pton(AF_INET, s, &(sa.sin_addr));
    return sa;
}

struct sockaddr_in get_netmask(char *s, int mask){
    struct sockaddr_in sa;
    inet_pton(AF_INET, s, &(sa.sin_addr));
    uint32_t netmask = ntohl(sa.sin_addr.s_addr);
    uint32_t fullmask = get_mask(mask);
    netmask &= fullmask;

    sa.sin_addr.s_addr = htonl(netmask);
    return sa;
}