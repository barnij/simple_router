#include <stdint.h>
#include <string.h>

int32_t strfind(char *s, char what){
    for(int i=0; s[i]; i++){
        if(s[i] == what)
            return i;
    }
    return -1;
}

void substr(char *sub, char *buff, int a, int n){
    //bzero(sub,sizeof(sub));
    memcpy(sub, &buff[a], n);
    sub[n]='\0';
}
