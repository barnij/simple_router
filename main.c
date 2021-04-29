#include <stdio.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "entry.h"
#include "timer.h"
#include "udp_sender.h"
#include "ip_stuff.h"
#include "str_stuff.h"

struct entry V[1000];
struct entry directly[100];
uint32_t vsize;
uint32_t dsize;
const double ROUND_TIME = 4.0;
double timer;
const int DEFAULT_ACTIVITY = 4;

bool if_round() {
	if (getTime() - timer > ROUND_TIME) {
        timer = getTime();
        return true;
    }
    return false;
}

bool find_direct(uint32_t x) {
    for (uint32_t i=0; i<dsize; i++) {
        if (directly[i].netmask.sin_addr.s_addr == htonl(ntohl(x) & get_mask(directly[i].mask))) {
            V[i].activity = DEFAULT_ACTIVITY;
            return directly[i].via.sin_addr.s_addr == x;
        }
    }
    return false;
}

void vector_add_first(char *ip_str, char *mask_str, int dist) {
    int mask = atoi(mask_str);
    V[vsize].broadcast = get_broadcast(ip_str, mask);
    V[vsize].netmask = get_netmask(ip_str, mask);
    V[vsize].direct = true;
    V[vsize].dist = dist;
    V[vsize].mask = mask;
    V[vsize].via = get_ip(ip_str);
    V[vsize].activity = DEFAULT_ACTIVITY;
    vsize++;
}

void vector_add(char *netmask_str, struct sockaddr_in ip, uint8_t mask, uint32_t dist) {
    V[vsize].via = ip;
    V[vsize].broadcast = get_broadcast(netmask_str, mask);
    V[vsize].netmask = get_ip(netmask_str);
    V[vsize].direct = false;
    V[vsize].dist = dist;
    V[vsize].mask = mask;
    V[vsize].activity = DEFAULT_ACTIVITY;
    vsize++;
}

void print_vector() {
    fprintf(stdout, "\e[1;1H\e[2J");
    for (uint32_t i=0; i<vsize; i++) {
        char netmask[20];
        struct entry *e = &V[i];

        inet_ntop(AF_INET, &(e->netmask.sin_addr), netmask, sizeof(netmask));
        fprintf(stdout, "%s/%d ", netmask, e->mask);

        if (e->activity > 0 && e->dist < INFINITY){
            fprintf(stdout, "distance %d ",e->dist);
        } else {
            fprintf(stdout, "unreachable ");
        }

        if (e->direct) {
            fprintf(stdout, "connected directly");
        } else {
            char via[20];
            inet_ntop(AF_INET, &(e->via.sin_addr), via, sizeof(via));
            fprintf(stdout, "via %s", via);
        }

        fprintf(stdout, "\n");
    }
    fflush(stdout);
}


int find_entry_by_netmask(uint32_t x) {
    uint32_t w = INFINITY;
    uint32_t f = -1;
    for (uint32_t i=0; i<vsize; i++) {
        if (V[i].netmask.sin_addr.s_addr == x && V[i].dist <= w) {
            w = V[i].dist;
            f = i;
        }
    }

    return f;
}


int main(){
    timer = getTime();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}

    int broadcast_enable = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));
    if(ret) {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        close(sockfd);
		return EXIT_FAILURE;
    }

    
    struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(54321);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		fprintf(stderr, "bind error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
	}


    int n;
    scanf("%d", &n);
    for (int i=0; i<n; i++) {
        int dist;
        char temp_ip[20], ip[20], t[9], mask[3];

        scanf("%s %s %d", temp_ip, t, &dist);
        int32_t pos = strfind(temp_ip, '/');

        if(pos < 0){
            fprintf(stderr, "input error\n"); 
		    return EXIT_FAILURE;
        }
        
        substr(mask, temp_ip, pos+1, strlen(temp_ip)-pos-1);
        substr(ip, temp_ip, 0, pos);

        vector_add_first(ip, mask, dist);
        directly[dsize]=V[dsize];
        dsize++;
    }

    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;

    for(;;) {
        fd_set 	descriptors;
        FD_ZERO (&descriptors);
        FD_SET 	(sockfd, &descriptors);

        if(if_round()){
            print_vector();
            udp_sender(sockfd, V, directly, &vsize);
        }

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
		{
			fprintf(stderr, "select error: %s\n", strerror(errno)); 
			return EXIT_FAILURE;
		}

        tv.tv_sec = 4 - (getTime() - timer);
        tv.tv_sec = tv.tv_sec < 0 ? 0 : tv.tv_sec;

        if(ready == 0){
            tv.tv_sec = 4;
            tv.tv_usec = 0;
            continue;
        }

		struct sockaddr_in 	sender;	
		socklen_t 			sender_len = sizeof(sender);
		uint8_t 			buffer[IP_MAXPACKET];

		ssize_t datagram_len = recvfrom (
            sockfd,
            buffer,
            IP_MAXPACKET,
            MSG_DONTWAIT,
            (struct sockaddr*)&sender,
            &sender_len
        );

		if (datagram_len < 0) {
			fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

        

		char sender_ip_str[20];
		inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));
		// printf ("Received UDP packet from IP address: %s, port: %d\n", sender_ip_str, ntohs(sender.sin_port));

		buffer[datagram_len] = 0;
        struct in_addr rip;
        rip.s_addr = *((uint32_t *)buffer);
        uint8_t mask2 = *((uint8_t *)(buffer+4));
        uint32_t dist2 = ntohl(*((uint32_t *)(buffer+5)));
		char netmask2[20];
        inet_ntop(AF_INET, &(rip), netmask2, sizeof(netmask2));
        // printf("%s %d %d\n",netmask2,mask2,dist2);

        int32_t t1 = find_entry_by_netmask(rip.s_addr);
        bool d1 = find_direct(sender.sin_addr.s_addr);
        // printf("t1: %d, odl: %d, d1: %d\n",t1,V[t1].dist, d1);

        if(d1){
            continue;
        }

        if(t1 < 0){
            if(dist2 < INFINITY)
                vector_add(netmask2, sender, mask2, dist2);
        }else{
            if(sender.sin_addr.s_addr == V[t1].via.sin_addr.s_addr){
                if(dist2 != INFINITY)
                    V[t1].activity = DEFAULT_ACTIVITY;
                V[t1].dist = dist2;
            }else{
                if(dist2 < V[t1].dist){
                    V[t1].dist = dist2;
                    V[t1].via = sender;
                    V[t1].activity = DEFAULT_ACTIVITY;
                }
            }
        }

		fflush(stdout);
	}

    close (sockfd);

    return EXIT_SUCCESS;
}