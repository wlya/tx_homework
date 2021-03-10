#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define sppp(fff, ...)  printf( "%d:%s:" fff ,__LINE__,__FUNCTION__,__VA_ARGS__)

#define DEBUG 1
#define loge printf
#define vlog printf
#define vlog_success printf
#define vlog_error printf

#ifdef DEBUG
#define logd printf
#else
#define logd(...) /**/
#endif
char* LOGIN_STATUS[] = {
    "UN_SEE",
    "logged-in",
    "logged-out"
};
enum CmdId
{
    AUTHOR = 0,
    IP,
    PORT,
    LIST,

    STATISTICS,
    BLOCKED,

    LOGIN,
    REFRESH,
    SEND,
    BROADCAST,
    BLOCK,
    UNBLOCK,
    LOGOUT,
    EXIT,

    CMD_ID_MAX
};

#define MODE_SERVER
enum CommandType
{
    NONE,
    SERVER,
    CLIENT,
    COMMON,
};
enum ClientStatus{
    UN_SEE = 0,
    LOGGED_IN,
    LOGGED_OUT
};

#define IP_ADDR_MAX_LEN 16
#define STATUS_MAX_LEN 16
#define MAX_MSG_LEN 256
#define HOSTNAME_MAX_LEN 32
#define MAX_CLIENTS_LIMIT 128
#define MAX_LOGIN_LIMIT 128
#define MAX_CMD_TRANSFER_SIZE 508
typedef struct MSG_TRANSFER
{
    uint32_t cmd;
    char src_ip[IP_ADDR_MAX_LEN];
    char dst_ip[IP_ADDR_MAX_LEN];
    char msg[MAX_CMD_TRANSFER_SIZE];
}MSG_TRANSFER;


typedef struct Command
{
    unsigned char *cmd;
    uint32_t cmd_id;
    uint32_t cmd_shell_state;
    void (*shell_func)(char *, char *);
    void (*server_func)(int, char *, size_t);
    unsigned char *v;
    void (*client_recv_func)(int, MSG_TRANSFER*);
} Command;

// typedef struct ClientOnRecvCmd
// {
//     unsigned char *cmd;
//     uint32_t cmd_id;
//     void (*handler)(int, MSG_TRANSFER*);
//     unsigned char *description;
// }ClientOnRecvCmd;

#define MODE_NONE  0x001
#define MODE_SERVER 0x010
#define MODE_CLIENT 0x100

#define ONLINE 0xAB
int CUR_MODE = MODE_NONE;
int LISTEN_PORT = -2;


#define SHELL_STATE_ALL             0x1111111
#define SHELL_STATE_SERVER          0x0000010
#define SHELL_STATE_CLIENT          0x0000100
#define SHELL_STATE_CLIENT_LOGINED  0x0001000
#define SHELL_STATE_CLIENT_LOGOUTED ~SHELL_STATE_CLIENT_LOGINED //0x1110111



uint32_t CUR_SHELL_STATE = 0x0;


typedef struct VClient
{
    uint32_t fd;
    uint64_t block_list;
    uint32_t status;
    char hostname[HOSTNAME_MAX_LEN];
    char ip_addr[IP_ADDR_MAX_LEN];
    uint32_t port;
    uint32_t num_msg_sent;
    uint32_t num_msg_rcv;
} VClient;

VClient myself_info;
size_t client_clients_count = 0;
size_t server_clients_count = 0;
VClient server_clients[MAX_CLIENTS_LIMIT];
VClient client_clients[MAX_CLIENTS_LIMIT];

typedef struct student{
	int score;
	struct student *next;
} LinkList;
void dump_msg_transfer(MSG_TRANSFER* msg){
    return;
    logd("+++++++++++++++++++++++++++++\n");
    logd("src_ip: %s\n", msg->src_ip);
    logd("dst_ip: %s\n", msg->dst_ip);
    logd("cmd: %d\n", msg->cmd);
    logd("msg: %s\n", msg->msg);
}
void dump_client_info(VClient *client)
{
    return;
    logd("+++++++++++++++++++++++++++++\n");
    logd("fd is %d\n", client->fd);
    // logd("id is %d\n", client->id);
    logd("status  is %d\n", client->status);
    logd("hostname is %s\n", client->hostname);
    logd("ip_addr  is %s\n", client->ip_addr);
    logd("port     is %d\n", client->port);
    logd("num_msg_sent is %d\n", client->num_msg_sent);
    logd("num_msg_rcv is  %d\n", client->num_msg_rcv);
    logd("+----------------------------+\n");
}
typedef struct MSG_SEND{
    char msg[MAX_MSG_LEN];
}MSG_SEND;

typedef struct MSG_BROADCAST{
    char msg[MAX_MSG_LEN];
};


void get_my_public_ip(char *str)
{
    int client_sockfd;
    int len;
    struct sockaddr_in remote_addr;                       //服务器端网络地址结构体
    char buf[32];                                         //数据传送的缓冲区
    memset(&remote_addr, 0, sizeof(remote_addr));         //数据初始化--清零
    remote_addr.sin_family = AF_INET;                     //设置为IP通信
    remote_addr.sin_addr.s_addr = inet_addr("35.187.147.242"); //服务器IP地址
    remote_addr.sin_port = htons(28888);                   //服务器端口号

    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
    if ((client_sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return 1;
    }

    /*将套接字绑定到服务器的网络地址上*/
    if (connect(client_sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        perror("connect error");
        return 1;
    }
    // logd("connected to server\n");
    len = recv(client_sockfd, str, 32, 0); //接收服务器端信息
    close(client_sockfd);
    str[len] = '\0';
    logd("recv[%d]:[%s]", len, str); //打印服务器端信息
    return;
}

int is_valid_ip_addr(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    logd("is valid ip addr = %d, [%s] \n", result, ipAddress);
    return result; //!= 0;? 1:0;
}


void str_to_upper(unsigned char *s)
{
    while (*s)
    {
        *s = toupper(*(unsigned char *)s);
        s++;
    }
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int min(int a, int b)
{
    return a > b ? b : a;
}

void func111(char *a, char *b)
{
}

void hexDump(const char *desc, const void *addr, const int len)
{
    int i;
    unsigned char buff[17];
    const unsigned char *pc = (const unsigned char *)addr;
    // Output description if given.
    printf("\n==============%s : [%d]==============\n", desc, len);
    // Length checks.
    if (len == 0)
    {
        printf("  ZERO LENGTH\n");
        return;
    }
    else if (len < 0)
    {
        printf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }
    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0)
        {
            // Don't print ASCII buffer for the "zeroth" line.
            if (i != 0)
                printf("  %s\n", buff);
            // Output the offset.
            printf("  %04x ", i);
        }
        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);
        // And buffer a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) // isprint() may be better.
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }
    // And print the final ASCII buffer.
    printf("  %s\n", buff);
    printf("\n============================\n");

}

int vsend(int fd, char *buffer, size_t size, int flags)
{
    // hexDump(" send >>> ", buffer, size);
    return send(fd, buffer, size, flags);
}

int vrecv(int fd, char *buffer, size_t size, int flags)
{
    int ret = recv(fd, buffer, size, flags);
    // hexDump(" recv <<<  ", buffer, ret);
    return ret;
}

void func_client_refresh_handle_back(char *str_out);

