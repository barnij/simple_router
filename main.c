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
uint32_t vsize;
const double roundtime = 2.0;
double timer = 0.0;
const int default_activity = 4;

bool ifround()
{
	if(getTime() - timer > roundtime){
        timer = getTime();
        return true;
    }
    return false;
}

void vector_add_special(char *ip_str, char *mask_str, int dist){
    uint32_t x = vsize;
    vsize++;
    int mask = atoi(mask_str);
    V[x].ip = get_ip(ip_str);
    V[x].broadcast = get_broadcast(ip_str, mask);
    V[x].netmask = get_netmask(ip_str, mask);
    V[x].connected = true;
    V[x].direct = true;
    V[x].dist = dist;
    V[x].mask = mask;
    V[x].activity = default_activity;
}

void vector_add(char *ip_str, uint8_t mask, uint32_t dist){
    uint32_t x = vsize;
    vsize++;
    V[x].ip = get_ip(ip_str);
    V[x].broadcast = get_broadcast(ip_str, mask);
    V[x].netmask = get_netmask(ip_str, mask);
    V[x].connected = true;
    V[x].direct = false;
    V[x].dist = dist;
    V[x].mask = mask;
    V[x].activity = default_activity;
}

void print_vector(){
    fprintf(stdin, "\e[1;1H\e[2J");
    for(uint32_t i=0; i<vsize; i++){
        char netmask[20];
        struct entry *e = &V[i];
        inet_ntop(AF_INET, &(e->netmask.sin_addr), netmask, sizeof(netmask));
        fprintf(stdout, "%s/%d ", netmask, e->mask);
        if(e->connected){
            fprintf(stdout, "distance %d ",e->dist);
        }else{
            fprintf(stdout, "unreachable ");
        }
        if(e->direct){
            fprintf(stdout, "connected directly");
        }else{
            char via[20];
            inet_ntop(AF_INET, &(e->via.sin_addr), via, sizeof(via));
            fprintf(stdout, "via %s", via);
        }
        fprintf(stdout, "\n");
    }
    fflush(stdout);
}

ssize_t find_entry_by_ip(uint32_t x){
    for(uint32_t i=0; i<vsize; i++){
        if(V[i].ip.sin_addr.s_addr == x){
            return i;
        }
    }
    return -1;
}

int find_entry_by_netmask(uint32_t x){
    for(uint32_t i=0; i<vsize; i++){
        if(V[i].netmask.sin_addr.s_addr == x){
            return i;
        }
    }
    return -1;
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
    if(ret){
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
    for(int i=0; i<n; i++){
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

        vector_add_special(ip, mask, dist);
    }

    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;

    for(;;) {
        fd_set 	descriptors;
        FD_ZERO (&descriptors);
        FD_SET 	(sockfd, &descriptors);

        if(ifround()){
            print_vector();
            udp_sender(sockfd, V, &vsize);
        }

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
		{
			fprintf(stderr, "select error: %s\n", strerror(errno)); 
			return EXIT_FAILURE;
		}

        tv.tv_sec = 4 - (getTime()- timer);

        if(ready == 0){
            tv.tv_sec = 4;
            tv.tv_usec = 0;
            continue;
        }

		struct sockaddr_in 	sender;	
		socklen_t 			sender_len = sizeof(sender);
		uint8_t 			buffer[IP_MAXPACKET+1];

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
		printf ("Received UDP packet from IP address: %s, port: %d\n", sender_ip_str, ntohs(sender.sin_port));

		buffer[datagram_len] = 0;
        struct in_addr rip;
        rip.s_addr = ntohl(*((uint32_t *)buffer));
        uint8_t mask2 = *((uint8_t *)(buffer+4));
        uint32_t dist2 = ntohl(*((uint32_t *)(buffer+5)));
		char ip2[20];
        inet_ntop(AF_INET, &(rip), ip2, sizeof(ip2));
        printf("%s\n",ip2);

        int t1 = find_entry_by_netmask(rip.s_addr);
        printf("t1: %d\n",t1);
        V[t1].activity = default_activity;

        if(t1 < 0){
            printf("HALO\n");
            vector_add(sender_ip_str, mask2, dist2);
        }else{
            if(sender.sin_addr.s_addr == V[t1].via.sin_addr.s_addr){
                V[t1].dist = dist2;
            }else{
                if(dist2 < V[t1].dist){
                    V[t1].dist = dist2;
                    V[t1].via = sender;
                }
            }
        }

		fflush(stdout);
	}

    close (sockfd);

    return EXIT_SUCCESS;
}