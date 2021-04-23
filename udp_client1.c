#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "entry.h"

uint32_t get_mask(int m){
    uint32_t t = 0;
    m = 32 - m;
    while(m--){
        t<<=1;
        t|=1;
    }
    return ~t;
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



int main()
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(54321);
	inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

	struct entry e;
    int mask = atoi("18");
	e.ip = get_ip("255.255.255.255");
    e.broadcast = get_broadcast("255.255.255.255", mask);
    e.netmask = get_netmask("255.255.255.255", mask);
    e.connected = true;
    e.direct = true;
    e.dist = 1345;
    e.mask = mask;
	
	uint8_t buffer[9];
	*((struct in_addr *)buffer) = e.netmask.sin_addr;
	buffer[4] = e.mask;
	*((uint32_t *)(buffer+5)) = htonl(e.dist);
	ssize_t buffer_size = 9;
	if (sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size) {
		fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;		
	}

	close (sockfd);
	return EXIT_SUCCESS;
}
	
