#include <iostream>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <cstring>
#include <utility>
using namespace std;

struct entry{
    string ip;
    string netmask;
    string via;
    bool direct;
    bool connected;
    int mask;
    int dist;
};

uint32_t get_mask(int m){
    uint32_t t = 0;
    m = 32 - m;
    while(m--){
        t<<=1;
        t|=1;
    }
    return ~t;
}

string get_netmask(string &s, int mask){
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, s.c_str(), &(sa.sin_addr));
    uint32_t tempaddr = ntohl(sa.sin_addr.s_addr);
    tempaddr &= get_mask(mask);
    sa.sin_addr.s_addr = htonl(tempaddr);
    char tab[20];
    inet_ntop(AF_INET, &(sa.sin_addr), tab, sizeof(tab));
    string ret = tab;
    return ret;
}

void print_vector(vector<entry> &v){
    // cout<<"\e[1;1H\e[2J";
    for(entry e: v){
        cout<<e.ip<<"/"<<e.mask<<" ";
        if(e.connected){
            cout<<"distance "<<e.dist<<" ";
        }else{
            cout<<"unreachable ";
        }
        if(e.direct){
            cout<<"connected directly";
        }else{
            cout<<"via "<<e.via;
        }
        cout<<"\n";
    }
}

int main(){
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
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
    

    vector < entry > V; 

    int n;
    cin>>n;
    for(int i=0; i<n; i++){
        int mask, dist;
        string::size_type sz;
        string ip, t;

        cin>>ip>>t>>dist;
        size_t pos = ip.find('/');
        
        mask = stoi(ip.substr(pos+1), &sz);
        ip = ip.substr(0, pos);
        V.push_back({ip, get_netmask(ip, mask), "", true, true, mask,dist});
    }

    fd_set 	descriptors;
	FD_ZERO (&descriptors);
	FD_SET 	(sockfd, &descriptors);
	struct 	timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

    for (;;) {

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0)
		{
			fprintf(stderr, "select error: %s\n", strerror(errno)); 
			return EXIT_FAILURE;
		}

        if(! FD_ISSET(sockfd, &descriptors)){
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