#include "stdio.h"
#include "base.h"
#include <unistd.h>


VClient myself_info;

void init_client_info(){
    memset(&myself_info, 0, sizeof(myself_info));
    gethostname(myself_info.hostname, HOSTNAME_MAX_LEN);
    // logd("hostname is %s\n", myself_info.hostname);
    myself_info.port = LISTEN_PORT;
}


int client_main(int argc, char *argv[])
{
    init_client_info();
    dump_client_info(&myself_info);
    return 0;
}
