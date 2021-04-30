// Bartosz Jaśkiewicz, 307893
#include <stdint.h>
#include <string.h>
#include <time.h>

int32_t strfind(char *s, char what){
    for(int i=0; s[i]; i++){
        if(s[i] == what)
            return i;
    }
    return -1;
}

void substr(char *sub, char *buff, int a, int n){
    memcpy(sub, &buff[a], n);
    sub[n]='\0';
}

double get_time()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec + now.tv_nsec*1e-9;
}