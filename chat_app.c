#include <netinet/in.h>  //For the AF_INET (Address Family)
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  //For Sockets
#include <unistd.h>

#include "base.h"

#define INTERNAL_ERROR -1

#define DEBUG 1

VClient myself_info;
void init_client_info() {
    memset(&myself_info, 0, sizeof(myself_info));
    gethostname(myself_info.hostname, HOSTNAME_MAX_LEN);
    // logd("hostname is %s\n", myself_info.hostname);
    myself_info.port = LISTEN_PORT;
    memset(&server_clients, 0, sizeof(server_clients));
    memset(&client_clients, 0, sizeof(client_clients));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int as_client_server_fd;
char as_client_server_addr[IP_ADDR_MAX_LEN];
int as_client_server_port;

int is_in_local_list_cache(char *ip) {
    for (int i = 0; i < client_clients_count; i++) {
        if (0 == strncmp(ip, client_clients[i].ip_addr, IP_ADDR_MAX_LEN)) {
            logd("ip in cache ok: %s\n", ip);
            return 1;
        }
    }
    logd("ip in cache not !!!!!: %s\n", ip);

    return 0;
}
int get_id_by_ip(char *ip) {
    VClient *tclient = NULL;
    for (int i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        if (CUR_MODE == MODE_SERVER) {
            tclient = &server_clients[i];
        } else {
            tclient = &client_clients[i];
        }
        if (strncmp(tclient->ip_addr, ip, IP_ADDR_MAX_LEN) == 0) {
            return tclient->fd;
        }
    }
    return INTERNAL_ERROR;
}
VClient *get_vclient_by_ip(char *ip) {
    VClient *tclient = NULL;
    for (int i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        if (CUR_MODE == MODE_SERVER) {
            tclient = &server_clients[i];
        } else {
            tclient = &client_clients[i];
        }
        if (strncmp(tclient->ip_addr, ip, IP_ADDR_MAX_LEN) == 0) {
            return tclient;
        }
    }
    return NULL;
}

int send_msg_to_ip(char *ip, MSG_TRANSFER *msg, char *msg_out, size_t *msg_out_len) {
    int client_sockfd, len;
    struct sockaddr_in remote_addr;                //服务器端网络地址结构体
    char buf[32];                                  //数据传送的缓冲区
    memset(&remote_addr, 0, sizeof(remote_addr));  //数据初始化--清零
    remote_addr.sin_family = AF_INET;              //设置为IP通信
    remote_addr.sin_addr.s_addr = inet_addr(ip);   //服务器IP地址
    VClient *tclient = get_vclient_by_ip(ip);
    if (ip == as_client_server_addr) {
        remote_addr.sin_port = as_client_server_port;
    } else {
        if (tclient == NULL) {
            vlog_error("IP is not found.");
            return INTERNAL_ERROR;
        }
        remote_addr.sin_port = tclient->port;  //服务器端口号
    }
    logd("send msg to [%s:%d]\n", ip, remote_addr.sin_port);
    
    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        vlog_error("Socket error");
        return 1;
    }
    /*将套接字绑定到服务器的网络地址上*/
    if (connect(client_sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0) {
        perror("connect error");
        return 1;
    }
    logd("connected to server\n");
    int ret = vsend(client_sockfd, msg, sizeof(MSG_TRANSFER), 0);
    if (msg_out) {
        *msg_out_len = recv(client_sockfd, msg_out, sizeof(MSG_TRANSFER), 0);  //接收服务器端信息
    }
    close(client_sockfd);
    return ret;
}

void func_client_LOGIN(char *str_in, char *str_out) {
    if(as_client_server_fd >0){
        vlog_error("already login\n");
        return;
    }
    logd("login str_in is: [%s]\n", str_in);
    char *message = "Hello Server";
    struct sockaddr_in servaddr;
    logd("ready to 1111\n");

    int n, len;
    int port = 0;
    int ret = sscanf(str_in, "%s %d", as_client_server_addr, &as_client_server_port);
    logd("ready to 2222\n");
    if (ret != 2 || as_client_server_port <= 0 || as_client_server_port > 65535 || is_valid_ip_addr(as_client_server_addr) == 0) {
        vlog("Invalid IP address/port, ret = %d, error:%s:%d\n", ret, as_client_server_addr, as_client_server_port);
        return;
    }
    logd("ready to 3333\n");

    // Creating socket file descriptor
    if ((as_client_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(as_client_server_port);
    servaddr.sin_addr.s_addr = inet_addr(as_client_server_addr);
    logd("ready to connect[%s][%d]\n", as_client_server_addr, as_client_server_port);
    if (connect(as_client_server_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return INTERNAL_ERROR;
    }
    logd("connected!!!\n");
    
    MSG_TRANSFER tMSG_TRANSFER;
    memset(&tMSG_TRANSFER, 0, sizeof(MSG_TRANSFER));
    tMSG_TRANSFER.cmd = LOGIN;
    dump_client_info(&myself_info);
    memcpy(tMSG_TRANSFER.msg, &myself_info, sizeof(myself_info));
    logd("ready to vsend\n");
    vsend(as_client_server_fd, &tMSG_TRANSFER, sizeof(tMSG_TRANSFER), 0);
    logd("vsend!!!\n");
    // recv(i, buf, sizeof(buf), 0))
    // write(as_client_server_fd, &myself_info, sizeof(myself_info));

    printf("Message from server: ");
    vrecv(as_client_server_fd, str_out, sizeof(VClient) * MAX_CLIENTS_LIMIT, 0);

    // close(as_client_server_fd);
}
void func_server_login(int fd, char *str_in, size_t size) {
    // uint32_t count = *(uint32_t*)str_in;
    // count = min(count, MAX_CLIENTS_LIMIT);
    // dump_client_info()
    if(server_clients[fd].status != LOGGED_IN){
        server_clients_count++;
        server_clients[fd].status = LOGGED_IN;
    }
    strncpy(server_clients[fd].hostname, ((VClient *)str_in)->hostname, HOSTNAME_MAX_LEN);
    server_clients[fd].port = ((VClient *)str_in)->port;
    dump_client_info(&server_clients[fd]);
    func_server_list(fd, NULL, 0);
}

void func_client_AUTHOR(char *str_in, char *str_out) {
    vlog("I, %s, have read and understood the course academic integrity policy.\n", "tanxi");
}
void func_client_IP(char *str_in, char *str_out) {
    char myip[IP_ADDR_MAX_LEN];
    get_my_public_ip(myip);
    strncpy(&myself_info, myip, IP_ADDR_MAX_LEN);
    vlog("IP:%s\n", myip);
}
void func_client_PORT(char *str_in, char *str_out) {
    vlog("PORT:%d\n", LISTEN_PORT);
}

void func_client_LIST(char *str_in, char *str_out) {
    for (int i = 0; i < client_clients; i++) {
        if (client_clients[i].status == LOGGED_IN)
            dump_client_info(&client_clients[i]);
    }
}
void func_client_list_init() {
}

void func_server_list(int fd, char *str_in, size_t size) {
    int online_cout = 0;
    char *str_out = malloc(sizeof(VClient) * MAX_CLIENTS_LIMIT + 8);
    char *str_out_head = str_out;
    char *str_out_copy = str_out + 8;
    for (int i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        if (server_clients[i].status == LOGGED_IN) {
            memcpy(str_out_copy + (sizeof(VClient) * online_cout), &server_clients[i], sizeof(VClient));
            online_cout++;
        }
    }
    *(char *)str_out_head = (char)online_cout;
    server_clients_count = online_cout;
    vsend(fd, str_out, online_cout * sizeof(VClient) + 8, 0);
    free(str_out);
    logd("count = %d\n", online_cout);
}

void func_client_send(char *str_in, char *str_out) {
    //SEND <client-ip> <msg>
    char *msg = strchr(str_in, ' ');
    char ip[IP_ADDR_MAX_LEN];
    memset(ip, 0, IP_ADDR_MAX_LEN);
    strncpy(ip, str_in, msg - str_in);
    if ( is_valid_ip_addr(ip) && is_in_local_list_cache(ip)) {
        MSG_TRANSFER tmsg_transfer;
        memset(&tmsg_transfer, 0, sizeof(MSG_TRANSFER));
        tmsg_transfer.cmd = SEND;
        strncpy(tmsg_transfer.dst_ip, ip, IP_ADD_MEMBERSHIP);
        strncpy(tmsg_transfer.msg, msg + 1, MAX_MSG_LEN);
        vsend(as_client_server_fd, &tmsg_transfer, sizeof(tmsg_transfer), 0);
    }
}
void func_client_recv(MSG_TRANSFER *tmsg_transfer) {
}
void func_server_send(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    int dst_id = get_id_by_ip(tmsg_transfer->dst_ip);
    if (dst_id > 0 && dst_id < server_clients_count) {
        char *src_ip = server_clients[dst_id].ip_addr;
        send_msg_to_ip(server_clients[dst_id].ip_addr, &tmsg_transfer, NULL, NULL);
        // vsend(server_clients[dst_id].fd, &tmsg_transfer, sizeof(MSG_TRANSFER), 0);
    }
}
void func_client_broadcast(char *str_in, char *str_out) {
    // size_t msg_len = strnlen(msg, MAX_MSG_LEN);
    // *(msg+msg_len) = '\0';
}

void func_client_refresh_handle_back(char *str_out) {
    uint32_t online_count = *(uint32_t *)str_out;
    client_clients_count = online_count;
    for (int i = 0; i < online_count; i++) {
        dump_client_info(str_out + 8 + sizeof(VClient) * i);
    }
}

void func_client_refresh(char *strin, char *strout) {
    MSG_TRANSFER msg_trasfer;
    memset(&msg_trasfer, 0, sizeof(MSG_TRANSFER));
    msg_trasfer.cmd = REFRESH;
    MSG_TRANSFER msg_back;
    memset(&msg_back, 0, sizeof(MSG_TRANSFER));
    size_t out_len = 0;
    vsend(as_client_server_fd, &msg_trasfer, sizeof(msg_trasfer), 0);
    size_t outlen = vrecv(as_client_server_fd, &msg_back, sizeof(MSG_TRANSFER), 0);
    if (out_len > 0) {
        func_client_refresh_handle_back(&msg_back);
    }
}
void func_server_refresh(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    logd("server: recv cmd >> refresh\n");
    char *msg = malloc(sizeof(VClient) * server_clients_count + 8);
    *(uint32_t *)msg = server_clients_count;
    char *msg_0 = msg + 8;
    logd("server_clients_count : %d\n", server_clients_count);
    for (int i = 1; i < 10; i++) {
        logd("fd = %d, [%d].fd = %d, status = %d\n", fd, i, server_clients[i].fd, server_clients[i].status );
        if (server_clients[i].fd != fd && server_clients[i].status != UN_SEE) {
            dump_client_info(&server_clients[i]);
            memcpy(msg_0, &server_clients[i], sizeof(VClient));
            msg_0 += sizeof(VClient);
        }
    }
    
    vsend(fd, msg, sizeof(VClient) * server_clients_count + 8, 0);
}
Command cmds[] = {
    {"AUTHOR", AUTHOR, COMMON, func_client_AUTHOR, func_client_AUTHOR, "ID func is test ID"},
    {"IP", IP, COMMON, func_client_IP, func_client_IP, "LIST description is list users,...."},
    {"PORT", PORT, COMMON, func_client_PORT, func_client_PORT, "HELP print all help messages description is whicnn,...."},
    {"LIST", LIST, COMMON, func_client_LIST, func_server_list, "HELP print all help messages description is whicnn,...."},

    {"STATISTICS", STATISTICS, SERVER, func111, func111, ""},
    {"BLOCKED", BLOCKED, SERVER, func111, func111, ""},

    {"LOGIN", LOGIN, CLIENT, func_client_LOGIN, func_server_login, "LOG IN 127.0.0.1 1234"},
    {"REFRESH", REFRESH, CLIENT, func_client_refresh, func_server_refresh, ""},
    {"SEND", SEND, CLIENT, func_client_send, func_server_send, ""},
    {"BROADCAST", BROADCAST, CLIENT, func111, func111, ""},
    {"BLOCK", BLOCK, CLIENT, func111, func111, ""},
    {"UNBLOCK", UNBLOCK, CLIENT, func111, func111, ""},
    {"LOGOUT", LOGOUT, CLIENT, func111, func111, ""},
    {"EXIT", EXIT, CLIENT, func111, func111, ""},
};




void func_client_on_recv_send(int fd, MSG_TRANSFER* msg){
    vlog_success("received message from %s\n", msg->src_ip);
    vlog_success("msg is : [%s]\n", msg->msg);
}

ClientOnRecvCmd tClientOnRecvCmds[] = {
    {"SEND", SEND, func_client_on_recv_send, "When client received a message relayed from the server."}
};

struct TEST_CMD {
    char* str;
    char* cmd;
};
char *test_cmd[] = {
    "LOGIN 192.168.42.134 11111",
    "SEND 192.168.42.134 HELLOWORLD"
};
void run_command(unsigned char *str) {
    // str_to_upper(str);
    if(strlen(str) <= 2){
        int select = atoi(str);
        if(select >=0 && select < sizeof(test_cmd)/sizeof(char*))
            str = test_cmd[select];
    }
    


    for (int i = 0; i < sizeof(cmds) / sizeof(Command); i++) {
        if (strstr(str, cmds[i].cmd) == str) {
            if (cmds[i].type == CUR_MODE || cmds[i].type == COMMON || DEBUG) {
                char *buff_out = malloc(sizeof(VClient) * MAX_CLIENTS_LIMIT);
                logd("[running cmd: [%s](%s) ]\n", cmds[i].cmd, str + strlen(cmds[i].cmd) + 1);
                cmds[i].client_func(str + strlen(cmds[i].cmd) + 1, buff_out);
                free(buff_out);
                break;
            } else {
                loge("You are in xxx mode, but run yyy's command");
            }
        }
    }
    return 0;
}

int test_shell_main(int argc, char const *argv[]) {
    char buff[300];
    scanf("%s", buff);
    run_command(buff);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int func_server_recv_msg_handler(int fd, char *str_in, size_t size) {
    MSG_TRANSFER *tMSG_TRANSFER = (MSG_TRANSFER *)str_in;
    if (tMSG_TRANSFER->cmd >= 0 && tMSG_TRANSFER->cmd < CMD_ID_MAX) {
        logd("server run cmd: [%d],[%d]\n",fd,tMSG_TRANSFER->cmd);
        cmds[tMSG_TRANSFER->cmd].server_func(fd, tMSG_TRANSFER->msg, size);
    }
}

int func_client_recv_msg_handler(int fd, char *str_in, size_t size) {
    MSG_TRANSFER *tmsg_transfer = (MSG_TRANSFER *)str_in;
    if (tmsg_transfer->cmd >= 0 && tmsg_transfer->cmd < CMD_ID_MAX) {
        tClientOnRecvCmds[tmsg_transfer->cmd].handler(fd, tmsg_transfer);
    }
}

int server_main(int argc, char *argv[]) {
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
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Server-socket() error lol!");
        /*just exit lol!*/
        exit(1);
    }
    printf("Server-socket() is OK...\n");
    /*"address already in use" error message */
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("Server-setsockopt() error lol!");
        exit(1);
    }
    printf("Server-setsockopt() is OK...\n");
    /* bind */
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(LISTEN_PORT);
    memset(&(serveraddr.sin_zero), '\0', 8);
    if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1) {
        perror("Server-bind() error lol!");
        exit(1);
    }
    printf("Server-bind() is OK...\n");
    /* listen */
    if (listen(listener, 10) == -1) {
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
    for (;;) {
        /* copy it */
        read_fds = master;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Server-select() error lol!");
            exit(1);
        }
        // logd("Server-select() is OK...\n");
        /*run through the existing connections looking for data to be read*/
        // vhit: stdin input handler
        if (FD_ISSET(0, &read_fds)) {
            char cmd_buffer[512];
            nbytes = read(0, cmd_buffer, sizeof(cmd_buffer));
            cmd_buffer[nbytes - 1] = '\0';
            logd("running cmd >>>[%s]\n", cmd_buffer);
            run_command(cmd_buffer);
            // if(nbytes>0){
            //     write(0, cmd_buffer, strnlen(cmd_buffer, 200));
            //     // printf("ffffffffff: %d, [%s]\n", nbytes, ss);
            // }
        }
        for (i = 1; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { /* we got one... */
                if (i == listener) {
                    /* handle new connections */
                    addrlen = sizeof(clientaddr);
                    if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1) {
                        perror("Server-accept() error lol!");
                    } else {
                        logd("Server-accept() ok -> fd: %d , IP/port is: %s:%d\n", newfd, inet_ntoa(clientaddr.sin_addr), (int)ntohs(clientaddr.sin_port));
                        // logd("port is: %d\n", (int)ntohs(clientaddr.sin_port));
                        // logd("Server-accept() is OK...\n");

                        //vhit: init clinet info
                        server_clients[newfd].status = LOGGED_IN;
                        server_clients[newfd].fd = newfd;
                        strncpy(server_clients[newfd].ip_addr, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        // strncpy(server_clients[newfd].hostname, inet_ntoa(clientaddr.sin_addr), IP_ADDR_MAX_LEN);
                        server_clients[newfd].num_msg_sent = 0;
                        server_clients[newfd].num_msg_rcv = 0;

                        FD_SET(newfd, &master); /* add to master set */

                        if (newfd > fdmax) { /* keep track of the maximum */
                            fdmax = newfd;
                        }

                        printf("%s: New connection from %s on socket %d\n", argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
                    }
                } else {
                    /* handle data from a client */
                    if ((nbytes = vrecv(i, buf, sizeof(buf), 0)) <= 0) {
                        //vhit: erase logouted client msg
                        memset(&server_clients[i], 0, sizeof(server_clients[i]));
                        /* got error or connection closed by client */
                        if (nbytes == 0) {
                            /* connection closed */
                            printf("%s: socket %d hung up\n", argv[0], i);
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                            ////////////////////////////////////////
                        } else {
                            /* close it... */
                            perror("recv() error lol!");
                        }
                        close(i);
                        /* remove from master set */
                        FD_CLR(i, &master);
                    } else {
                        /* we got some data from a client*/
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        ////////////////////////////////////////
                        if (CUR_MODE == MODE_SERVER) {
                            logd("server_mode, msg from:client 11 >> %d, send>>>%s\n", i, buf);
                            func_server_recv_msg_handler(i, buf, nbytes);
                        } else {
                            if (CUR_MODE == MODE_CLIENT) {
                            logd("msg from:client %d, send>>>%s\n", i, buf);
                                func_client_recv_msg_handler(i, buf, nbytes);
                            }
                        }

                        // logd("msg from:%s, to:%s\n[msg]:%s\n", );
                        // for (j = 1; j <= fdmax; j++)
                        // {
                        //     /* send to everyone! */
                        //     if (FD_ISSET(j, &master))
                        //     {
                        //         /* except the listener and ourselves */
                        //         if (j != listener && j != i)
                        //         {
                        //             if (vsend(j, buf, nbytes, 0) == -1)
                        //                 logd("send() error lol! j = %d ", j);
                        //         }
                        //     }
                        // }
                    }
                }
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        loge("Please enter mode and port: [c/s] [port]\n");
        return INTERNAL_ERROR;
    }
    if (strcmp(argv[1], "c") == 0) {
        //client mode
        logd("in client mode");
        CUR_MODE = MODE_CLIENT;
        LISTEN_PORT = atoi(argv[2]);
    } else {
        if (strcmp(argv[1], "s") == 0) {
            //server mode
            logd("in server mode");
            CUR_MODE = MODE_SERVER;
            LISTEN_PORT = atoi(argv[2]);
            // server_main()
        } else {
            loge("Please enter mode and port: [c/s] [port]\n");
            return INTERNAL_ERROR;
        }
    }
    if (LISTEN_PORT < 1) {
        loge("port error");
        return INTERNAL_ERROR;
    }
    if (LISTEN_PORT < 1 || LISTEN_PORT > 65535) {
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
