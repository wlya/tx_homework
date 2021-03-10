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

int is_has_clients(char *ip) {
    for (int i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        VClient *tclient = NULL;

        if (CUR_MODE == MODE_CLIENT) {
            tclient = &client_clients[i];
        } else {
            tclient = &server_clients[i];
        }

        if (strncmp(tclient->ip_addr, ip, IP_ADDR_MAX_LEN) == 0) {
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

int send_msg_to_ip(char *ip, int port, MSG_TRANSFER *msg, char *msg_out, size_t *msg_out_len) {
    int client_sockfd;
    struct sockaddr_in remote_addr;                //服务器端网络地址结构体
    char buf[32];                                  //数据传送的缓冲区
    memset(&remote_addr, 0, sizeof(remote_addr));  //数据初始化--清零
    remote_addr.sin_family = AF_INET;              //设置为IP通信
    remote_addr.sin_addr.s_addr = inet_addr(ip);   //服务器IP地址
    remote_addr.sin_port = htons(port);

    logd("send msg to [%s:%d]\n", ip, remote_addr.sin_port);

    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        vlog_error("Socket error");
        return 1;
    }
    /*将套接字绑定到服务器的网络地址上*/
    if (connect(client_sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0) {
        logd("inet_addr(ip):[%08x][%d],", inet_addr(ip), remote_addr.sin_port);
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

void func_shell_LOGIN(char *str_in, char *str_out) {
    if (as_client_server_fd > 0) {
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
    CUR_SHELL_STATE |= SHELL_STATE_CLIENT_LOGINED;

    printf("Message from server: ");
    vrecv(as_client_server_fd, str_out, sizeof(VClient) * MAX_CLIENTS_LIMIT, 0);
    func_shell_refresh_handle_back(str_out);
    // close(as_client_server_fd);
}
void func_server_login(int fd, MSG_TRANSFER *str_in, size_t size) {
    uint32_t count = *(uint32_t *)str_in;
    VClient *tclient = (VClient *)str_in->msg;
    // count = min(count, MAX_CLIENTS_LIMIT);
    // dump_client_info()
    if (server_clients[fd].status != LOGGED_IN) {
        server_clients_count++;
        server_clients[fd].status = LOGGED_IN;
    }
    strncpy(server_clients[fd].hostname, tclient->hostname, HOSTNAME_MAX_LEN);
    server_clients[fd].port = tclient->port;
    dump_client_info(&server_clients[fd]);
    func_server_list(fd, NULL, 0);
}

void func_shell_AUTHOR(char *str_in, char *str_out) {
    vlog("I, %s, have read and understood the course academic integrity policy.\n", "tanxi");
}
void func_shell_IP(char *str_in, char *str_out) {
    char myip[IP_ADDR_MAX_LEN];
    get_my_public_ip(myip);
    strncpy(&myself_info, myip, IP_ADDR_MAX_LEN);
    vlog("IP:%s\n", myip);
}
void func_shell_PORT(char *str_in, char *str_out) {
    vlog("PORT:%d\n", LISTEN_PORT);
}
int sort_vclient_port(VClient *t1, VClient *t2) {
    return t1->port > t2->port ? 1 : -1;
}
VClient *sort_clients() {
    VClient *resultArray = (VClient *)malloc(sizeof(VClient) * MAX_CLIENTS_LIMIT);
    VClient *tmpPtr = resultArray;
    VClient *data_src = NULL;
    if (CUR_MODE == MODE_SERVER) {
        data_src = server_clients;
    } else {
        data_src = client_clients;
    }
    size_t active_count = 0;
    for (size_t i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        if (data_src[i].port > 0) {
            memcpy(&tmpPtr[active_count], &data_src[i], sizeof(VClient));
            active_count++;
        }
    }
    if (CUR_MODE == MODE_SERVER) {
        server_clients_count = active_count;
    } else {
        // CUR_MODE == MODE_CLIENT
        client_clients_count = active_count;
    }

    // for(int i = 0;i < active_count; i++) printf("%d:%d \n", i, resultArray[i].port);
    // printf("----------\n");

    qsort(resultArray, active_count, sizeof(VClient), sort_vclient_port);
    // printf("----------\n");
    // for(int i = 0;i < active_count; i++) printf("%d:%d \n", i, resultArray[i].port);
    return resultArray;
}

void func_shell_statistics(char *strin, char *strout) {
    if (CUR_MODE == MODE_SERVER) {
        //statics
        VClient *sorted_clients_by_port = sort_clients();
        for (int i = 0; i < server_clients_count; i++) {
            if (sorted_clients_by_port[i].status != UN_SEE) {
                vlog_success("%-5d%-35s%-8d%-8d%-8s|%-35s:%d\n", i + 1, sorted_clients_by_port[i].hostname, sorted_clients_by_port[i].num_msg_sent, sorted_clients_by_port[i].num_msg_rcv, LOGIN_STATUS[sorted_clients_by_port[i].status], sorted_clients_by_port[i].ip_addr, sorted_clients_by_port[i].port);
                // vlog_success("%-5d%-35s%-8d%-8d%-8s\n", i + 1, sorted_clients_by_port[i].hostname, sorted_clients_by_port[i].num_msg_sent, sorted_clients_by_port[i].num_msg_rcv, LOGIN_STATUS[sorted_clients_by_port[i].status]);
            }
        }
        free(sorted_clients_by_port);
    }
}

void func_shell_list(char *str_in, char *str_out) {
    logd("running shell list, count=%d\n", client_clients_count);
    //list
    VClient *tarray = sort_clients();
    int total_count = 0;
    if (CUR_MODE == MODE_SERVER) {
        total_count = server_clients_count;
    } else {
        total_count = client_clients_count;
    }
    if (total_count == 0) {
        vlog_success("is empty.\n");
    }
    for (int i = 0; i < total_count; i++) {
        vlog_success("%-5d%-35s%-20s%-8d\n", i + 1, tarray[i].hostname, tarray[i].ip_addr, tarray[i].port);
    }
    free(tarray);
}

void func_server_list(int fd, MSG_TRANSFER *str_in, size_t size) {
    int online_cout = 0;
    size_t buf_size = sizeof(VClient) * MAX_CLIENTS_LIMIT + 8;
    char *str_out = malloc(buf_size);
    memset(str_out, buf_size, 0);
    char *str_out_head = str_out;
    char *str_out_copy = str_out + 8;
    for (int i = 0; i < MAX_CLIENTS_LIMIT; i++) {
        if (server_clients[i].status == LOGGED_IN && fd != server_clients[i].fd) {
            memcpy(str_out_copy + (sizeof(VClient) * online_cout), &server_clients[i], sizeof(VClient));
            online_cout++;
        }
    }
    *(uint32_t *)str_out_head = (uint32_t)online_cout;
    server_clients_count = online_cout;
    vsend(fd, str_out, online_cout * sizeof(VClient) + 8, 0);
    free(str_out);
    logd("count = %d\n", online_cout);
}

void func_shell_send(char *str_in, char *str_out) {
    //SEND <client-ip> <msg>
    char *msg = strchr(str_in, ' ');
    char ip[IP_ADDR_MAX_LEN];
    memset(ip, 0, IP_ADDR_MAX_LEN);
    strncpy(ip, str_in, msg - str_in);
    if (is_valid_ip_addr(ip) && is_has_clients(ip)) {
        MSG_TRANSFER tmsg_transfer;
        memset(&tmsg_transfer, 0, sizeof(MSG_TRANSFER));
        tmsg_transfer.cmd = SEND;
        strncpy(tmsg_transfer.dst_ip, ip, IP_ADD_MEMBERSHIP);
        strncpy(tmsg_transfer.msg, msg + 1, MAX_MSG_LEN);
        dump_msg_transfer(&tmsg_transfer);
        vsend(as_client_server_fd, &tmsg_transfer, sizeof(tmsg_transfer), 0);
        char msg_out[64];
        vrecv(as_client_server_fd, msg_out, 64, 0);
    }
}

int is_blocked_by_client(int src_fd, VClient *dst_client) {
    uint64_t tmp = ((uint64_t)0x1) << dst_client->fd;
    if (server_clients[src_fd].block_list & tmp) {
        return 1;
    }
    return 0;
}
int is_blocked_by_id(int src_fd, int dst_fd) {
    return is_blocked_by_client(src_fd, &server_clients[dst_fd]);
}

int is_1_blocked_by_2_ptr(VClient *ptr1, VClient *ptr2) {
    uint64_t tmp = ((uint64_t)0x1) << ptr1->fd;
    if (ptr2->block_list & tmp) {
        return 1;
    }
    return 0;
}
int is_1_blocked_by_2(int fd1, int fd2) {
    return is_1_blocked_by_2_ptr(&server_clients[fd1], &server_clients[fd2]);
}

void func_server_send(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    VClient *dst_client = get_vclient_by_ip(tmsg_transfer->dst_ip);
    strncpy(tmsg_transfer->src_ip, server_clients[fd].ip_addr, IP_ADDR_MAX_LEN);
    logd("fd:%d\n", fd);
    dump_client_info(dst_client);
    if (is_1_blocked_by_2(fd, dst_client->fd)) {
        logd("blocking msg: src:[%d] to [%s]msg\n", fd, dst_client->ip_addr);
    } else {
        if (dst_client && dst_client->status != UN_SEE) {
            char *src_ip = dst_client->ip_addr;
            dump_msg_transfer(tmsg_transfer);
            send_msg_to_ip(dst_client->ip_addr, dst_client->port, tmsg_transfer, NULL, NULL);
        } else {
            logd("remote device is not online");
            vsend(fd, "Target not online\n", 38, 0);
            return;
        }
    }
    vsend(fd, "send success!", 14, 0);
}

void func_shell_refresh_handle_back(char *str_out) {
    uint32_t online_count = *(uint32_t *)str_out;
    logd("func_shell_refresh_handle_back client_clients_count = %d\n", online_count);
    client_clients_count = online_count;
    memset(client_clients, 0, sizeof(VClient) * MAX_CLIENTS_LIMIT);
    for (int i = 0; i < online_count; i++) {
        memcpy(&client_clients[i], str_out + 8 + sizeof(VClient) * i, sizeof(VClient));
        // dump_client_info(str_out + 8 + sizeof(VClient) * i);
    }
}

void func_shell_refresh(char *strin, char *strout) {
    MSG_TRANSFER msg_trasfer;
    memset(&msg_trasfer, 0, sizeof(MSG_TRANSFER));
    msg_trasfer.cmd = REFRESH;
    MSG_TRANSFER msg_back;
    memset(&msg_back, 0, sizeof(MSG_TRANSFER));
    vsend(as_client_server_fd, &msg_trasfer, sizeof(msg_trasfer), 0);
    size_t out_len = vrecv(as_client_server_fd, &msg_back, sizeof(MSG_TRANSFER), 0);
    logd("ourlen is [%d]\n", out_len);
    if (out_len >= 0) {
        func_shell_refresh_handle_back(&msg_back);
    }
}

void func_server_refresh(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    logd("server: recv cmd >> refresh\n");
    size_t buf_size = sizeof(VClient) * server_clients_count + 8;
    char *msg = malloc(buf_size);
    memset(msg, 0, buf_size);
    char *msg_0 = msg + 8;
    logd("server_clients_count : %d\n", server_clients_count);
    hexDump("", msg, buf_size);
    int tmp_count = 0;
    for (int i = 1; i < MAX_CLIENTS_LIMIT; i++) {
        if (server_clients[i].status != UN_SEE) {
            logd("fd = %d, [%d].fd = %d, status = %d\n", fd, i, server_clients[i].fd, server_clients[i].status);
            dump_client_info(&server_clients[i]);
        }
        if (server_clients[i].fd != fd && server_clients[i].status != UN_SEE) {
            tmp_count++;
            memcpy(msg_0, &server_clients[i], sizeof(VClient));
            hexDump("", msg, buf_size);
            msg_0 += sizeof(VClient);
        }
    }
    server_clients_count = tmp_count + 1;
    *(uint32_t *)msg = tmp_count;
    vsend(fd, msg, buf_size, 0);
    free(msg);
}

void func_base_shell_cmd_to_server(int cmd, char *str_in, size_t str_in_size, char *str_out, size_t *str_out_size) {
    MSG_TRANSFER msg_to, msg_back;
    memset(&msg_to, 0, sizeof(MSG_TRANSFER));
    memset(&msg_back, 0, sizeof(MSG_TRANSFER));
    msg_to.cmd = cmd;
    if (str_in) {
        logd("ready to send: %s\n", str_in);
        strncpy(msg_to.msg, str_in, min(MAX_MSG_LEN, str_in_size));
    }
    vsend(as_client_server_fd, &msg_to, sizeof(MSG_TRANSFER), 0);
    if (str_out) {
        *str_out_size = vrecv(as_client_server_fd, str_out, min(sizeof(MSG_TRANSFER), *str_out_size), 0);
        vlog_success("%s\n", str_out);
    }
}
void func_shell_logout(char *strin, char *strout) {
    char msg_back[MAX_MSG_LEN];
    size_t msg_back_len = MAX_MSG_LEN;
    func_base_shell_cmd_to_server(LOGOUT, strin, strnlen(strin, MAX_MSG_LEN), msg_back, &msg_back_len);
    close(as_client_server_fd);
    CUR_SHELL_STATE &= SHELL_STATE_CLIENT_LOGOUTED;
    as_client_server_fd = INTERNAL_ERROR;
}

void func_server_logout(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    if (fd > 0 && fd < MAX_CLIENTS_LIMIT) {
        server_clients[fd].status = LOGGED_OUT;
        vsend(fd, "logout success", 15, 0);
    }
}
/*
void func_shell_DEMO(char* strin, char* strout){
    char msg_back[MAX_MSG_LEN];
    size_t msg_back_len;
    func_base_shell_cmd_to_server(LOGOUT, strin, strnlen(strin, MAX_MSG_LEN), msg_back, &msg_back_len);
    close(as_client_server_fd);
    as_client_server_fd = INTERNAL_ERROR;
}

void func_server_DEMO(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    if(fd >0 && fd< MAX_CLIENTS_LIMIT){
        server_clients[fd].status = LOGGED_OUT;
        vsend(fd, "logout success", 15, 0);
    }
}

*/

void func_shell_block(char *strin, char *strout) {
    char msg_back[MAX_MSG_LEN];
    char *ip = strin;
    if (is_valid_ip_addr(ip) && is_has_clients(ip)) {
        size_t msg_back_len = MAX_MSG_LEN;
        func_base_shell_cmd_to_server(BLOCK, strin, strnlen(strin, IP_ADDR_MAX_LEN), msg_back, &msg_back_len);
        logd("%s\n", msg_back);
    } else {
        logd("IP [%s] is invalid or not exist.\n", ip);
    }
}
void func_shell_unblock(char *strin, char *strout) {
    char msg_back[MAX_MSG_LEN];
    char *ip = strin;
    if (is_valid_ip_addr(ip) && is_has_clients(ip)) {
        size_t msg_back_len = MAX_MSG_LEN;
        func_base_shell_cmd_to_server(UNBLOCK, strin, strnlen(strin, IP_ADDR_MAX_LEN), msg_back, &msg_back_len);
        logd("%s\n", msg_back);
    } else {
        logd("IP [%s] is invalid or not exist.\n", ip);
    }
}

void func_server_block(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    if (fd > 0 && fd < MAX_CLIENTS_LIMIT) {
        char *ip = tmsg_transfer->msg;
        if (is_valid_ip_addr(ip) && is_has_clients(ip)) {
            int id = get_id_by_ip(ip);
            logd("ip [%s] is blocked by:  [%d]\n", ip, fd);
            switch (tmsg_transfer->cmd) {
                case BLOCK:
                    server_clients[fd].block_list |= ((uint64_t)0x1) << id;
                    vsend(fd, "block success", 15, 0);
                    break;
                case UNBLOCK:
                    server_clients[fd].block_list &= ~(((uint64_t)0x1) << id);
                    vsend(fd, "unblock success", 15, 0);
                    break;
                default:
                    break;
            }
        }
    }
}

void func_shell_exit(char *strin, char *strout) {
    func_base_shell_cmd_to_server(EXIT, strin, strnlen(strin, MAX_MSG_LEN), NULL, NULL);
    close(as_client_server_fd);
    as_client_server_fd = INTERNAL_ERROR;
    vlog_success("exit");
    exit(0);
}

void func_server_exit(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    if (fd > 0 && fd < MAX_CLIENTS_LIMIT) {
        memset(&server_clients[fd], 0, sizeof(VClient));
    }
}

void func_shell_broadcast(char *strin, char *strout) {
    char msg_back[MAX_MSG_LEN];
    size_t msg_back_len = MAX_MSG_LEN;
    func_base_shell_cmd_to_server(BROADCAST, strin, strnlen(strin, MAX_MSG_LEN), msg_back, &msg_back_len);
}
void func_shell_broadcast_old(char *strin, char *strout) {
    MSG_TRANSFER msg_to, msg_back;
    memset(&msg_to, 0, sizeof(MSG_TRANSFER));
    memset(&msg_back, 0, sizeof(MSG_TRANSFER));
    msg_to.cmd = BROADCAST;
    logd("broadcast ready to send: %s\n", strin);
    strncpy(msg_to.msg, strin, MAX_MSG_LEN);
    vsend(as_client_server_fd, &msg_to, sizeof(MSG_TRANSFER), 0);
    size_t out_len = vrecv(as_client_server_fd, &msg_back, sizeof(MSG_TRANSFER), 0);
    logd("ourlen is [%d]\n", out_len);
}
void func_server_broadcast(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    MSG_TRANSFER msg_back;
    size_t size_bak;
    memset(&msg_back, 0, sizeof(MSG_TRANSFER));
    logd("broadcast gogogo . 1111\n");
    strncpy(tmsg_transfer->src_ip, server_clients[fd].ip_addr, IP_ADDR_MAX_LEN);
    for (int i = 1; i < MAX_CLIENTS_LIMIT; i++) {
        VClient *tclient = &(server_clients[i]);
        if (tclient == NULL || fd == tclient->fd || tclient->status != LOGGED_IN || is_1_blocked_by_2(fd, tclient->fd)) {
            // logd("skip send self, fd = %d\n", fd);
        } else {
            send_msg_to_ip(tclient->ip_addr, tclient->port, tmsg_transfer, &msg_back, &size_bak);
        }
    }
    vsend(fd, "Broadcast success!", 19, 0);
}

void func_shell_blocked(char *strin, char *strout) {
    if (CUR_MODE == MODE_SERVER) {
        int id = get_id_by_ip(strin);
        for (int i = 0; i < 64; i++) {
            if (server_clients[id].block_list & ((uint64_t)0x1) << i) {
                logd("is blocked by %d, %s\n", i, server_clients[i].ip_addr);
            }
        }
    }
}

void func_shell_on_recv_send(int fd, MSG_TRANSFER *msg) {
    vlog_success("received message from %s\n", msg->src_ip);
    vlog_success("msg is : [%s]\n", msg->msg);
    vsend(fd, "ok", 3, 0);
}

Command cmds[] = {
    {"AUTHOR",  AUTHOR, SHELL_STATE_ALL ,   func_shell_AUTHOR, func_shell_AUTHOR, "ID func is test ID"},
    {"IP",      IP,     SHELL_STATE_ALL,    func_shell_IP, func_shell_IP, "LIST description is list users,...."},
    {"PORT",    PORT,   SHELL_STATE_ALL,    func_shell_PORT, func_shell_PORT, "HELP print all help messages description is whicnn,...."},
    {"LIST",    LIST,   SHELL_STATE_CLIENT_LOGINED|SHELL_STATE_SERVER, 
                                            func_shell_list, func_server_list, "HELP print all help messages description is whicnn,...."},

    {"STATISTICS", STATISTICS, SHELL_STATE_SERVER, func_shell_statistics, func111, ""},
    {"BLOCKED", BLOCKED, SHELL_STATE_SERVER, func_shell_blocked, func111, ""},

    //WHEN NOT LOGIN, ONLY LOGIN,EXIT,IP,PORT,and AUTHOR,
    {"LOGIN", LOGIN,    SHELL_STATE_CLIENT, func_shell_LOGIN, func_server_login, "LOG IN 127.0.0.1 1234"},
    {"REFRESH", REFRESH,SHELL_STATE_CLIENT_LOGINED, func_shell_refresh, func_server_refresh, ""},
    {"SEND", SEND,      SHELL_STATE_CLIENT_LOGINED, func_shell_send, func_server_send, "send msg", func_shell_on_recv_send},
    {"BROADCAST", BROADCAST,SHELL_STATE_CLIENT_LOGINED, func_shell_broadcast, func_server_broadcast, "", func_shell_on_recv_send},
    {"BLOCK", BLOCK,        SHELL_STATE_CLIENT_LOGINED, func_shell_block, func_server_block, ""},
    {"UNBLOCK", UNBLOCK,    SHELL_STATE_CLIENT_LOGINED, func_shell_unblock, func_server_block, ""},
    {"LOGOUT", LOGOUT,      SHELL_STATE_CLIENT_LOGINED, func_shell_logout, func_server_logout, ""},
    {"EXIT", EXIT,          SHELL_STATE_ALL, func_shell_exit, func_server_exit, ""},
};

struct TEST_CMD {
    char *str;
    char *cmd;
};

char *test_cmd[] = {
    "LOGIN 192.168.42.134 11111",                        //0
    "LOGIN 192.168.122.1 11111",                         //1
    "LOGIN 127.0.0.1 11111",                             //2
    "SEND 192.168.42.134 HELLOWORLD_to134",              //3
    "SEND 192.168.122.1 HELLO_to_192.168.122.1_OWORLD",  //4
    "SEND 127.0.0.1 HELLO_to_127.0.0.1_OWORLD",          //5
    "REFRESH",                                           //6
    "LIST",                                              //7
    "BROADCAST HELLO_BROADCAST_1231111111",              //8
    "BROADCAST HELLO_BROADCAST_6666666666",              //9
    "LOGOUT",                                            //10
    "BLOCK 192.168.122.1",                               //11
    "UNBLOCK 192.168.122.1",                             //12
    "STATISTICS",                                        //13
    "BLOCKED 192.168.42.134",                            //14
    "BLOCK 127.0.0.1",                                   //15
    "STATISTICS",                                        //16

};
void run_command(unsigned char *str) {
    // str_to_upper(str);
    if (strlen(str) == 0) {
        return;
    }
    if (strlen(str) <= 2 && strcmp(str, "IP") != 0) {
        int select = atoi(str);
        logd("select[%d]\n", select);
        if (select >= 0 && select < sizeof(test_cmd) / sizeof(char *)){
            str = test_cmd[select];
        }
    }
    logd("[%s]\n", str);
    char cmd_str_copy[256];
    memset(cmd_str_copy, 0, 256);
    strncpy(cmd_str_copy, str, 255);
    char* space_ptr = strchr(cmd_str_copy, ' ');
    logd("[%s][%s]\n", str, space_ptr);

    if( space_ptr != NULL ){
        *space_ptr = '\0';
        space_ptr++;
    }else{
        space_ptr = cmd_str_copy;
    }
    
    logd("[%s]\n", str);
    
    for (int i = 0; i < sizeof(cmds) / sizeof(Command); i++) {
        // logd("%s, %s\n", cmd_str_copy, cmds[i].cmd);
        if (strcmp(cmd_str_copy, cmds[i].cmd) == 0) {
            // logd("cmds[%d].cmd_shell_state=[%08x],CUR_SHELL_STATE=[%08x]\n", i, cmds[i].cmd_shell_state, CUR_SHELL_STATE);
            if (cmds[i].cmd_shell_state & CUR_SHELL_STATE) {
                logd("Mode check OK.\n");
                char *buff_out = malloc(sizeof(VClient) * MAX_CLIENTS_LIMIT);
                logd("[running cmd: [%s](%s) ]\n", cmds[i].cmd, str + strlen(cmds[i].cmd) + 1);
                cmds[i].shell_func(space_ptr , buff_out);
                free(buff_out);
            } else {
                logd("Mode check NNNNNNNNNNNN. You are in xxx mode, but run yyy's command\n");

            }
            return 0;
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
        logd("server run cmd: [%d],[%d]\n", fd, tMSG_TRANSFER->cmd);
        cmds[tMSG_TRANSFER->cmd].server_func(fd, tMSG_TRANSFER, size);
    }
}

int func_shell_recv_msg_handler(int fd, MSG_TRANSFER *tmsg_transfer, size_t size) {
    dump_msg_transfer(tmsg_transfer);
    if (tmsg_transfer->cmd >= 0 && tmsg_transfer->cmd < CMD_ID_MAX) {
        cmds[tmsg_transfer->cmd].client_recv_func(fd, tmsg_transfer);
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
        // vhit: stdin input handler
        if (FD_ISSET(0, &read_fds)) {
            char cmd_buffer[512];
            nbytes = read(0, cmd_buffer, sizeof(cmd_buffer));
            cmd_buffer[nbytes - 1] = '\0';
            run_command(cmd_buffer);
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
                        //
                        /* got error or connection closed by client */
                        if (nbytes == 0) {
                            /* connection closed */
                            if (server_clients[i].status == LOGGED_IN) {
                                memset(&server_clients[i], 0, sizeof(VClient));
                                server_clients_count--;
                            }

                            printf("%s: socket %d hung up\n", argv[0], i);

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
                        if (CUR_MODE == MODE_SERVER) {
                            logd("server_mode, msg from:client  >> %d, send>>>%s\n", i, buf);
                            func_server_recv_msg_handler(i, buf, nbytes);
                        } else {
                            if (CUR_MODE == MODE_CLIENT) {
                                logd("msg from:client %d, send>>>%s\n", i, buf);
                                func_shell_recv_msg_handler(i, buf, nbytes);
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

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        loge("Please enter mode and port: [c/s] [port]\n");
        return INTERNAL_ERROR;
    }
    if (strcmp(argv[1], "c") == 0) {
        //client mode
        logd("in client mode");
        CUR_MODE = MODE_CLIENT;
        CUR_SHELL_STATE |= SHELL_STATE_CLIENT;
        LISTEN_PORT = atoi(argv[2]);
    } else {
        if (strcmp(argv[1], "s") == 0) {
            //server mode
            logd("in server mode");
            CUR_MODE = MODE_SERVER;
            CUR_SHELL_STATE |= SHELL_STATE_SERVER;
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
    logd("port is %d\n", LISTEN_PORT);
    init_client_info();
    server_main(argc, argv);

    return 0;
}