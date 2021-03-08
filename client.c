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


int as_client_fd;
char as_client_server_addr[IP_ADDR_MAX_LEN];
int as_client_server_port;

void func_LOGIN(char *str_in, char *str_out){
    logd("login str_in is: [%s]\n", str_in);
    int as_client_fd; 
    char* message = "Hello Server"; 
    struct sockaddr_in servaddr; 
    
    int n, len;
    int port = 0;
    int ret = sscanf(str_in, "%s %d", as_client_server_addr, &as_client_server_port);
    if (ret != 2 || port <= 0 || port > 65535 || is_valid_ip_addr(as_client_server_addr)==0 ){
        vlog("Invalid IP address/port error");
        return;
    }
    
    // Creating socket file descriptor 
    if ((as_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("socket creation failed"); 
        exit(0); 
    }
  
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(as_client_server_port); 
    servaddr.sin_addr.s_addr = inet_addr(as_client_server_addr); 
  
    if (connect(as_client_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) { 
        printf("\n Error : Connect Failed \n"); 
    } 
  
    memset(str_out, 0, sizeof(str_out)); 
    strcpy(str_out, "Hello Server"); 
    write(as_client_fd, str_out, sizeof(str_out)); 
    printf("Message from server: "); 
    read(as_client_fd, str_out, sizeof(str_out)); 
    puts(str_out); 
    close(as_client_fd); 
}

int client_main(int argc, char *argv[])
{
    init_client_info();
    dump_client_info(&myself_info);
    return 0;
}
