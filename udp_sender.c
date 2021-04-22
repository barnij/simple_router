#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "entry.h"

int udp_sender(int sockfd, struct sockaddr_in *server_adress, struct entry *v, uint32_t *vsize)
{
	for (int i=0; i<*vsize; i++) {
		struct entry *e = &v[i];

		e->activity--;
		if(e->activity <= 0){
			e->connected=false;
			e->dist=INFINITY;
		}

		struct sockaddr_in server_address;
		bzero (&server_address, sizeof(server_address));
		server_address.sin_family      = AF_INET;
		server_address.sin_port        = htons(54321);
		server_address.sin_addr		   = e->broadcast.sin_addr;

		uint8_t buffer[9];
		*((struct in_addr *)buffer) = e->ip.sin_addr;
		buffer[4] = e->mask;
		*((uint32_t *)(buffer+5)) = e->dist;
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

