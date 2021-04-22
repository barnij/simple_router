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

struct entry V[1000];
uint32_t vsize;
double roundtime = 2.0;
double timer = 0.0;

uint32_t get_mask(int m){
    uint32_t t = 0;
    m = 32 - m;
    while(m--){
        t<<=1;
        t|=1;
    }
    return ~t;
}

bool ifround()
{
	if(getTime() - timer > roundtime){
        timer = getTime();
        return true;
    }
    return false;
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
}


void print_vector(){
    //fprintf(stdin, "\e[1;1H\e[2J");
    for(int i=0; i<vsize; i++){
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

int32_t strfind(char *s, char what){
    for(int i=0; s[i]; i++){
        if(s[i] == what)
            return i;
    }
    return -1;
}

void substr(char *sub, char *buff, int a, int n){
    bzero(sub,sizeof(sub));
    memcpy(sub, &buff[a], n);
    sub[n]='\0';
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

    fd_set 	descriptors;
	FD_ZERO (&descriptors);
	FD_SET 	(sockfd, &descriptors);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while(true) {

        if(ifround()){
            print_vector();
            udp_sender(sockfd, &server_address, V, vsize);
        }

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
		{
			fprintf(stderr, "select error: %s\n", strerror(errno)); 
			return EXIT_FAILURE;
		}

        if(ready == 0){
            continue;
        }

		struct sockaddr_in 	sender;	
		socklen_t 			sender_len = sizeof(sender);
		u_int8_t 			buffer[IP_MAXPACKET+1];

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
		printf ("%ld-byte message: +%s+\n", datagram_len, buffer);
		
		char* reply = "Thank you!";
		ssize_t reply_len = strlen(reply);
		if (sendto(sockfd, reply, reply_len, 0, (struct sockaddr*)&sender, sender_len) != reply_len) {
			fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
			return EXIT_FAILURE;
		}

		fflush(stdout);
	}

    close (sockfd);

    return EXIT_SUCCESS;
}