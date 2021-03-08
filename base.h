#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h> 
#include <string.h>

#include <sys/types.h> 

#define DEBUG 1
#define loge printf
#define vlog printf

#ifdef DEBUG
#define logd printf
#else
#define logd(...) /**/  
#endif

enum CommandType{
    NONE,
    SERVER,
    CLIENT,
    COMMON,
};

typedef struct Command
{
    unsigned char *cmd;
    enum CommandType type;
    void (*func)(char *, char *);
    unsigned char *description;
} Command;


#define P_STATE_NONE 0x001
#define P_STATE_SERVER 0x002
#define P_STATE_CLIENT 0x003

#define IN_USE 0xAB
int CUR_MODE = P_STATE_NONE;
int LISTEN_PORT = -2;

struct server_data{
    size_t client_count;
    
};

#define IP_ADDR_MAX_LEN     16
#define STATUS_MAX_LEN      16
#define HOSTNAME_MAX_LEN    256
#define MAX_CLIENTS_LIMIT   128
#define MAX_LOGIN_LIMIT     128
typedef struct VClient
{
    uint32_t fd;
    char in_use;
    char hostname[HOSTNAME_MAX_LEN];
    char ip_addr[IP_ADDR_MAX_LEN];
    uint32_t port;
    uint32_t num_msg_sent;
    uint32_t num_msg_rcv;
    char status[STATUS_MAX_LEN];
}VClient;

VClient myself_info;
VClient clients[MAX_CLIENTS_LIMIT];

void dump_client_info(VClient* client){
    logd("hostname is %s\n", client->hostname);
    logd("ip_addr  is %s\n", client->ip_addr);
    logd("port     is %d\n", client->port);
    logd("num_msg_rcv is  %d\n", client->num_msg_rcv);
    logd("num_msg_sent is %d\n", client->num_msg_sent);
}

void get_my_public_ip(char* str){
    int client_sockfd;
	int len;
	struct sockaddr_in remote_addr; //服务器端网络地址结构体
	char buf[32];  //数据传送的缓冲区
	memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
	remote_addr.sin_family=AF_INET; //设置为IP通信
	remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址
	remote_addr.sin_port=htons(8082); //服务器端口号
	
	/*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/
	if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket error");
		return 1;
	}
	
	/*将套接字绑定到服务器的网络地址上*/
	if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
	{
		perror("connect error");
		return 1;
	}
	// logd("connected to server\n");
	len=recv(client_sockfd,str,32,0);//接收服务器端信息
    str[len]='\0';
	logd("recv[%d]:[%s]",len, str); //打印服务器端信息
    return;
}


int is_valid_ip_addr(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    logd("is valid ip addr = %d\n", result);
    return result;//!= 0;? 1:0;
}


