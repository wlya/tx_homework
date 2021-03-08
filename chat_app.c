#include <stdio.h>
#include <sys/socket.h> //For Sockets
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h> //For the AF_INET (Address Family)
#include <string.h>
#include "base.h"


#define INTERNAL_ERROR -1


#define DEBUG 1




VClient myself_info;
void init_client_info(){
    memset(&myself_info, 0, sizeof(myself_info));
    gethostname(myself_info.hostname, HOSTNAME_MAX_LEN);
    // logd("hostname is %s\n", myself_info.hostname);
    myself_info.port = LISTEN_PORT;
}



///////////////////////////////////////////////////////////////////////////////////////////////////


int as_client_fd;
char as_client_server_addr[IP_ADDR_MAX_LEN];
int as_client_server_port;

void func_client_LOGIN(char *str_in, char *str_out){
    logd("login str_in is: [%s]\n", str_in);
    int as_client_fd; 
    char* message = "Hello Server"; 
    struct sockaddr_in servaddr; 
    
    int n, len;
    int port = 0;
    int ret = sscanf(str_in, "%s %d", as_client_server_addr, &as_client_server_port);
    is_valid_ip_addr(as_client_server_addr);
    if (ret != 2 || as_client_server_port <= 0 || as_client_server_port > 65535 || is_valid_ip_addr(as_client_server_addr) == 0 ){
        vlog("Invalid IP address/port, ret = %d, error:%s:%d\n", ret, as_client_server_addr, as_client_server_port);
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
    write(as_client_fd, &myself_info, sizeof(myself_info));
    
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






void func111(char *str)
{
    printf("this is func  %s\n", str);
}

void str_to_upper(unsigned char *s)
{
    while(*s){
        *s = toupper(*(unsigned char*)s);
        s++;
    }
}
void func_client_AUTHOR(char* str_in, char* str_out){
    vlog("I, %s, have read and understood the course academic integrity policy.\n", "tanxi");
}
void func_client_IP(char* str_in, char* str_out){
    char myip[IP_ADDR_MAX_LEN];
    get_my_public_ip(myip);
    strncpy(&myself_info, myip, IP_ADDR_MAX_LEN);
    vlog("IP:%s\n", myip);
}
void func_client_PORT(char* str_in, char* str_out){
    vlog("PORT:%d\n", LISTEN_PORT);
}
void func_client_LIST(char* str_in, char* str_out){
    
    

    for(int i = 0; i<*(char*)str_out_head; i++){
        VClient* vclient = (VClient*)str_out_head+8;
        dump_client_info(vclient);
    }
}
void run_command(unsigned char *str){
    Command cmds[] = {
        {"AUTHOR",  COMMON, func_client_AUTHOR, "ID func is test ID"},
        {"IP",      COMMON, func_client_IP,    "LIST description is list users,...."},
        {"PORT",    COMMON, func_client_PORT,    "HELP print all help messages description is whicnn,...."},
        {"LIST",    COMMON, func_client_LIST,    "HELP print all help messages description is whicnn,...."},

        {"STATISTICS",  SERVER, func111,    ""},
        {"BLOCKED",     SERVER, func111,    ""},

        {"LOGIN",       CLIENT, func_client_LOGIN, "LOGIN 127.0.0.1 1234"},
        {"REFRESH",     CLIENT, func111, ""},
        {"SEND",        CLIENT, func111, ""},
        {"BROADCAST",   CLIENT, func111, ""},
        {"BLOCK",       CLIENT, func111, ""},
        {"UNBLOCK",     CLIENT, func111, ""},
        {"LOGOUT",      CLIENT, func111, ""},
        {"EXIT",        CLIENT, func111, ""},
    };
    
    // str_to_upper(str);
    for (int i = 0; i < sizeof(cmds) / sizeof(Command); i++)
    {
        if (strstr(str, cmds[i].cmd) == str){
            if( cmds[i].type == CUR_MODE || cmds[i].type == COMMON || DEBUG){
                char* buff_out = malloc(sizeof(VClient)*MAX_CLIENTS_LIMIT);
                logd("[ %s ]\n", cmds[i].description);
                cmds[i].func( str + strlen(cmds[i].cmd) + 1, buff_out);
                free(buff_out);
                break;
            }else{
                loge("You are in xxx mode, but run yyy's command");
            }
            
        }
    }
    return 0;
}


int test_shell_main(int argc, char const *argv[]){
    char buff[300];
    scanf("%s", buff);
    run_command(buff);
}



///////////////////////////////////////////////////////////////////////////////////////////////////


void func_server_login(char* str_in, char* str_out){
    
}

void func_server_list(int fd, char* str_in){
    int online_cout = 0;
    char* str_out_head = str_out;
    char* str_out_copy = str_out+8;
    for(int i = 0; i < MAX_CLIENTS_LIMIT; i++){
        if (server_clients[i].in_use == IN_USE){
            memcpy(str_out_copy+(sizeof(VClient)*online_cout), &server_clients[i], sizeof(VClient));
            online_cout++;
        }
    }
    *(char*)str_out_head = (char)online_cout;
    logd("count = %d\n", online_cout);
}

int server_main(int argc, char *argv[])
{
    memset(server_clients, 0, sizeof(server_clients));
    /* master file descriptor list */
    fd_set master;
    /* temp file descriptor list for select() */
    fd_set read_fds;
    /* server address */
    struct sockaddr_in serveraddr;
    /* client address */
    struct sockaddr_in clientaddr;
    /* maximum file descriptor number */
    int fdmax;
    /* listening socket descriptor */
    int listener;
    /* newly accept()ed socket descriptor */
    int newfd;
    /* buffer for client data */
    char buf[1024];
    int nbytes;
    /* for setsockopt() SO_REUSEADDR, below */
    int yes = 1;
    int addrlen;
    int i, j;
    /* clear the master and temp sets */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    /* get the listener */
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Server-socket() error lol!");
        /*just exit lol!*/
        exit(1);
    }
    printf("Server-socket() is OK...\n");
    /*"address already in use" error message */
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");
    /* bind */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(LISTEN_PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("Server-bind() error lol!");
        exit(1);
    }
    printf("Server-bind() is OK...\n");
    /* listen */
    if (listen(listener, 10) == -1)
    {
        perror("Server-listen() error lol!");
        exit(1);
    }
    printf("Server-listen() is OK...\n");
    /* add the listener to the master set */
    FD_SET(0, &master);
    FD_SET(listener, &master);
    /* keep track of the biggest file descriptor */
    fdmax = listener; /* so far, it's this one*/
    /* loop */
    for (;;)
    {
        /* copy it */
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("Server-select() error lol!");
            exit(1);
        }
        // logd("Server-select() is OK...\n");
        /*run through the existing connections looking for data to be read*/
        // vhit: stdin input handler
        if (FD_ISSET(0, &read_fds))
        {
            char cmd_buffer[512];
            nbytes = read(0, cmd_buffer, sizeof(cmd_buffer));
            cmd_buffer[nbytes-1] = '\0';
            logd("running cmd >>>[%s]\n", cmd_buffer);
            run_command(cmd_buffer);
            // if(nbytes>0){
            //     write(0, cmd_buffer, strnlen(cmd_buffer, 200));
            //     // printf("ffffffffff: %d, [%s]\n", nbytes, ss);
            // }
        }
        for (i = 1; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))
            { /* we got one... */
                if (i == listener)
                {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
                    {
                        perror("Server-accept() error lol!");
                    }
                    else
                    {
                        logd("IP address is: %s\n", inet_ntoa(clientaddr.sin_addr));
                        logd("port is: %d\n", (int) ntohs(clientaddr.sin_port));
                        logd("Server-accept() is OK...\n");

                        //vhit: init clinet info
                        server_clients[newfd].in_use = IN_USE;
                        server_clients[newfd].fd = newfd;
                        strncpy(server_clients[newfd].ip_addr, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        strncpy(server_clients[newfd].hostname, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        server_clients[newfd].num_msg_sent = 0;
                        server_clients[newfd].num_msg_rcv =  0;
                        
                        FD_SET(newfd, &master); /* add to master set */
                        
                        if (newfd > fdmax)
                        { /* keep track of the maximum */
                            fdmax = newfd;
                        }
                        
                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                }
                else
                {
                    /* handle data from a client */
                    if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
                    {
                        //vhit: erase logouted client msg
                        memset(&server_clients[i], 0 , sizeof(server_clients[i]));
                        /* got error or connection closed by client */
                        if (nbytes == 0)
                        {
                            /* connection closed */
                            printf("%s: socket %d hung up\n", argv[0], i);
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                        }
                        else
                        {
                            /* close it... */
                            perror("recv() error lol!");
                        }
                        close(i);
                        /* remove from master set */
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        /* we got some data from a client*/
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        logd("msg from:client %d, send>>>%s\n", i, buf);
                        // logd("msg from:%s, to:%s\n[msg]:%s\n", );
                        for (j = 1; j <= fdmax; j++)
                        {
                            /* send to everyone! */
                            if (FD_ISSET(j, &master))
                            {
                                /* except the listener and ourselves */
                                if (j != listener && j != i)
                                {
                                    if (send(j, buf, nbytes, 0) == -1)
                                        logd("send() error lol! j = %d ", j);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////



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
    if (LISTEN_PORT<1 || LISTEN_PORT > 65535){
        vlog("LISTEN PORT ERROR\n");
        return 1;
    }
    init_client_info();
    server_main(argc, argv);
    // switch (CUR_MODE)
    // {
    // case P_STATE_SERVER:
    //     server_main(argc, argv);
    //     break;
    // case P_STATE_CLIENT:
    //     client_main(argc, argv);
    // default:
    //     break;
    // }
    
    logd("port is %d\n", LISTEN_PORT);
    
    return 0;
}
