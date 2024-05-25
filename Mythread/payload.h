#pragma once
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h>

class Payload {

    public:
        char* buf;
        int len;
        struct sockaddr_in addr;
        Payload(int, char*, struct sockaddr_in);
        ~Payload(){
            free(buf);
        }
};