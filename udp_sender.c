#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "entry.h"

void remove_entry(struct entry *v, uint32_t *vsize, ssize_t pos){
	for(uint32_t i=pos; i<*vsize; i++){
		v[i]=v[i+1];
	}
	(*vsize)--;
}

int udp_sender(int sockfd, struct entry *v, uint32_t *vsize)
{
	for (ssize_t i=0; i<*vsize; i++) {
		struct entry *e = &v[i];

		e->activity--;
		if(e->activity <= 0){
			if(e->activity < 5 && !e->direct){
				remove_entry(v, vsize, i);
				i--;
				continue;
			}

			e->connected=false;
			e->dist=INFINITY;
		}

		struct sockaddr_in server_address;
		bzero (&server_address, sizeof(server_address));
		server_address.sin_family      = AF_INET;
		server_address.sin_port        = htons(54321);
		server_address.sin_addr		   = e->broadcast.sin_addr;

		uint8_t buffer[9];
		*((struct in_addr *)buffer) = e->netmask.sin_addr;
		buffer[4] = e->mask;
		*((uint32_t *)(buffer+5)) = htonl(e->dist);
		ssize_t buffer_size = 9;

		if (sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size) {
			e->connected = false;
			e->dist = INFINITY;
			// fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
			// return EXIT_FAILURE;		
		}

	}

	return EXIT_SUCCESS;
}

