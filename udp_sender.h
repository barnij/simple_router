#include <stdint.h>

int udp_sender(int sockfd, struct sockaddr_in *server_adress, struct entry *v, uint32_t *vsize);