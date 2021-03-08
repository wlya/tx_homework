#include <stdio.h>
#include <sys/socket.h> //For Sockets
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h> //For the AF_INET (Address Family)
#include <string.h>
#include "base.h"

#include "server.h"

#define INTERNAL_ERROR -1


#define DEBUG 1




int main(int argc, char const *argv[])
{
    if( argc != 3 ){
        loge("Please enter mode and port: [c/s] [port]\n");
        return INTERNAL_ERROR;
    }
    if( strcmp( argv[1], "c") == 0 ){
       //client mode
        logd("in client mode");
        CUR_MODE = P_STATE_CLIENT;
        LISTEN_PORT = atoi(argv[2]);

    }else{
        if(strcmp(argv[1], "s" ) == 0 ){
            //server mode
            logd("in server mode");
            CUR_MODE = P_STATE_SERVER;
            LISTEN_PORT = atoi(argv[2]);
            // server_main()

        }else{
            loge("Please enter mode and port: [c/s] [port]\n");
            return INTERNAL_ERROR;
        }
    }
    if (LISTEN_PORT < 1){
        loge("port error");
        return INTERNAL_ERROR;
    }
    logd("port is %d\n", LISTEN_PORT);
    
    return 0;
}
