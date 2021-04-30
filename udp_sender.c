// Bartosz Jaśkiewicz, 307893
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "ip_stuff.h"

void remove_entry(struct entry *v, uint32_t *vsize, ssize_t pos) {
	for(uint32_t i=pos; i<*vsize; i++)
		v[i]=v[i+1];
	(*vsize)-=1;
}

uint32_t min(uint32_t a, uint32_t b){
	return a < b ? a : b;
}

int udp_sender(int sockfd, struct entry *v, struct entry *d, uint32_t *vsize) {
	for (uint32_t i=0; i<*vsize; i++) {
		struct entry *e = &v[i];

		e->activity--;
		if (e->activity <= 0) {
			if (e->activity < -5 && !e->direct) {
				remove_entry(v, vsize, i);
				i--;
				continue;
			}

			e->dist=INFINITY;
			for(uint32_t k=0; k<*vsize; k++) {
				if(e->netmask.sin_addr.s_addr == htonl(ntohl(v[k].via.sin_addr.s_addr) & get_mask(e->mask)))
					v[k].dist = INFINITY;
			}
		}

		if(!e->direct)
			continue;

		for(uint32_t j=0; j<*vsize; j++){

			struct entry *e1 = &v[j];

			struct sockaddr_in server_address;
			bzero (&server_address, sizeof(server_address));
			server_address.sin_family      = AF_INET;
			server_address.sin_port        = htons(54321);
			server_address.sin_addr		   = e->broadcast.sin_addr;

			uint8_t buffer[9];
			*((struct in_addr *)buffer) = e1->netmask.sin_addr;
			buffer[4] 					= e1->mask;
			*((uint32_t *)(buffer+5)) 	= htonl(min(INFINITY, e1->dist + e->dist));
			ssize_t buffer_size 		= 9;

			if (sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size) {
				e->dist = INFINITY;
				for(uint32_t k=0; k<*vsize; k++) {
					if(e->netmask.sin_addr.s_addr == htonl(ntohl(v[k].via.sin_addr.s_addr) & get_mask(e->mask)))
						v[k].dist = INFINITY;
				}
			} else {

				e->dist = d[i].dist;
				e->via = d[i].via;
			}
		}
	}

	return EXIT_SUCCESS;
}

